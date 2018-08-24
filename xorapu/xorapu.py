#!/usr/bin/env python3

import sys
import subprocess
import time
import signal
import re
import argparse
import os.path
import logging as log

from energy_parser import parse_file

if __name__ == "__main__":

    # Parse arguments
    parser = argparse.ArgumentParser(description='Xorapu. Automatic inference test on a tizen device. ' +
                                                 'This script assumes that: tunnel is created; sdb is connected; sdb is rooted')

    parser.add_argument('-v', '--verbose', action='store_true', help='Print information along execution')
    parser.add_argument('-g', '--save_graph', action='store_true', help='Save graph of energy usage of inference')
    parser.add_argument('-a', '--show_accuracy', action='store_true', help='Print the accuracy of the inference')

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('-i', '--image', type=str, metavar='IMAGE.bmp', help='Image input (.bmp)')
    group.add_argument('-f','--image_list', type=str, metavar='IMAGE_LIST.txt', help='Text file with a list of input images (.bmp)')

    args = parser.parse_args()

    # Get arguments
    if args.verbose:
        log.basicConfig(format="%(levelname)s: %(message)s", level=log.DEBUG)
    save_graph = args.save_graph
    show_accuracy = args.show_accuracy
    if args.image is not None:
        input_file = args.image
    else:
        input_file = args.image_list

    input_file_name = os.path.basename(input_file)

    # Start capturing energy readings
    log.info('starting bumblebee')
    subprocess.Popen("sdb shell './bumblebee > energy_output.txt'", shell=True, stdout=subprocess.DEVNULL)

    log.info('pushing input')
    subprocess.Popen("sdb push " + input_file + " .", shell=True, stdout=subprocess.DEVNULL)

    time.sleep(2)

    # Run infereces
    log.info('starting beeswax')
    if args.image is not None:
        beeswax = subprocess.Popen(["sdb", "shell", "./beeswax", "-i", input_file_name], stdout=subprocess.PIPE)
    else:
        beeswax = subprocess.Popen(["sdb", "shell", "./beeswax", "-f", input_file_name], stdout=subprocess.PIPE)
    log.info('finished beeswax')

    # Wait beeswax
    beeswax.wait()

    time.sleep(2)

    # Stop captuting energy readings
    log.info('sending HUP signal to bumblebee')
    subprocess.Popen("sdb shell pkill -HUP bumblebee", shell=True, stdout=subprocess.DEVNULL)

    time.sleep(1)

    # Retrieve energy readings
    log.info('pulling out bumblebee output')
    subprocess.call(["sdb", "pull", "energy_output.txt", "energy_output.txt"], stdout=subprocess.DEVNULL)

    # Read start and end timestamps of inferences
    start_timestamp = []
    stop_timestamp = []

    accuracy_line = []

    delimiter_regex = r"start-end: (?P<start>\d+\.?\d*) (?P<stop>\d+\.?\d*)"
    delimiter_pattern = re.compile(delimiter_regex)

    accuracy_regex =  r"top-5: ([A-Za-z ]* )\((\d+\.?\d*)%\)"
    accuracy_pattern = re.compile(accuracy_regex)

    match_delimiter = None
    match_accuracy = None

    for line in beeswax.stdout.readlines():
        # print_control_message(line.decode('utf-8').rstrip())

        match_delimiter = delimiter_pattern.match(line.decode('utf-8'))
        if match_delimiter:
            start_timestamp.append(int(float(match_delimiter.group("start")) * 1000))
            stop_timestamp.append(int(float(match_delimiter.group("stop")) * 1000))

        match_accuracy = accuracy_pattern.match(line.decode('utf-8'))
        if match_accuracy:
            accuracy_line.append(line)

    # Call energy readings parser
    log.info("Call parser for each inference:")
    i = 0
    for (start,stop) in zip(start_timestamp, stop_timestamp):
        print("\nInference " + str(i))
        if save_graph:
            parse_file("energy_output.txt", start, stop, graph_name='inference' + str(i) + '_graph.png')
        else:
            parse_file("energy_output.txt", start, stop)
        print("Duration:", (stop_timestamp[i] - start_timestamp[i]), "ms")
        if show_accuracy:
            print(accuracy_line[i].decode('utf-8').rstrip())
        i += 1


    if len(start_timestamp) > 1:
        log.info(("Call parser for all inferences:"))

        print("\nAll inferences")
        # Output: Always printed
        if save_graph:
            parse_file("energy_output.txt", start_timestamp[0], stop_timestamp[-1], graph_name='all_inferences_graph.png')
        else:
            parse_file("energy_output.txt", start_timestamp[0], stop_timestamp[-1])

        print("Duration:", (stop_timestamp[-1] - start_timestamp[0]), "ms")


    log.info("Start times: " + str(start_timestamp))
    log.info("Stop times:" + str(stop_timestamp))




