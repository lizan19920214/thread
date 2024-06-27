/**
 * 互斥锁
 * 互斥锁是线程同步最常用的一种方式，通过互斥锁可以锁定一个代码块, 被锁定的这个代码块, 
 * 所有的线程只能顺序执行(不能并行处理)，这样多线程访问共享资源数据混乱的问题就可以被解决了，
 * 需要付出的代价就是执行效率的降低，因为默认临界区多个线程是可以并行处理的，现在只能串行处理。
 * 
 * 在Linux中互斥锁的类型为pthread_mutex_t，创建一个这种类型的变量就得到了一把互斥锁：
 * pthread_mutex_t  mutex;
 * 
 * 
 * 在创建的锁对象中保存了当前这把锁的状态信息：锁定还是打开，如果是锁定状态还记录了给这把锁加锁的线程信息（线程ID）。
 * 一个互斥锁变量只能被一个线程锁定，被锁定之后其他线程再对互斥锁变量加锁就会被阻塞，直到这把互斥锁被解锁，
 * 被阻塞的线程才能被解除阻塞。一般情况下，每一个共享资源对应一个把互斥锁，锁的个数和线程的个数无关。
 * 
 * 初始化锁：
 * int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restrict attr);
 * mutex 是一个指向互斥锁对象的指针，用于指定要初始化的互斥锁。
 * attr 是一个指向互斥锁属性对象的指针，用于指定互斥锁的属性。如果将 attr 参数设置为 NULL，则使用默认的互斥锁属性。
 * 
 * 加锁：
 * int pthread_mutex_lock(pthread_mutex_t *mutex);
 * 这个函数被调用, 首先会判断参数 mutex 互斥锁中的状态是不是锁定状态:
 * 没有被锁定, 是打开的, 这个线程可以加锁成功, 这个这个锁中会记录是哪个线程加锁成功了
 * 如果被锁定了, 其他线程加锁就失败了, 这些线程都会阻塞在这把锁上
 * 当这把锁被解开之后, 这些阻塞在锁上的线程就解除阻塞了，并且这些线程是通过竞争的方式对这把锁加锁，没抢到锁的线程继续阻塞
 * 
 * 尝试加锁：
 * int pthread_mutex_trylock(pthread_mutex_t *mutex);
 * 调用这个函数对互斥锁变量加锁还是有两种情况:
 * 如果这把锁没有被锁定是打开的，线程加锁成功
 * 如果锁变量被锁住了，调用这个函数加锁的线程，不会被阻塞，加锁失败直接返回错误号，相当于加锁的非阻塞版本
 * 
 * 解锁
 * int pthread_mutex_unlock(pthread_mutex_t *mutex);
 * 调用这个函数对互斥锁变量解锁，解锁之后，阻塞在这个锁上的线程就可以解除阻塞了
 * 不是所有的线程都可以对互斥锁解锁，哪个线程加的锁, 哪个线程才能解锁成功。
 * 
 * 销毁锁
 * int pthread_mutex_destroy(pthread_mutex_t *mutex);
 * 调用这个函数可以销毁一个互斥锁对象，销毁之后，这个互斥锁对象就不能再使用了
 * 释放锁和解锁不一样，解锁的话锁还在，只是解开了，释放锁是将锁占的系统资源释放，锁就不存在了。
 * 
 * 
 * 死锁的产生
 * 当多个线程访问共享资源, 需要加锁, 如果锁使用不当, 就会造成死锁这种现象。如果线程死锁造成的后果是：
 * 所有的线程都被阻塞，并且线程的阻塞是无法解开的（因为可以解锁的线程也被阻塞了）。
 * 1、加锁后不解锁
 *      那么只能有一个线程拥有锁，且这个线程退出后，也没有线程能拿到锁
 * 2、重复加锁
 *      第二次加锁的时候，发现已经加锁了，当前线程被阻塞，等待解锁
 *      问题是加锁的是当前线程，等待解锁的也是当前线程，死锁！
 * 3、多个线程互相等待阻塞
 *      有两个共享资源:X, Y，X对应锁A, Y对应锁B
 *          线程A访问资源X, 加锁A
 *          线程B访问资源Y, 加锁B
 *      线程A要访问资源Y, 线程B要访问资源X，因为资源X和Y已经被对应的锁锁住了，因此这个两个线程被阻塞
 *          线程A被锁B阻塞了, 无法打开A锁
 *          线程B被锁A阻塞了, 无法打开B锁
 * 
 * 
*/

#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>

using namespace std;

#define NUM_THREADS 5

//创建全局互斥锁
pthread_mutex_t mutex_counter;
//创建计数器
int counter = 0;

//线程函数，对计数器进行增加
void* thread_function(void* arg)
{
    int* index = (int*)arg;
    //对计数器进行增加
    for (int i = 0; i < 10000; i++)
    {
        //获取互斥锁
        pthread_mutex_lock(&mutex_counter);
        //操作共享资源
        counter ++;
        cout << "thread index:" << *index << " count:" << counter <<endl;
        //释放互斥锁
        pthread_mutex_unlock(&mutex_counter);
        sleep(1);
    }
    
    return NULL;
}

int main()
{
    //线程
    pthread_t threads[NUM_THREADS];
    int index[NUM_THREADS];
    //初始化线程锁
    pthread_mutex_init(&mutex_counter, NULL);

    //创建线程
    for (int i = 0; i < NUM_THREADS; i++)
    {
        index[i] = i;
        pthread_create(&threads[i], NULL, thread_function, &index[i]);
    }

    //等待线程结束
    for (int i = 0; i < 2; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    //输出结果
    cout << "counter = " << counter << endl;
    
    //销毁锁
    pthread_mutex_destroy(&mutex_counter);
    
    return 0;
}