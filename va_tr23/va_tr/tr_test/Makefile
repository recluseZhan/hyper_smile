obj-m += crc1.o
crc1-objs := crc.o

#obj-m += limit1.o
#limit1-objs := limit.o

#obj-m += jxdev2.o
#jxdev2-objs := jxdev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean

