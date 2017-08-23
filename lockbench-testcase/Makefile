ifneq ($(KERNELRELEASE),)
#	obj-m := lockbench-series-parallel.o
	obj-m := lockbench.o
else
	#KERNELDIR ?= /lib/modules/`uname -r`/build
	KERNELDIR ?= /lib/modules/`uname -r`/build  #/lib/modules/`uname -r`/build
	#KERNELDIR ?= /home/zhouchengming/sailingos #/lib/modules/`uname -r`/build
	MAKE = make ARCH=arm64 #CROSS_COMPILE=aarch64-linux-gnu- #-ffunction-sections -fdata-sections
all:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules

clean:
	$(MAKE) -C $(KERNELDIR) M=`pwd` clean
	rm -f Modules.symvers

endif
