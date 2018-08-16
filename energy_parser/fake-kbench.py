#!/usr/bin/env python3

from subprocess import Popen, call
from time import sleep, clock_gettime
import signal
import re

import tflenergy

# Assuming:
#   tunnel is created
#   sdb is connected
#   sdb is rooted
#   model, images and labels are inside the device

if __name__ == "__main__":
    bumblebee = Popen(["sdb", "shell", "./bumblebee", ">", "energy_output.txt"])

    sleep(5)

    beeswax = Popen("sdb", "shell", "./beeswax", "-i", "grace_hopper.bmp")
    stdout_beeswax, stderr_besswax = beeswax.communicate()

    # read start time in beeswax stdout
    # read end time in beeswax stdout

    sleep(1)

    bumblebee.send_signal(signal.SIGINT)

    sleep(5)

    call("sdb", "pull", "energy_output.txt", "energy_output.txt")

    delimiter_regex = r"start-end: (?P<start>\d+\.?\d*) (?P<stop>\d+\.?\d*)"
    delimiter_pattern = re.compile(delimiter_regex)

    match = None
    start_timestamp = None
    stop_timestamp = None

    for output_line in stdout_beeswax:
        match = delimiter_pattern.match(output_line):

        if match:
            start_timestamp = float(match.group("start"))*1000
            stop_timestamp = float(match.group("stop"))*1000
            break

    # call parser
    tflenergy.parse_file("energy_output.txt", start_time, end_time)
