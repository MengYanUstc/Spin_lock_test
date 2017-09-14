#! /bin/bash


DONE="/sys/module/lockbench/parameters/test_done"
MODULE="lockbench.ko"

rmmod $MODULE > /dev/null

if ! insmod $MODULE threads_num=32; then
	echo "insmod fail"
	exit -1
fi

while true; do
	ret=$(cat $DONE)
	if [ $ret -ne 0 ]; then
		break
	fi
done

if [ $ret -eq -1 ]; then
	echo "test fail"
	rmmod $MODULE
	exit -1
fi

rmmod $MODULE

dmesg | tail -n 32 | grep lockbench: > test.txt

