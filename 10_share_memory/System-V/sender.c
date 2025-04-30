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

typedef struct {
    int semid;
    int shmid;
    char *shmaddr;
} SharedMemorySender;

int init_shared_memory_sender(SharedMemorySender *sender) {
    // 仅获取已存在的资源（接收方必须先运行）
    sender->semid = semget(SEM_KEY, 1, 0);
    sender->shmid = shmget(SHM_KEY, SHM_SIZE, 0);
    if (sender->semid == -1 || sender->shmid == -1) {
        perror("sender: semget/shmget failed");
        return -1;
    }
    sender->shmaddr = (char *)shmat(sender->shmid, NULL, 0);
    if (sender->shmaddr == (void *)-1) {
        perror("sender: shmat failed");
        return -1;
    }
    return 0;
}

void semaphore_post(int semid) {
    struct sembuf sops = {0, 1, 0};
    semop(semid, &sops, 1);
}

int send_message(SharedMemorySender *sender, const char *message) {
    strcpy(sender->shmaddr, message);
    semaphore_post(sender->semid);
    printf("Sent message: %s\n", message);
    return 0;
}

void cleanup_shared_memory_sender(SharedMemorySender *sender) {
    shmdt(sender->shmaddr);  // 仅分离，不删除（接收方负责）
}

#define MSG_COUNT 5000

int main() {
    SharedMemorySender sender;
    char message[SHM_SIZE];
    if (init_shared_memory_sender(&sender) != 0) {
        return 1;
    }

    for (int i = 0; i < MSG_COUNT; i++) {
        sprintf(message, "test message %d from sender",i);
        send_message(&sender, message);
        usleep(100000);  // 100ms 间隔
    }

    cleanup_shared_memory_sender(&sender);
    return 0;
}