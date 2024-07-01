#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include "TaskQueue.h"
#include <thread>

class ThreadPool
{
public:
    ThreadPool(const int min, const int max);
    ThreadPool() : ThreadPool(5, 20) {}
    ~ThreadPool();

    //添加任务
    void addTask(Task task);
    //添加任务
    void addTask(callback func, void* arg);
    //获得忙线程个数
    const int getBusyNumber();
    //获得活着的线程个数
    const int getAliveNumber();

private:
    //工作线程的任务函数
    static void* worker(void* arg);
    //管理者线程的任务函数
    static void* manager(void* arg);
    //线程退出
    void threadExit();

private:
    pthread_mutex_t m_lock;
    pthread_cond_t m_not_Empty;
    pthread_t* m_threadIDs;
    pthread_t m_managerID;
    TaskQueue* m_taskQ;
    int m_minNum;
    int m_maxNum;
    int m_busyNum;
    int m_aliveNum;
    int m_exitNum;
    bool m_shutdown;
};

#endif // !_THREAD_POOL_H_