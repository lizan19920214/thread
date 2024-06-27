/**
 * g++ -o ThreadCreate ThreadCreate.cpp -lpthread
 * 
 * 线程创建：
 * #include <pthread.h>
 * int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
 * 
 * thread: 传出参数，是无符号长整形数，线程创建成功, 会将线程ID写入到这个指针指向的内存中
 * attr: 线程的属性, 一般情况下使用默认属性即可, 写NULL
 * start_routine: 函数指针，创建出的子线程的处理动作，也就是该函数在子线程中执行。
 * arg: 作为实参传递到 start_routine 指针指向的函数内部
 * 
 * 返回值：线程创建成功返回0，创建失败返回对应的错误号
 * 
 * 获得当前线程id
 * pthread_t pthread_self(void);
 * 
 * 虚拟地址空间的生命周期和主线程是一样的，与子线程无关
 * 
 * 
 * 线程退出
 * #include <pthread.h>
 * void pthread_exit(void *retval);
 * retval: 线程退出的时候携带的数据，当前子线程的主线程会得到该数据。如果不需要使用，指定为NULL
 * 只要调用该函数当前线程就马上退出了，并且不会影响到其他线程的正常运行，不管是在子线程或者主线程中都可以使用
 * 
 * 
 * 
 * 
 * 
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

using namespace std;

//线程处理函数
void* working(void* arg)
{
    cout << "son thread id:" << pthread_self() << endl;
    for (int i = 0; i < 9; i++)
    {
        cout << "son thread:" << i << endl;
    }

    return nullptr;
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, NULL, working, NULL);

    cout << "create thread success tid:" << tid << endl;
    cout << "main thread id:" << pthread_self() << endl;

    for (int i = 0; i < 3; i++)
    {
        cout << "main thread:" << i << endl;
        //这里注释掉运行可以看到，子线程的输出还没有执行完毕，主线程直接退出了，虚拟地址空间就被释放了, 
        // 子线程就一并被销毁了。
        // 但是如果某一个子线程退出了, 主线程仍在运行, 虚拟地址空间依旧存在。
        // sleep(1);
    }
    
    return 0;
}