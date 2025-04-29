#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>

#define SHM_NAME "/posix_shm_example"
#define WRITE_SEM "/write_sem"
#define READ_SEM "/read_sem"
#define SHM_SIZE 4096

// 共享内存和信号量结构
struct shm_context
{
    char *shm_ptr;
    sem_t *write_sem;
    sem_t *read_sem;
};

int init_resources(struct shm_context *ctx)
{
    // 打开共享内存对象
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open");
        return -1;
    }

    // 映射共享内存
    ctx->shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    if (ctx->shm_ptr == MAP_FAILED)
    {
        perror("mmap");
        return -1;
    }

    // 打开信号量
    ctx->write_sem = sem_open(WRITE_SEM, 0);
    if (ctx->write_sem == SEM_FAILED)
    {
        perror("sem_open write_sem");
        munmap(ctx->shm_ptr, SHM_SIZE);
        return -1;
    }

    ctx->read_sem = sem_open(READ_SEM, 0);
    if (ctx->read_sem == SEM_FAILED)
    {
        perror("sem_open read_sem");
        sem_close(ctx->write_sem);
        munmap(ctx->shm_ptr, SHM_SIZE);
        return -1;
    }

    return 0;
}

int write_data(struct shm_context *ctx, const char *data)
{
    if (ctx == NULL || data == NULL)
    {
        fprintf(stderr, "Invalid arguments\n");
        return -1;
    }

    // 写入数据
    strncpy(ctx->shm_ptr, data, SHM_SIZE - 1);
    ctx->shm_ptr[SHM_SIZE - 1] = '\0';

    // 通知读取端数据已准备好
    if (sem_post(ctx->write_sem) == -1)
    {
        perror("sem_post");
        return -1;
    }

    printf("Writer: Data written, waiting for reader...\n");

    // 等待读取确认
    if (sem_wait(ctx->read_sem) == -1)
    {
        perror("sem_wait");
        return -1;
    }

    return 0;
}

void cleanup_resources(struct shm_context *ctx)
{
    if (ctx == NULL)
        return;

    // 清理共享内存
    if (ctx->shm_ptr != NULL)
    {
        munmap(ctx->shm_ptr, SHM_SIZE);
    }

    // 清理信号量
    if (ctx->write_sem != SEM_FAILED)
    {
        sem_close(ctx->write_sem);
    }
    if (ctx->read_sem != SEM_FAILED)
    {
        sem_close(ctx->read_sem);
    }
}

int main()
{
    struct shm_context ctx = {0};

    if (init_resources(&ctx) != 0)
    {
        fprintf(stderr, "Failed to initialize resources\n");
        cleanup_resources(&ctx);
        return 1;
    }

    const char *message = "Synchronized hello from C writer!";
    if (write_data(&ctx, message) != 0)
    {
        fprintf(stderr, "Failed to write data\n");
    }

    cleanup_resources(&ctx);
    return 0;
}
