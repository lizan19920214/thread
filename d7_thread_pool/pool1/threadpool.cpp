#include "threadpool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <iostream>

//线程创建函数
void* thread_routine(void* arg)
{
    struct timespec abstime;
    int timeout;
    std::cout << "thread tid:" << pthread_self() << " starting" << std::endl;
    threadpool_t* pool = (threadpool_t*)arg;

    while(1)
    {
        timeout = 0;
        //访问线程池之前加锁
        condition_lock(&pool->ready);

        //空闲线程加1
        pool->idle ++;
        
        //等待队列有任务到来或者线程池销毁通知
        while(pool->first == NULL && !pool->quit)
        {
            //否则线程阻塞等地啊
            std::cout << "thread tid:" << pthread_self() << " is waiting" << std::endl;
            //获取当前时间，并加上等待时间，设置线程的超市睡眠时间
            clock_gettime(CLOCK_REALTIME, &abstime);
            abstime.tv_sec += 2;
            int status;
            //该函数会解锁，允许其他线程访问，当被唤醒时，加锁
            status = condition_timedwait(&pool->ready, &abstime);
            if (status == ETIMEDOUT)
            {
                std::cout << "thread tid:" << pthread_self() << " time out" << std::endl;
                timeout = 1;
                break;
            }
        }

        //线程被唤醒，空闲线程--
        pool->idle --;
        
        if (pool->first != NULL)
        {
            //取出队列最前的任务，移除任务并执行
            task_t* t = pool->first;
            pool->first = t->next;
            //由于线程任务的执行需要时间，先解锁让其他线程访问线程池
            condition_unlock(&pool->ready);
            //执行任务
            t->run(t->arg);
            //执行完释放内存
            free(t);
            //重新加锁
            condition_lock(&pool->ready);
        }
        
        //退出线程池
        if (pool->quit && pool->first == NULL)
        {
            //当前工作线程--
            pool->counter --;
            //如果线程池中没有线程，通知等待的线程(主线程) 全部任务已经完成
            if (pool->counter == 0)
            {
                condition_signal(&pool->ready);
            }
            condition_unlock(&pool->ready);
            break;
        }
        
        //超时退出
        if (timeout == 1)
        {
            pool->counter --;
            condition_unlock(&pool->ready);
            break;
        }
        
        condition_unlock(&pool->ready);
    }

    std::cout << "thread tid:" << pthread_self() << " is exiting" << std::endl;

    return NULL;    
}

//线程池初始化
void threadpool_init(threadpool_t* pool, int threads)
{
    condition_init(&pool->ready);
    pool->first = NULL;
    pool->last = NULL;
    pool->counter = 0;
    pool->idle = 0;
    pool->max_threads = threads;
    pool->quit = 0;
}

//添加任务到线程池
void threadpool_add_task(threadpool_t* pool, void*(*run)(void* arg), void* arg)
{
    //产生一个新任务
    task_t* newtask = (task_t*)malloc(sizeof(task_t));
    newtask->run = run;
    newtask->arg = arg;
    //新任务放入队尾
    newtask->next = NULL;

    //线程池锁被多个线程共享，添加任务前加锁
    condition_lock(&pool->ready);

    //第一个任务
    if (pool->first == NULL)
    {
        pool->first = newtask;
    }
    else
    {
        //添加队尾
        pool->last->next = newtask;
    }
    
    //队尾指向最新的任务
    pool->last = newtask;

    //唤醒线程池中空闲线程
    if (pool->idle > 0)
    {
        condition_signal(&pool->ready);
    }
    //线程池中线程个数没达到最大，创建一个新线程
    else if(pool->counter < pool->max_threads)
    {
        pthread_t pid;
        pthread_create(&pid, NULL, thread_routine, pool);
        pool->counter ++;
    }
    
    //结束访问
    condition_unlock(&pool->ready);
}

//线程池销毁
void threadpool_destroy(threadpool_t* pool)
{
    //如果已经调用销毁，直接返回
    if (pool->quit)
    {
        return;
    }

    //加锁
    condition_lock(&pool->ready);
    //设置销毁标志
    pool->quit = 1;

    //线程池个数大于0
    if(pool->counter > 0)
    {
        //对于等待的线程，发送信号唤醒
        if (pool->idle > 0)
        {
            condition_broadcast(&pool->ready);

        }

        //等待执行任务的线程结束任务
        while(pool->counter)
        {
            condition_wait(&pool->ready);
        }
    }

    condition_unlock(&pool->ready);
    condition_destroy(&pool->ready);
}