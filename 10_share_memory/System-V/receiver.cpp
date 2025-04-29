#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <cstring>

const int SHM_SIZE = 1024;
const key_t SEM_KEY = 1234;
const key_t SHM_KEY = 5678;

// 封装共享内存接收者的类
class SharedMemoryReceiver
{
private:
    int semid;
    int shmid;
    char *shmaddr;

    // 等待信号量操作
    void semaphore_wait()
    {
        struct sembuf sops = {0, -1, 0};
        if (semop(semid, &sops, 1) == -1)
        {
            perror("semop wait");
            exit(EXIT_FAILURE);
        }
    }

    // 模拟消息处理函数
    void process_message(const char *message)
    {
        std::cout << "Received message: " << message << std::endl;
    }

public:
    // 构造函数，用于初始化
    SharedMemoryReceiver() : semid(-1), shmid(-1), shmaddr(nullptr)
    {
        // 获取或创建信号量
        semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
        if (semid == -1)
        {
            perror("semget");
            return;
        }

        // 获取或创建共享内存
        shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
        if (shmid == -1)
        {
            perror("shmget");
            return;
        }
        // 附加共享内存到当前进程
        shmaddr = reinterpret_cast<char *>(shmat(shmid, nullptr, 0));
        if (shmaddr == reinterpret_cast<char *>(-1))
        {
            perror("shmat");
            return;
        }
    }

    // 接收消息的函数
    void receive_messages()
    {
        while (true)
        {
            // 阻塞等待信号量
            semaphore_wait();

            // 从共享内存读取消息
            char buffer[SHM_SIZE];
            std::strcpy(buffer, shmaddr);

            // 处理接收到的消息
            process_message(buffer);
        }
    }

    // 析构函数，用于清理资源
    ~SharedMemoryReceiver()
    {
        if (shmaddr != nullptr)
        {
            // 分离共享内存
            if (shmdt(shmaddr) == -1)
            {
                perror("shmdt");
            }
        }
    }
};

int main()
{
    SharedMemoryReceiver receiver;
    receiver.receive_messages();
    return 0;
}
