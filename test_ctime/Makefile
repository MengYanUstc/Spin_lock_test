CFLAGS_lockbench_trace.o = -I$(src)

ifneq ($(KERNELRELEASE),)
	obj-m := lockbench.o
#	obj-m += lockbench_1.o
#	obj-m := lockbench_2.o
#	obj-m := lockbench_3.o
#	obj-m := lockbench_4.o
#	obj-m := lockbench_5.o
#	obj-m := lockbench_6.o
#	obj-m := lockbench_7.o
else
	KERNELDIR ?= /lib/modules/`uname -r`/build
	#KERNELDIR ?= /home/zhouchengming/sailingos #/lib/modules/`uname -r`/build
	#KERNELDIR ?= /home/zhouchengming/sailingos #/lib/modules/`uname -r`/build
	MAKE = make #ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- #-ffunction-sections -fdata-sections
all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean
	rm -f Modules.symvers

endif
