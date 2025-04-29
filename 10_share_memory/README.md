System V共享内存则更传统、更广泛支持

```bash
gcc sender.c -o sender
# gcc receiver.c -o receiver
# g++ sender.cpp -o write_shm
g++ receiver.cpp -o receiver
```

```bash
# 查看共享情况
ipcs -m
```

POSIX共享内存是另一种更现代的共享内存实现方式。

```bash
# 编译写入端
gcc posix_write.c -o writer -lrt -lpthread

# 编译读取端
g++ posix_read.cpp -o reader -lrt -lpthread
```

POSIX共享内存通常位于/dev/shm目录下


如果程序异常终止，可能需要手动清理残留资源：(先进入目录查看，然后看情况清理)

```bash
sudo rm /dev/shm/posix_shm_example
sudo rm /dev/shm/sem.write_sem
sudo rm /dev/shm/sem.read_sem
```
