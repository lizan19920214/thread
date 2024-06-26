/**
 * g++ -o ShmWrite ShmWrite.cpp -lrt
 * 
 * shmget系统调用创建一段新的共享内存，或者获取一段已经存在的共享内存
 * #include <sys/ipc.h>
 * #include <sys/shm.h>
 * int shmget(key_t key, size_t size, int shmflg);
 * 
 * key: 共享内存标识符，可以由 ftok 函数生成或使用 IPC_PRIVATE。
 * size: 要创建的共享内存段的大小（以字节为单位）。
 * shmflg: 创建共享内存的标志，通常为权限标志（例如 0666），
 * 也可以与 IPC_CREAT 或 IPC_EXCL 等组合使用
 * 
 * shmget 函数的返回值是共享内存的标识符（如果成功），或者在出错时返回 -1，
 * 并设置 errno 来指示错误的原因。
 * 
 * ftok
 * #include <sys/types.h>
 * #include <sys/ipc.h>
 * // 将两个参数作为种子, 生成一个 key_t 类型的数值
 * key_t ftok(const char *pathname, int proj_id);
 * 
 * pathname: 当前操作系统中一个存在的路径
 * proj_id: 这个参数只用到了int中的一个字节, 传参的时候要将其作为 char 进行操作，
 *          取值范围: 1-255
 * 返回值：函数调用成功返回一个可用于创建、打开共享内存的key值，调用失败返回-1
 * 
 * 
 * 共享内存被创建/获取之后，我们不能立即访问它，而是需要先将它关联到进程的地址空间中。
 * 使用完共享内存之后，我们也需要将它从进程地址空间中分离。
 * 
 * 
 * #include <sys/shm.h>
 * void* shmat(int shm_id, const void* shm_addr, int shmflg);
 * int shmdt(const void* shm_addr);
 * 
 * shm_id参数是由shmget调用返回的共享内存标识符
 * shm_addr参数指定将共享内存关联到进程的哪块地址空间，一般来讲用户不知道，需要置NULL
 * shmflg参数的可选标志:
 * 
 * 
 * shmctl系统调用控制共享内存的某些属性。其定义如下：
 * #include <sys/shm.h>
 * int shmctl(int shm_id, int command, struct shmid_ds* buf);
 * 
 * shm_id参数是由shmget调用返回的共享内存标识符。
 * command参数指定要执行的命令
 * 
 * 
*/
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
    //打开共享内存
    int key = ftok("./shm", 'A');
    int shm_key = shmget(key, 4096, IPC_CREAT | 0666);

    //将共享内存关联到进程的地址空间
    void* shm_addr = shmat(shm_key, NULL, 0);

    //读取共享内存数据
    cout << (char*)shm_addr << endl;

    //解除共享内存与地址空间的关联
    shmctl(shm_key, IPC_RMID, NULL);
    cout << "delete shm" << endl;

    return 0;
}