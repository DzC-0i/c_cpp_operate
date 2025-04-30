#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <csignal>
#include <atomic>

#define SHM_NAME "/posix_shm_example"
#define WRITE_SEM "/write_sem"
#define READ_SEM "/read_sem"
#define SHM_SIZE 4096

std::atomic<bool> should_exit(false); // 原子标志，控制主循环退出

// 信号处理函数：设置退出标志
void signal_handler(int sig)
{
    should_exit.store(true);
}

class SyncSharedMemoryReader
{
public:
    SyncSharedMemoryReader() : shm_ptr(nullptr),
                               write_sem(SEM_FAILED),
                               read_sem(SEM_FAILED) {}

    ~SyncSharedMemoryReader()
    {
        cleanup();
    }

    bool init()
    {
        // 开始前清除所有残余资源
        shm_unlink(SHM_NAME);
        sem_unlink(WRITE_SEM);
        sem_unlink(READ_SEM);

        // 创建共享内存对象
        int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1)
        {
            std::cerr << "shm_open failed: " << strerror(errno) << std::endl;
            return false;
        }

        // 配置共享内存大小
        if (ftruncate(shm_fd, SHM_SIZE) == -1)
        {
            std::cerr << "ftruncate failed: " << strerror(errno) << std::endl;
            close(shm_fd);
            return false;
        }

        // 映射共享内存
        shm_ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
        close(shm_fd);
        if (shm_ptr == MAP_FAILED)
        {
            std::cerr << "mmap failed: " << strerror(errno) << std::endl;
            return false;
        }

        // 创建/打开信号量
        write_sem = sem_open(WRITE_SEM, O_CREAT | O_EXCL, 0666, 0);
        if (write_sem == SEM_FAILED)
        {
            std::cerr << "sem_open write_sem failed: " << strerror(errno) << std::endl;
            munmap(shm_ptr, SHM_SIZE);
            return false;
        }

        read_sem = sem_open(READ_SEM, O_CREAT | O_EXCL, 0666, 0);
        if (read_sem == SEM_FAILED)
        {
            std::cerr << "sem_open read_sem failed: " << strerror(errno) << std::endl;
            sem_close(write_sem);
            munmap(shm_ptr, SHM_SIZE);
            return false;
        }

        return true;
    }

    std::string read_data()
    {
        // 等待数据可用
        if ((sem_wait(write_sem) == -1) || should_exit.load())
        {
            // 如果是因为信号中断，检查 should_exit
            if (should_exit.load())
            {
                std::cerr << "Exiting read_data due to Ctrl+C" << std::endl;
                return "";
            }
            std::cerr << "sem_wait failed: " << strerror(errno) << std::endl;
            return "";
        }

        // 读取数据
        std::string data(static_cast<const char *>(shm_ptr));

        // 发送确认信号
        if (sem_post(read_sem) == -1)
        {
            std::cerr << "sem_post failed: " << strerror(errno) << std::endl;
        }

        return data;
    }

private:
    void cleanup()
    {
        if (shm_ptr != nullptr)
        {
            munmap(shm_ptr, SHM_SIZE);
            shm_unlink(SHM_NAME);
        }
        if (write_sem != SEM_FAILED)
        {
            sem_close(write_sem);
            sem_unlink(WRITE_SEM);
        }
        if (read_sem != SEM_FAILED)
        {
            sem_close(read_sem);
            sem_unlink(READ_SEM);
        }
    }

    void *shm_ptr;
    sem_t *write_sem;
    sem_t *read_sem;
};

int main()
{
    // 注册信号处理：响应Ctrl+C（SIGINT）
    // signal(SIGINT, signal_handler);  // 这个不是很好用

    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // 不自动重启被中断的系统调用
    if (sigaction(SIGINT, &sa, NULL) == -1)
    {
        std::cerr << "Failed to set signal handler: " << strerror(errno) << std::endl;
        return 1;
    }

    SyncSharedMemoryReader reader;

    if (!reader.init())
    {
        std::cerr << "Failed to initialize reader" << std::endl;
        return 1;
    }
    while (!should_exit.load())
    {
        std::string received = reader.read_data();
        if (!received.empty())
        {
            std::cout << "Reader: Received data: " << received << std::endl;
        }
        else
        {
            break; // Exit if received empty data due to signal
        }
    }

    return 0;
}

