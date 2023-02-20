obj-m	:= proc.o
obj     := proc

KERNELDIR := /home/dbsrud/rpi-linux
PWD       := $(shell pwd)

all: default

default:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KERNELDIR) M=$(PWD) modules
	rm -rf *.o *.mod* *dule*

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

