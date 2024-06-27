/**
 * 线程分离
 * 
 * 在某些情况下，程序中的主线程有属于自己的业务处理流程，如果让主线程负责子线程的资源回收，
 * 调用pthread_join()只要子线程不退出主线程就会一直被阻塞，主要线程的任务也就不能被执行了。
 * 在线程库函数中为我们提供了线程分离函数pthread_detach()，调用这个函数之后指定的子线程就可以和主线程分离，
 * 当子线程退出的时候，其占用的内核资源就被系统的其他进程接管并回收了。
 * 线程分离之后在主线程中使用pthread_join()就回收不到子线程资源了。
 * 
 * #include <pthread.h>
 * int pthread_detach(pthread_t thread);
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <iostream>

using namespace std;

//子线程处理函数
void* working(void* arg)
{
    cout << "thread is working id:" << pthread_self() << endl;
    //让主线程先退出
    sleep(3);
    for (int i = 0; i < 9; i++)
    {
        printf("child == i: = %d\n", i);
    }
    
    return NULL;
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, NULL, working, NULL);
    cout << "son create success id:" << tid << " main id:" << pthread_self() << endl;

    for(int i=0; i<3; ++i) 
    {
        printf("i = %d\n", i);
    }

    //设置子线程和主线程分离
    pthread_detach(tid);
    //主线程退出
    pthread_exit(NULL);

    return 0;
}