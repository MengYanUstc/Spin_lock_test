test_num=16
let "num=$test_num-1"
dmesg|tail -n $num >test_ctime.txt
scp ./test_ctime.txt yanmeng@192.168.1.107:~/tftp/ubuntu/
