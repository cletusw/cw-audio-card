#!/bin/bash

# Ignore errors, just means someone already set up this pin for access
echo "4" > /sys/class/gpio/export 2> /dev/null

# Amp enable
echo "out" > /sys/class/gpio/gpio4/direction
# Redundant since 0 is the default out value
echo "0" > /sys/class/gpio/gpio4/value

echo "Amp enabled. Run amp-unmute.sh"

./amp-status.sh
