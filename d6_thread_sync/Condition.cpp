/**
 * 条件变量
 * 严格意义上来说，条件变量的主要作用不是处理线程同步, 而是进行线程的阻塞。
 * 如果在多线程程序中只使用条件变量无法实现线程的同步, 必须要配合互斥锁来使用。
 * 
 * 一般情况下条件变量用于处理生产者和消费者模型，并且和互斥锁配合使用。
 * 条件变量类型对应的类型为pthread_cond_t，这样就可以定义一个条件变量类型的变量了：
 * pthread_cond_t cond;
 * 被条件变量阻塞的线程的线程信息会被记录到这个变量中，以便在解除阻塞的时候使用。
 * 
 * 初始化：
 *      int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr);
 *      cond: 条件变量的地址
 *      attr: 条件变量属性, 一般使用默认属性, 指定为NULL
 * 
 * 释放：
 *      int pthread_cond_destroy(pthread_cond_t *cond);
 *      销毁条件变量以释放资源
 * 
 * 线程阻塞函数：
 *      int pthread_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict mutex);
 *      通过函数原型可以看出，该函数在阻塞线程的时候，需要一个互斥锁参数，这个互斥锁主要功能是进行线程同步，
 *      让线程顺序进入临界区，避免出现数共享资源的数据混乱。该函数会对这个互斥锁做以下几件事情：
 *          在阻塞线程时候，如果线程已经对互斥锁mutex上锁，那么会将这把锁打开，这样做是为了避免死锁
 *          ！！！！！当线程解除阻塞的时候，函数内部会帮助这个线程再次将这个mutex互斥锁锁上，继续向下访问临界区
 * 
 * 线程唤醒函数：
 *      // 唤醒阻塞在条件变量上的线程, 至少有一个被解除阻塞
 *      int pthread_cond_signal(pthread_cond_t *cond);
 *      // 唤醒阻塞在条件变量上的线程, 被阻塞的线程全部解除阻塞
 *      int pthread_cond_broadcast(pthread_cond_t *cond);
 * 
 * 调用上面两个函数中的任意一个，都可以换线被pthread_cond_wait或者pthread_cond_timedwait阻塞的线程，
 * 区别就在于pthread_cond_signal是唤醒至少一个被阻塞的线程（总个数不定），pthread_cond_broadcast是唤醒所有被阻塞的线程。
 * 
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>

using namespace std;

//缓冲区大小
#define BUFFER_SIZE 5

int buffer[BUFFER_SIZE];
//缓冲区当前元素个数
int count = 0;
//生产者放入数据的位置
int in = 0;
//消费者取出数据的位置
int out =0;

//初始化互斥锁和两个条件变量
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;

//生产者数量
#define PRODUCER_NUM 3
//消费者数量
#define CONSUMER_NUM 2


//生产者
void* producer(void* arg)
{
    int index = *((int*)arg);
    int item;
    while(1)
    {
        item = rand() % 100; //产生一个随机数

        //加锁
        pthread_mutex_lock(&mutex);

        while(count == BUFFER_SIZE)
        {
            //如果缓冲区已经满了，那么生产者线程阻塞等待,当缓冲区为空时再次生产
            cout << "producer buff is full ,waiting...." << endl;

            //注意这里wait等待，再次被唤醒时，会重新lock上mutex
            pthread_cond_wait(&not_full, &mutex);
        }

        buffer[in] = item;
        cout << "producer index:" << index << " item:" << item << " count:" << count << endl;

        in = (in + 1) % BUFFER_SIZE;
        count++;

        pthread_mutex_unlock(&mutex);

        //通知消费者缓冲区有数据了
        pthread_cond_signal(&not_empty);

        sleep(1);
    }
    
    return NULL;
}

//消费者
void* consumer(void* arg)
{
    int index = *((int*)arg);
    int item;
    while(1)
    {
        //加锁
        pthread_mutex_lock(&mutex);

        while(count == 0)
        {
            //如果缓冲区为空，那么消费者线程阻塞等待，当缓冲区有数据时再次消费
            cout << "consumer buff is empty ,waiting...." << endl;
            pthread_cond_wait(&not_empty, &mutex);
        }

        item = buffer[out];
        cout<< "consumer index:" << index << "->>>>>>>>>>>>>>>>>>>>>>>> item:" << item << " count:" << count << endl;

        out = (out + 1) % BUFFER_SIZE;
        count--;

        pthread_mutex_unlock(&mutex);

        //通知生产者缓冲区有空位了
        pthread_cond_signal(&not_full);

        sleep(1);
    }

    return NULL;
}

int main()
{
    //生产者和消费者线程
    pthread_t producer_thread[PRODUCER_NUM];
    pthread_t consumer_thread[CONSUMER_NUM];

    //记录索引
    int producer_index[PRODUCER_NUM];
    int consumer_index[CONSUMER_NUM];

    for (int i = 0; i < PRODUCER_NUM; i++)
    {
        producer_index[i] = i + 1;
        pthread_create(&producer_thread[i], NULL, producer, &producer_index[i]);
    }
    
    for (int i = 0; i < CONSUMER_NUM; i++)
    {
        consumer_index[i] = i + 1;
        pthread_create(&consumer_thread[i], NULL, consumer, &consumer_index[i]);
    }
    

    //等待线程结束
    for (int i = 0; i < PRODUCER_NUM; i++)
    {
        pthread_join(producer_thread[i], NULL);
    }
    
    for (int i = 0; i < CONSUMER_NUM; i++)
    {
        pthread_join(consumer_thread[i], NULL);
    }
    

    return 0;
}