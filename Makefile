# Allows building with sudo
# See https://sysprog21.github.io/lkmpg/#the-simplest-module
PWD := $(CURDIR)

default:
	echo "TODO"

clean:
	echo "TODO"

load:
	echo "TODO"

unload:
	echo "TODO"

dts: cw-audio-card-overlay.dts
	dtc -@ -I dts -O dtb -o /boot/overlays/cw-audio-card.dtbo cw-audio-card-overlay.dts
