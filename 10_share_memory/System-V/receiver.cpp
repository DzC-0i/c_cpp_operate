#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <cstring>
#include <csignal>
#include <atomic>

const int SHM_SIZE = 1024;
const key_t SEM_KEY = 1234;
const key_t SHM_KEY = 5678;

std::atomic<bool> should_exit(false);  // 原子标志，控制主循环退出

// 信号处理函数：设置退出标志
void signal_handler(int sig) {
    should_exit.store(true);
}

class SharedMemoryReceiver {
private:
    int semid;
    int shmid;
    char* shmaddr;

    void semaphore_wait() {
        struct sembuf sops = {0, -1, 0};
        while (!should_exit.load() && semop(semid, &sops, 1) == -1) {
            if (errno == EINTR) {  // 处理被信号中断的系统调用
                continue;
            } else if (errno == EIDRM) {  // 资源已删除
                std::cout << "Resource removed, exiting." << std::endl;
                return;
            } else {
                perror("semop wait");
                exit(EXIT_FAILURE);
            }
        }
    }

    void process_message(const char* message) {
        std::cout << "Received message: " << message << std::endl;
    }

public:
    SharedMemoryReceiver() : semid(-1), shmid(-1), shmaddr(nullptr) {
        // 创建/获取信号量（接收方作为创建者）
        semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
        if (semid == -1) {
            perror("semget");
            exit(EXIT_FAILURE);
        }

        // 创建/获取共享内存
        shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
        if (shmid == -1) {
            perror("shmget");
            semctl(semid, 0, IPC_RMID);  // 清理信号量
            exit(EXIT_FAILURE);
        }

        shmaddr = reinterpret_cast<char*>(shmat(shmid, nullptr, 0));
        if (shmaddr == reinterpret_cast<char*>(-1)) {
            perror("shmat");
            shmctl(shmid, IPC_RMID, nullptr);  // 清理共享内存
            semctl(semid, 0, IPC_RMID);        // 清理信号量
            exit(EXIT_FAILURE);
        }
    }

    ~SharedMemoryReceiver() {
        if (shmaddr) {
            shmdt(shmaddr);  // 分离共享内存
        }
        // 标记资源为删除（核心回收逻辑）
        if (shmid != -1) {
            shmctl(shmid, IPC_RMID, nullptr);
        }
        if (semid != -1) {
            semctl(semid, 0, IPC_RMID);
        }
    }

    void receive_messages() {
        while (!should_exit.load()) {
            semaphore_wait();
            if (should_exit.load()) break;  // 退出前检查标志
            char buffer[SHM_SIZE];
            std::strcpy(buffer, shmaddr);
            process_message(buffer);
            memset(shmaddr, 0, SHM_SIZE);  // 清空内存
        }
    }
};

int main() {
    // 注册信号处理：响应Ctrl+C（SIGINT）
    signal(SIGINT, signal_handler);

    SharedMemoryReceiver receiver;
    receiver.receive_messages();

    return 0;
}