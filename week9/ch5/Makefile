KDIR := /home/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m = ch5.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
