#!/bin/bash

# Ignore errors, just means someone already set up this pin for access
echo "4" > /sys/class/gpio/export 2> /dev/null

# Amp disable
echo "out" > /sys/class/gpio/gpio4/direction
echo "1" > /sys/class/gpio/gpio4/value

./amp-status.sh
