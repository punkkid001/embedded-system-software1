KDIR := /home/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := ku_ipc.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
