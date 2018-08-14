#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy
import os
import re
import sys

def s_to_ms(s):
    return int("".join(s.split(".")))


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 tfenergy <input_file_name>")
        exit(1)

    filename = sys.argv[1]

    input_file = [line[:-1] for line in open(filename, "r").readlines()]

    if len(input_file) < 6:
        print("Error: input file is only " + str(len(input_file)) + " lines long.")
        exit(1)

    input_file = input_file[2:-2]

    if len(input_file) % 2 != 0:
        print("Error: input file has an odd number of lines.")
        exit(1)

    clock_regex = r"CLOCK: (?P<clock>\d+.\d{3}) s"
    clock_pattern = re.compile(clock_regex)
    match = clock_pattern.match(input_file[0])
    starting_timestamp = 0
    if match == None:
        print("Error: first line is not in the format " + clock_regex)
        exit(1)
    starting_timestamp = s_to_ms(match.group("clock"))
    previous_timestamp = starting_timestamp
    voltage = 5.0
    power = []
    for i in range(0, len(input_file), +2):
        clock = input_file[i]
        readings = input_file[i + 1]
        timestamp = s_to_ms(clock_pattern.match(clock).group("clock"))
        readings = [float(r) for r in readings.split(",")]
        timestamp_offset = timestamp - previous_timestamp
        print(str(timestamp) + " - " + str(previous_timestamp) + " = " + str(timestamp_offset))
        power += [float('NaN') for i in range(len(readings) + timestamp_offset)]
        previous_timestamp = timestamp + len(readings)

