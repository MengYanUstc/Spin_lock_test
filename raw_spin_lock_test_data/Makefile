CFLAGS_lockbench_trace.o = -I$(src)

ifneq ($(KERNELRELEASE),)
	obj-m := lockbench.o
	#obj-m += lockbench_trace.o
else
	#KERNELDIR ?= /lib/modules/`uname -r`/build
	KERNELDIR ?= /home/zhouchengming/sailingos #/lib/modules/`uname -r`/build
	#KERNELDIR ?= /home/zhouchengming/sailingos #/lib/modules/`uname -r`/build
	MAKE = make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- #-ffunction-sections -fdata-sections
all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean
	rm -f Modules.symvers

endif
