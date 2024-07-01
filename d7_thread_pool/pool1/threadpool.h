#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "condition.h"

//封装线程池中的对象需要执行的任务对象
typedef struct task
{
    //函数指针，需要执行的任务
    void*(*run)(void* args);
    //参数
    void* arg;
    //下一个任务
    struct task* next;
}task_t;

//线程池结构体
typedef struct threadpool
{
    condition_t ready;       //状态量
    task_t* first;          //任务队列第一个任务
    task_t* last;           //任务队列最后一个任务
    int counter;            //线程池中已有线程数
    int idle;               //线程池中的空闲线程数
    int max_threads;        //线程池中最大线程数
    int quit;               //线程池退出标志
}threadpool_t;

//线程池初始化
void threadpool_init(threadpool_t* pool, int threads);

//往线程池中添加任务
void threadpool_add_task(threadpool_t* pool, void*(*run)(void* arg), void* arg);

//销毁线程池
void threadpool_destroy(threadpool_t* pool);




#endif // _THREAD_POOL_H_