cores=8
p_to_s=31
let "num=$cores*$p_to_s+2"
dmesg|tail -n $num|grep lockbench:>test.txt
scp ./test.txt yanmeng@192.168.1.107:~/tftp/ubuntu/
