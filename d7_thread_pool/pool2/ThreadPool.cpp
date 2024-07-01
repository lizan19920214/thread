#include "ThreadPool.h"
#include <string.h>
#include <unistd.h>
#include <iostream>

ThreadPool::ThreadPool(const int min, const int max):
m_minNum(min),
m_maxNum(max),
m_busyNum(0),
m_aliveNum(min),
m_exitNum(0),
m_shutdown(false)
{
    //实例化任务队列
    m_taskQ = new TaskQueue();
    //给线程数组分配内存
    m_threadIDs = new pthread_t[m_maxNum];
    memset(m_threadIDs, 0, sizeof(m_threadIDs));
    //初始化锁和条件变量
    pthread_mutex_init(&m_lock, NULL);
    pthread_cond_init(&m_not_Empty, NULL);
    //创建管理者线程
    pthread_create(&m_managerID, NULL, manager, this);
    
    //创建最少初始数量工作线程
    for (int i = 0; i < min; i++)
    {
        pthread_create(&m_threadIDs[i], NULL, worker, this);
    }
    
}

ThreadPool::~ThreadPool()
{
    this->m_shutdown = true;
    //销毁管理者线程
    pthread_join(m_managerID, NULL);
    //唤醒所有的消费者线程
    for (int i = 0; i < m_aliveNum; i++)
    {
        pthread_cond_signal(&m_not_Empty);
    }

    //销毁任务队列
    if (m_taskQ)
    {
        delete m_taskQ;
        m_taskQ = NULL;
    }
    
    //销毁保存消费者id的数组
    if (m_threadIDs)
    {
        delete[] m_threadIDs;
        m_threadIDs = NULL;
    }
    
    //销毁锁和条件变量
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_not_Empty);    
}

//添加任务
void ThreadPool::addTask(Task task)
{
    if (m_shutdown)
    {
        return;
    }

    //添加任务
    m_taskQ->addTask(task);
    //唤醒一个任务列表为空的工作处理线程
    pthread_cond_signal(&m_not_Empty);
}

void ThreadPool::addTask(callback func, void* arg)
{
    if (m_shutdown)
    {
        return;
    }

    //添加任务
    m_taskQ->addTask(func, arg);
    //唤醒一个任务列表为空的工作处理线程
    pthread_cond_signal(&m_not_Empty);
}

//工作线程任务函数
void* ThreadPool::worker(void* arg)
{
    //强转参数
    ThreadPool* pool = static_cast<ThreadPool*>(arg);

    //循环执行任务
    while (true)
    {
        //任务队列访问先加锁
        pthread_mutex_lock(&pool->m_lock);
        //未退出并且，任务为空则阻塞
        while(pool->m_taskQ->empty() && !pool->m_shutdown)
        {
            //阻塞等待非空信号
            pthread_cond_wait(&pool->m_not_Empty, &pool->m_lock);
            //空任务队列线程被唤醒
            std::cout << " thread tid:" << pthread_self() << " is wake up ,cur exit num:" << pool->m_exitNum << std::endl;

            //解除阻塞之后判断是否要销毁线程
            //任务列表为空的线程，并且线程池需要线程退出，
            //则减少退出线程数量
            if (pool->m_exitNum > 0)
            {
                //减少退出线程数量
                pool->m_exitNum--;
                if (pool->m_aliveNum > pool->m_minNum)
                {
                    pool->m_aliveNum --;
                    pthread_mutex_unlock(&pool->m_lock);
                    pool->threadExit();
                }
            }
        }

        //如果线程池要结束
        if (pool->m_shutdown)
        {
            //解锁，销毁
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }

        //从任务队列中取出一个任务
        Task task = pool->m_taskQ->getTask();
        //工作线程加1
        pool->m_busyNum ++;
        //解锁
        pthread_mutex_unlock(&pool->m_lock);

        //执行任务
        std::cout << "arg:" << task.getArg() << " callback" << std::endl;
        task.function(task.arg);

        //释放任务参数指针
        free(task.arg);
        task.arg = NULL;

        //任务执行完成，工作线程减1
        pthread_mutex_lock(&pool->m_lock);
        pool->m_busyNum --;
        pthread_mutex_unlock(&pool->m_lock);
        
    }

    return nullptr;
}

//管理者线程任务函数
void* ThreadPool::manager(void* arg)
{
    //参数强转
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    //线程池没关闭就一直检测
    while(!pool->m_shutdown)
    {
        //每5s监控一次线程池状态
        sleep(5);
        //取出任务数量和线程数量
        pthread_mutex_lock(&pool->m_lock);
        int queuesize = pool->m_taskQ->taskNumber();
        int liveNum = pool->m_aliveNum;
        int busyNum = pool->m_busyNum;
        pthread_mutex_unlock(&pool->m_lock);

        //创建线程
        const int NUMBER = 2;
        //当前任务太多，需要增加线程处理
        if (queuesize > liveNum && liveNum < pool->m_maxNum)
        {
            //线程池加锁
            pthread_mutex_lock(&pool->m_lock);
            int num = 0;
            for (int i = 0; i < pool->m_maxNum && num < NUMBER && pool->m_aliveNum < pool->m_maxNum; i++)
            {
                if (pool->m_threadIDs[i] == 0)
                {
                    pthread_create(&pool->m_threadIDs[i], NULL, worker, pool);
                    num ++;
                    pool->m_aliveNum++;
                }
            }

            //线程池解锁
            pthread_mutex_unlock(&pool->m_lock);
        }
        
        //当前任务太少，需要减少线程，减轻系统负担
        if (busyNum * 2 < liveNum && liveNum > pool->m_minNum + NUMBER)
        {
            //加锁
            pthread_mutex_lock(&pool->m_lock);
            pool->m_exitNum = NUMBER;
            //解锁
            pthread_mutex_unlock(&pool->m_lock);

            //唤醒线程，自动删除无任务的线程
            for (int i = 0; i < NUMBER; i++)
            {
                pthread_cond_signal(&pool->m_not_Empty);
            }
        }
    }

    return nullptr;
}

//线程退出
void ThreadPool::threadExit()
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < m_maxNum; i++)
    {
        if (m_threadIDs[i] == tid)
        {
            m_threadIDs[i] = 0;
            break;
        }
    }

    pthread_exit(NULL);
}
