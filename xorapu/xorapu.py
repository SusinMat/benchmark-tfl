#!/usr/bin/env python3

import sys
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

def print_control_message(msg, mute=False):
    if not mute:
        print(msg)


if __name__ == "__main__":

    mute_control = False
    generate_plot = False
    inference_image_path = "./grace_hopper.bmp" # Grace Hopper is the default
    inference_image_name = "grace_hopper.bmp"
    show_accuracy = False

    if "--mute" in sys.argv:
        mute_control = True

    if "--plot" in sys.argv:
        generate_plot = True

    if "--force_image" in sys.argv:
        i = sys.argv.index("--force_image")
        inference_image = sys.argv[i+1]
        inference_image_name = inference_image.split('/')[-1] # Not windows friendly

    # Start capturing energy readings


    # Start capturing energy readings and push image
    print_control_message('starting bumblebee', mute_control)
    if mute_control:
        bumblebee = subprocess.Popen("sdb shell './bumblebee > energy_output.txt'", shell=True,
                                     stdout=subprocess.DEVNULL)
        bumblebee = subprocess.Popen("sdb push " + inference_image + " .", shell=True,
                                     stdout=subprocess.DEVNULL)
    else:
        bumblebee = subprocess.Popen("sdb shell './bumblebee > energy_output.txt'", shell=True)
        bumblebee = subprocess.Popen("sdb push " + inference_image + " .", shell=True)

    time.sleep(2)

    # Run infereces
    print_control_message('starting beeswax', mute_control)
    beeswax = subprocess.Popen(["sdb", "shell", "./beeswax", "-i", inference_image_name],
                               stdout=subprocess.PIPE)

    #beeswax = subprocess.Popen(["sdb", "shell", "./beeswax", "-f", "image_list.txt"], stdout=subprocess.PIPE)
    print_control_message('finished beeswax', mute_control)

    time.sleep(2)

    # Stop captuting energy readings
    print_control_message('sending HUP signal to bumblebee', mute_control)

    if mute_control:
        subprocess.Popen("sdb shell pkill -HUP bumblebee", shell=True, stdout=subprocess.DEVNULL)

    else:
        subprocess.Popen("sdb shell pkill -HUP bumblebee", shell=True)


    time.sleep(2)

    # Retrieve energy readings
    print_control_message('pulling out bumblebee output', mute_control)

    if mute_control:
        subprocess.call(["sdb", "pull", "energy_output.txt", "energy_output.txt"],
                        stdout=subprocess.DEVNULL)
    else:
        subprocess.call(["sdb", "pull", "energy_output.txt", "energy_output.txt"])

    # Read start and end timestamps of inferences
    start_timestamp = []
    stop_timestamp = []

    delimiter_regex = r"start-end: (?P<start>\d+\.?\d*) (?P<stop>\d+\.?\d*)"
    delimiter_pattern = re.compile(delimiter_regex)
    match = None

    for line in beeswax.stdout.readlines():
        #print_control_message(line.decode('utf-8').rstrip())
        match = delimiter_pattern.match(line.decode('utf-8'))
        if match:
            start_timestamp.append(int(float(match.group("start")) * 1000))
            stop_timestamp.append(int(float(match.group("stop")) * 1000))

    # Call energy readings parser
    print_control_message("\nCall parser for each inference:", mute_control)
    i = 0

    if len(start_timestamp) > 1:
        for (start,stop) in zip(start_timestamp, stop_timestamp):
            print_control_message("\nInference ", i)
            if generate_plot:
                parse_file("energy_output.txt", start, stop,
                           graph_name='inference' + str(i) + '_graph.png')
            else:
                parse_file("energy_output.txt", start, stop, graph_name=None)
            i += 1

            print("Duration: ", (stop_timestamp[i] - start_timestamp[i]))


    print_control_message("\nCall parser for all inferences:\n", mute_control)
    if generate_plot:
        parse_file("energy_output.txt", start_timestamp[0], stop_timestamp[-1],
                   graph_name='all_inferences_graph.png')
    else:
        parse_file("energy_output.txt", start_timestamp[0], stop_timestamp[-1], graph_name=None)

    print("Duration: ", (stop_timestamp[-1] - start_timestamp[0]), "ms")


    s = "Start times: " + str(start_timestamp) + "\nStop times:" + str(stop_timestamp)
    print_control_message(s, mute_control)
