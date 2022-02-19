# bd6490fv-driver
Raspberry Pi (Linux) driver for a BD6490FV

## Setup

```sh
sudo apt install build-essential device-tree-compiler
```

## Usage

```sh
make
make dts
```

Add to `/boot/config.txt`:

```
dtoverlay=my-amp
```

Optionally add `force_eeprom_read=0` as well to disable any automatic overlay loading if connected to a HAT with EEPROM.

Currently the overlay installed by `make dts` expects the BD6490FV to be at I2C address 0x40 and a PCM512x to be at I2C address 0x4d (for my custom sound card).
