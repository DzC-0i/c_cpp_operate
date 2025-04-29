#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>

#define SHM_SIZE 1024
#define SEM_KEY 1234
#define SHM_KEY 5678

// 定义一个结构体来保存共享内存和信号量的信息
typedef struct
{
    int semid;
    int shmid;
    char *shmaddr;
} SharedMemorySender;

// 初始化共享内存和信号量
int init_shared_memory_sender(SharedMemorySender *sender)
{
    // 创建或获取信号量
    sender->semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sender->semid == -1)
    {
        perror("semget");
        return -1;
    }
    // 初始化信号量为 0
    union semun
    {
        int val;
    } arg;
    arg.val = 0;
    if (semctl(sender->semid, 0, SETVAL, arg) == -1)
    {
        perror("semctl");
        return -1;
    }

    // 创建或获取共享内存
    sender->shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (sender->shmid == -1)
    {
        perror("shmget");
        return -1;
    }
    // 附加共享内存到当前进程
    sender->shmaddr = (char *)shmat(sender->shmid, NULL, 0);
    if (sender->shmaddr == (void *)-1)
    {
        perror("shmat");
        return -1;
    }

    return 0;
}

// 释放信号量操作
void semaphore_post(int semid)
{
    struct sembuf sops = {0, 1, 0};
    if (semop(semid, &sops, 1) == -1)
    {
        perror("semop post");
        exit(EXIT_FAILURE);
    }
}

// 发送消息的函数
int send_message(SharedMemorySender *sender, const char *message)
{
    if (sender == NULL || message == NULL)
    {
        return -1;
    }
    // 将消息写入共享内存
    strcpy(sender->shmaddr, message);

    // 释放信号量，通知接收方有新消息
    semaphore_post(sender->semid);

    printf("Sent message: %s\n", message);

    return 0;
}

// 清理资源的函数
void cleanup_shared_memory_sender(SharedMemorySender *sender)
{
    if (sender == NULL)
    {
        return;
    }
    // 分离共享内存
    if (sender->shmaddr != NULL)
    {
        if (shmdt(sender->shmaddr) == -1)
        {
            perror("shmdt");
        }
    }
}

#define MSG_COUNT 50000

int main()
{
    SharedMemorySender sender;
    if (init_shared_memory_sender(&sender) != 0)
    {
        return 1;
    }

    for (int i = 0; i < MSG_COUNT; i++)
    {
        char message[SHM_SIZE];
        snprintf(message, sizeof(message), "Message %d from C sender!", i + 1);
        send_message(&sender, message);
        // 1ms
        usleep(1000);
    }

    cleanup_shared_memory_sender(&sender);

    return 0;
}
