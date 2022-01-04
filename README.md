# tas5756-driver
Raspberry Pi (Linux) driver for a TAS5756M

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

Currently the overlay installed by `make dts` expects the TAS5756 to be at I2C address 0x4c **and** a PCM512x to be at I2C address 0x4d (to test simple-audio-card's aux-devs).
