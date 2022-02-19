#!/bin/bash

# Ignore errors, just means someone already set up this pin for access
echo "27" > /sys/class/gpio/export 2> /dev/null

# Amp mute
echo "out" > /sys/class/gpio/gpio27/direction
echo "0" > /sys/class/gpio/gpio27/value

./amp-status.sh
