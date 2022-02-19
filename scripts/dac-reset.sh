#!/bin/bash

# Out of powerdown state
i2cset -f -y 1 0x4d 2 0x10

sleep 0.1

# Out of standby state
i2cset -f -y 1 0x4d 2 0x00

sleep 0.1

# GPIO 5 output left analog mute status (low = muted)
# i2cset -f -y 1 0x4d 84 0x08

# GPIO 5 invert output (since mux is wired for low = DAC)
# echo "Inverting GPIO 5 output (TODO: fix PCB layout instead)"
# i2cset -f -y 1 0x4d 87 0x10

# Temp: uninvert
i2cset -f -y 1 0x4d 87 0x00
# Temp: GPIO 5 output from register
i2cset -f -y 1 0x4d 84 0x02
# Temp: GPIO 5 output register: low
i2cset -f -y 1 0x4d 86 0x00
# high
# i2cset -f -y 1 0x4d 86 0x10

# GPIO 5 output enable
i2cset -f -y 1 0x4d 8 0x10

# Unmute
i2cset -f -y 1 0x4d 3 0x00
