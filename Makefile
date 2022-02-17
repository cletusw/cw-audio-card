TARGET_MODULE:=tas5756

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile

obj-m += $(TARGET_MODULE).o

else
# normal makefile

KDIR ?= /lib/modules/$(shell uname -r)/build
# Allows building with sudo
# See https://sysprog21.github.io/lkmpg/#the-simplest-module
PWD := $(CURDIR)

default:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

load:
	insmod ./$(TARGET_MODULE).ko

unload:
	rmmod ./$(TARGET_MODULE).ko

dts: my-amp-overlay.dts
	dtc -@ -I dts -O dtb -o /boot/overlays/my-amp.dtbo my-amp-overlay.dts

endif
