g++ -o system_info system_info.cpp
./system_info


查看内存使用情况
free -h
或者
cat /proc/meminfo

查看硬盘使用情况
df / | awk 'NR==2 {print $5}'
或者
df

