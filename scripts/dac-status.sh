#!/bin/bash

# Go to register page 0
i2cset -f 1 0x4d 0 0x0

echo "register 2 (standby, powerdown); should be 0x00"
i2cget -f -y 1 0x4d 2

echo "register 3 (mute request); should be 0x00"
i2cget -f -y 1 0x4d 3

echo "register 9 (master/slave controls); should be 0x11"
i2cget -f -y 1 0x4d 9

echo "register 42 (data path); should be 0x11"
i2cget -f -y 1 0x4d 42

echo "register 61 (left digital volume); should be 0x30"
i2cget -f -y 1 0x4d 61

echo "register 62 (right digital volume); should be 0x30"
i2cget -f -y 1 0x4d 62

echo "register 91 (sampling rate & sck ratio); should be 0x38"
i2cget -f -y 1 0x4d 91

echo "register 94 (clock and sample rate statuses); should be 0x20"
i2cget -f -y 1 0x4d 94

echo "register 95 (clock statuses and error flag); should be 0x00"
i2cget -f -y 1 0x4d 95

echo "register 108 (analog mute monitor); should be 0x?3"
i2cget -f -y 1 0x4d 108

echo "register 109 (short detect monitor); should be 0x00"
i2cget -f -y 1 0x4d 109

echo "register 114 (xsmt pin voltage level); should be 0x03"
i2cget -f -y 1 0x4d 114

echo "register 118 (power status); should be 0x88 in standby or 0x85 if ready to play"
i2cget -f -y 1 0x4d 118

echo "register 120 (auto mute status); should be 0x00"
i2cget -f -y 1 0x4d 120
