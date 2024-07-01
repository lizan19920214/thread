/**
 * g++ -o thread_pool main.cpp condition.cpp threadpool.cpp -lpthread
*/
#include "threadpool.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#define NUM_THREADS 3
#define MAX_TASKS 10

void* mytask(void* arg)
{
    std::cout << "thread tid:" << pthread_self() << " is working on task:" << *(int*)arg << std::endl;
    sleep(1);
    free(arg);
    return NULL;
}

int main()
{
    threadpool pool;
    //初始化N个线程
    threadpool_init(&pool, NUM_THREADS);
    int i;
    //创建N个任务
    for (int i = 0; i < MAX_TASKS; i++)
    {
        int* arg = (int*)malloc(sizeof(int));
        *arg = i;
        //提交任务
        threadpool_add_task(&pool, mytask, arg);
    }

    threadpool_destroy(&pool);

    return 0;
}