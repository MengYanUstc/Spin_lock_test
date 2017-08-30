rmmod lockbench
make clean
make 
insmod lockbench.ko
lsmod|grep lockbench
