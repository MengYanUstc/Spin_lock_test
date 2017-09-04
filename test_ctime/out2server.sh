test_num=63
let "num=$test_num*2+3"
dmesg|tail -n $num|grep cores:>test_ctime.txt
scp ./test_ctime.txt yanmeng@192.168.1.107:~/tftp/ubuntu/
