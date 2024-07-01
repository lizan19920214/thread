/**
 * g++ -o thread_pool main.cpp Task.h TaskQueue.h ThreadPool.cpp -lpthread
 * 
 * 该线程池的实现思路：
 * 1. 线程池中创建一个任务队列，用于存储任务
 * 2. 同时创建一定数量的工作者线程，用于从任务队列获取任务并执行任务
 * 3、线程池创建一个管理线程，来动态调整工作线程的数量
 * 
 * 缺点：
 * 1、频繁的加锁和解锁操作
 * 2、队列为空时，任务线程的唤醒会存在惊群问题
 * 
 * CR线程池实例经验：
 * 每个线程都会创建单独的锁和条件变量，在任务添加的时候，先获取工作任务少的线程，添加到该线程中
 * 这样可以减少频繁的锁操作，同时，也不会存在惊群问题
 * 
*/
#include "ThreadPool.h"
#include <iostream>
#include <unistd.h>

void taskFunc(void* arg)
{
    int num = *(int*)arg;
    std::cout << "thread tid:" << pthread_self() << " is working, number = " << num << std::endl;
    sleep(1);
}

int main()
{
    //创建线程池
    ThreadPool* pool = new ThreadPool(2, 5);
    for (int i = 0; i < 100; i++)
    {
        int* num = (int*)malloc(sizeof(int));
        *num = i;
        pool->addTask(taskFunc, num);
    }

    sleep(30);
    delete pool;
    pool = nullptr;
    
    return 0;
}