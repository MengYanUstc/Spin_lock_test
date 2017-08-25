cores=62
p_to_s=31
tests_num=5
let "num=($cores*$p_to_s*$tests_num+2+$cores*$tests_num)"
dmesg|tail -n $num|grep lockbench:>test.txt
scp ./test.txt yanmeng@192.168.1.107:~/tftp/ubuntu/
