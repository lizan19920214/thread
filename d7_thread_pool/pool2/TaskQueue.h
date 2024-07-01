#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_

#include "Task.h"
#include <queue>

//任务队列
class TaskQueue
{
public:
    TaskQueue()
    {
        pthread_mutex_init(&m_mutex, nullptr);
    }

    ~TaskQueue()
    {
        pthread_mutex_destroy(&m_mutex);
    }

    //添加任务
    inline void addTask(Task &task)
    {
        pthread_mutex_lock(&m_mutex);
        m_queue.push(task);
        pthread_mutex_unlock(&m_mutex);
    }

    //添加任务
    inline void addTask(callback func, void* arg)
    {
        pthread_mutex_lock(&m_mutex);
        m_queue.push(Task(func, arg));
        pthread_mutex_unlock(&m_mutex);
    }

    //取出任务
    inline Task getTask()
    {
        Task t;
        pthread_mutex_lock(&m_mutex);
        if (!m_queue.empty())
        {
            //取第一个任务弹出队列
            t = m_queue.front();
            m_queue.pop();
        }

        pthread_mutex_unlock(&m_mutex);
        return t;
    }

    //获得任务个数
    inline const int taskNumber()
    {
        return m_queue.size();
    }

    //判断队列是否为空
    inline const bool empty()
    {
        return m_queue.empty();
    }

private:
    pthread_mutex_t m_mutex;
    std::queue<Task> m_queue;
};

#endif // _TASK_QUEUE_H_