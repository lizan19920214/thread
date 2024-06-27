/**
 * 读写锁
 *      读写锁是互斥锁的升级版, 在做读操作的时候可以提高程序的执行效率，
 *      如果所有的线程都是做读操作, 那么读是并行的。
 * 
 * 读写锁是一把锁，锁的类型为pthread_rwlock_t，有了类型之后就可以创建一把互斥锁了：
 *      pthread_rwlock_t rwlock;
 * 
 * 读写锁的特点：
 *      使用读写锁的读锁锁定了临界区，线程对临界区的访问是并行的，读锁是共享的。
 *      使用读写锁的写锁锁定了临界区，线程对临界区的访问是串行的，写锁是独占的。
 *      使用读写锁分别对两个临界区加了读锁和写锁，两个线程要同时访问者两个临界区，
 *          访问写锁临界区的线程继续运行，访问读锁临界区的线程阻塞，因为写锁比读锁的优先级高。
 *
 * 如果说程序中所有的线程都对共享资源做写操作，使用读写锁没有优势，和互斥锁是一样的，
 * 如果说程序中所有的线程都对共享资源有写也有读操作，并且对共享资源读的操作越多，读写锁更有优势。
 * 
 * 初始化读写锁
 *      int pthread_rwlock_init(pthread_rwlock_t *restrict rwlock, const pthread_rwlockattr_t *restrict attr);
 *      rwlock: 读写锁的地址，传出参数
 *      attr: 读写锁属性，一般使用默认属性，指定为NULL
 * 
 * 加读锁：
 *      int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
 *      调用这个函数，如果读写锁是打开的，那么加锁成功；如果读写锁已经锁定了读操作，
 *      调用这个函数依然可以加锁成功，因为读锁是共享的；如果读写锁已经锁定了写操作，调用这个函数的线程会被阻塞。
 * 
 * 加写锁：
 *      int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
 *      调用这个函数，如果读写锁是打开的，那么加锁成功；
 *      如果读写锁已经锁定了读操作或者锁定了写操作，调用这个函数的线程会被阻塞。
 * 
 * 解锁
 *      int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
 *      不管锁定了读还是写，都可以用这个解锁
 * 
 * 释放锁
 *      int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
 *      释放锁占的资源
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>

using namespace std;

#define NUM_READERS 3
#define NUM_WRITERS 2
#define NUM_READ_ITERATIONS 5
#define NUM_WRITE_ITERATIONS 3


//定义读写锁
pthread_rwlock_t rwlock;
//共享资源
int share_resource = 0;

//读操作
void* readerFunc(void* arg)
{
    int *index = (int*)arg;
    for (int i = 0; i < NUM_READ_ITERATIONS; i++)
    {
        //加读锁
        pthread_rwlock_rdlock(&rwlock);;
        //读取共享资源
        cout << "read value:" << share_resource << " index:" << *index << endl;
        //释放读锁
        pthread_rwlock_unlock(&rwlock);
        //模拟读操作的耗时
        usleep(100000);
    }

    return NULL;
}

//写操作
void* writerFunc(void* arg)
{
    int *index = (int*)arg;
    for (int i = 0; i < NUM_WRITE_ITERATIONS; i++)
    {
        //加写锁
        pthread_rwlock_wrlock(&rwlock);
        //修改共享资源
        share_resource++;
        cout << "write value:" << share_resource << " index:" << *index << endl;
        //释放写锁
        pthread_rwlock_unlock(&rwlock);
        //模拟写操作的耗时
        usleep(200000);
    }
    
    return NULL;
}

int main()
{
    //初始化读写锁
    if (pthread_rwlock_init(&rwlock, NULL) != 0)
    {
        perror("rwlock init error");
        exit(EXIT_FAILURE);
    }

    //定义读写线程
    pthread_t reader[NUM_READERS];
    int readerIndex[NUM_READERS];
    pthread_t writer[NUM_WRITERS];
    int writerIndex[NUM_READERS];

    //创建读线程
    for (int i = 0; i < NUM_READERS; i++)
    {
        readerIndex[i] = i;
        pthread_create(&reader[i], NULL, readerFunc, &readerIndex[i]);
    }
    
    //创建写锁
    for (int i = 0; i < NUM_WRITERS; i++)
    {
        writerIndex[i] = i;
        pthread_create(&writer[i], NULL, writerFunc, &writerIndex[i]);
    }
    
    //等待线程结束
    for (int i = 0; i < NUM_READERS; i++)
    {
        pthread_join(reader[i], NULL);
    }
    for (int i = 0; i < NUM_WRITERS; i++)
    {
        pthread_join(writer[i], NULL);
    }

    //释放读写锁
    pthread_rwlock_destroy(&rwlock);
    
    return 0;
}