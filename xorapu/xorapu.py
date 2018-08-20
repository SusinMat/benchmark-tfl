#!/usr/bin/env python3

import subprocess
import time
import signal
import re

from energy_parser import parse_file

# Assuming:
#   tunnel is created
#   sdb is connected
#   sdb is rooted
#   model, images, list of images and labels are inside the device

if __name__ == "__main__":

    # Start capturing energy readings
    print('starting bumblebee')
    bumblebee = subprocess.Popen("sdb shell './bumblebee > energy_output.txt'", shell=True)

    time.sleep(5)

    # Run infereces
    print('starting beeswax')
    #beeswax = subprocess.Popen(["sdb", "shell", "./beeswax", "-i", "grace_hopper.bmp"], stdout=subprocess.PIPE)
    beeswax = subprocess.Popen(["sdb", "shell", "./beeswax", "-f", "image_list.txt"], stdout=subprocess.PIPE)
    print('finished beeswax')

    time.sleep(5)

    # Stop captuting energy readings
    print('sending HUP signal to bumblebee')
    subprocess.Popen("sdb shell pkill -HUP bumblebee", shell=True)

    time.sleep(10)

    # Retrieve energy readings
    print('pulling out bumblebee output')
    subprocess.call(["sdb", "pull", "energy_output.txt", "energy_output.txt"])

    # Read start and end timestamps of inferences
    start_timestamp = []
    stop_timestamp = []

    delimiter_regex = r"start-end: (?P<start>\d+\.?\d*) (?P<stop>\d+\.?\d*)"
    delimiter_pattern = re.compile(delimiter_regex)
    match = None

    for line in beeswax.stdout.readlines():
        #print(line.decode('utf-8').rstrip())
        match = delimiter_pattern.match(line.decode('utf-8'))
        if match:
            start_timestamp.append(int(float(match.group("start")) * 1000))
            stop_timestamp.append(int(float(match.group("stop")) * 1000))

    # Call energy readings parser
    print("\nCall parser for each inference:")
    i = 0
    for (start,stop) in zip(start_timestamp, stop_timestamp):
        print("\nInference ", i)
        parse_file("energy_output.txt", start, stop, graph_name='inference' + str(i) + '_graph.png')
        i +=1

    print("\nCall parser for all inferences:")
    print("")
    parse_file("energy_output.txt", start_timestamp[0], stop_timestamp[-1], graph_name='all_inferences_graph.png')

    print("Start times: " + str(start_timestamp) + "\nStop times: " +  str(stop_timestamp))
