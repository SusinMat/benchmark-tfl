#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import sys
import matplotlib.pyplot as plt

def s_to_ms(s):
    return int("".join(s.split(".")))

# start_time and end_time: ms since epoch
def find_start_end(timestamps, start_time, end_time):
    start_index = end_index = 0

    for index,time in enumerate(timestamps, 1):
        if start_time < time:
            start_index = index - 1
            break

    for index,time in enumerate(reversed(timestamps), 1):
        if end_time > time:
            end_index = (len(timestamps) - 1 - index) + 1
            break

    return (start_index, end_index)


def parse_file(filename, start_time=None, stop_time=None, graph_name=None):

    with open(filename, 'r') as f:
        # Strip the last character of each line, typically a \n
        input_file = [line[:-1] for line in f.readlines()]

    # input_file:
    #   "clock 0980980"
    #   "0.554,0.526,0.532,0.533,0.515,0.553,..."

    # Input file must have at least one record other than the first and last
    if len(input_file) < 6:
        print("Error: input file is only " + str(len(input_file)) + " lines long.")
        exit(1)

    # Strip the first and last records
    input_file = input_file[2:-2]

    # There must be an even number of lines (timestamp followed by readings)
    if len(input_file) % 2 != 0:
        print("Error: input file has an odd number of lines.")
        exit(1)

    # Set regex for clock line
    clock_regex = r"CLOCK: (?P<clock>\d+\.\d{3}) s"
    clock_pattern = re.compile(clock_regex)

    # Check if first line matches with the patern
    match = clock_pattern.match(input_file[0])
    if match == None:
        print("Error: first line is not in the format " + clock_regex)
        exit(1)

    starting_timestamp = s_to_ms(match.group("clock"))
    previous_timestamp = starting_timestamp
    voltage = 5.0
    energy_spent = 0

    timestamps = []
    readings = []
    for i in range(0, len(input_file), 2):
        # clock line
        i_clock = s_to_ms(clock_pattern.match(input_file[i]).group("clock"))
        # energy readings line
        i_reading = input_file[i + 1]
        i_reading = i_reading.split(",")

        for j, reading in enumerate(i_reading):
            timestamps.append(i_clock+j)
            readings.append(float(reading))

    start_index = 0
    stop_index = len(timestamps) - 1

    # Find clocks closest to start and end
    if start_time is not None and stop_time is not None:
        (start_index, stop_index) = find_start_end(timestamps, start_time, stop_time)

    for i in range(start_index, stop_index+1):
        energy_spent += (voltage * readings[i]) / 1000.0

    print("Energy spent: %.3f J" % energy_spent)
    print("Average power: %.3f W" % ((1000.0 * energy_spent) / (timestamps[stop_index] - timestamps[start_index])))

    if graph_name is not None:
        graph_timestamp = [t - timestamps[0] for t in timestamps]
        graph_power = [voltage * r for r in readings]
        plt.scatter(graph_timestamp,
                    graph_power,
                    s=2,
                    alpha=0.5)
        if start_time is not None and stop_time is not None:
            plt.axvline(x=graph_timestamp[start_index], color='r', alpha=0.25)
            plt.axvline(x=graph_timestamp[stop_index], color='r', alpha=0.25)
        plt.xlabel('Time (ms)')
        plt.ylabel('Power (W)')
        plt.savefig(graph_name, bbox_inches='tight')
        plt.clf()

if __name__ == "__main__":

    filename = sys.argv[1]
    parse_file(filename)
