#!/usr/bin/env python3

from subprocess import Popen, call, PIPE
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
    print('starting bumblebee')
    bumblebee = Popen("sdb shell './bumblebee > energy_output.txt'", shell="/bin/bash")

    sleep(5)

    print('starting beeswax')
    beeswax = Popen(["sdb", "shell", "./beeswax", "-i", "grace_hopper.bmp"], stdout=PIPE)
    # stdout_beeswax, stderr_besswax = beeswax.communicate()
    print('finished beeswax')

    # read start time in beeswax stdout
    # read end time in beeswax stdout

    sleep(5)

    print('sending HUP signal to bumblebee')
    bumblebee.send_signal(signal.SIGHUP)

    sleep(10)

    print('pulling out bumblebee output')
    call(["sdb", "pull", "energy_output.txt", "energy_output.txt"])

    delimiter_regex = r"start-end: (?P<start>\d+\.?\d*) (?P<stop>\d+\.?\d*)"
    delimiter_pattern = re.compile(delimiter_regex)

    match = None
    start_timestamp = None
    stop_timestamp = None

    # print("\n\nSTDOUT-BEESWAX", stdout_beeswax)

    while True:
        output_line = beeswax.stdout.readline()
        if output_line != '':
            match = delimiter_pattern.match(output_line.decode('utf-8'))

            if match:
                start_timestamp = int(float(match.group("start")) * 1000)
                stop_timestamp = int(float(match.group("stop")) * 1000)
                break
            else:
                print(output_line.decode('utf-8'))
        else:
            break

    # call parser
    print("call parser")
    print(start_timestamp, stop_timestamp)
    tflenergy.parse_file("energy_output.txt", start_timestamp, stop_timestamp)
