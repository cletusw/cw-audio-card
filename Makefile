ifneq ($(KERNELRELEASE),)
# kbuild part of makefile

obj-m += tas5756.o

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

main.out: userspace.c
	gcc -li2c -O -o $@ $^

run: main.out
	./$<

endif
