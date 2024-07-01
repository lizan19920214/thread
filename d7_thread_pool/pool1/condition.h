#ifndef _CONDITION_H_
#define _CONDITION_H_

#include <pthread.h>

typedef struct condition
{
    pthread_mutex_t pmutex;
    pthread_cond_t pcond;
}condition_t;

//状态操作函数
//初始化
int condition_init(condition_t* cond);
//加锁
int condition_lock(condition_t* cond);
//解锁
int condition_unlock(condition_t* cond);
//等待
int condition_wait(condition_t* cond);
//定时等待
int condition_timedwait(condition_t* cond, const struct timespec* abstime);
//唤醒一个睡眠线程
int condition_signal(condition_t* cond);
//唤醒所有睡眠线程
int condition_broadcast(condition_t* cond);
//销毁释放
int condition_destroy(condition_t* cond);

#endif // !_CONDITION_H_