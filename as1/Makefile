KDIR := /home/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := ku_ipc.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc ku_ipc_lib.c -o ku_ipc_lib
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
