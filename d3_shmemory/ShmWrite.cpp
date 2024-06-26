#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>


using namespace std;

int main()
{
    //创建共享内存
    int key = ftok("./shm", 'A');
    int shm_key = shmget(key, 4096, IPC_CREAT | 0666);

    //将共享内存关联到进程的地址空间
    void* shm_addr = shmat(shm_key, NULL, 0);

    //共享内存写入数据
    const char* str = "hello , send msg to memory";
    memcpy(shm_addr, str, strlen(str));

    sleep(10);

    //解除共享内存与地址空间的关联
    shmctl(shm_key, IPC_RMID, NULL);

    return 0;
}