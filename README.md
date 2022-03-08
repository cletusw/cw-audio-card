# cw-audio-card
Raspberry Pi (Linux) driver code for my custom PCM5122, BD3490FV, MA12070 audio
card: https://easyeda.com/c904/i2s-amp-with-aux-in

## Setup

```sh
sudo apt install build-essential device-tree-compiler
git submodule update --init --recursive
```

## Usage

Add to `/boot/config.txt`:

```
dtoverlay=cw-audio-card
```

```sh
make
sudo make dts
sudo reboot
sudo make load
```

Currently the overlay installed by `make dts` expects a PCM512x at I2C address
0x4d, a BD3490FV at I2C address 0x40, and a MA120x0 at I2C address 0x20.
