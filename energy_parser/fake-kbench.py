#!/bin/python3

from subprocess import Popen, call
from time import sleep, clock_gettime
import signal

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

    # call parser
    #tflenergy.parse_file("energy_output.txt", start_time, end_time)