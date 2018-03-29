KDIR := /home/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := simple_spin.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc -o simple_spin_reader simple_spin_reader.c
	arm-linux-gnueabi-gcc -o simple_spin_writer simple_spin_writer.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) clean
