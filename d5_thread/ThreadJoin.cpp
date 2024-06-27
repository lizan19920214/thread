/**
 * 线程回收
 * 线程和进程一样，子线程退出的时候其内核资源主要由主线程回收，线程库中提供的线程回收函数叫做
 * pthread_join()，这个函数是一个阻塞函数，如果还有子线程在运行，
 * 调用该函数就会阻塞，子线程退出函数解除阻塞进行资源的回收，函数被调用一次，
 * 只能回收一个子线程，如果有多个子线程则需要循环进行回收。
 * 
 * 另外通过线程回收函数还可以获取到子线程退出时传递出来的数据，函数原型如下：
 * 
 * #include <pthread.h>
 * // 这是一个阻塞函数, 子线程在运行这个函数就阻塞
 * // 子线程退出, 函数解除阻塞, 回收对应的子线程资源, 类似于回收进程使用的函数 wait()
 * int pthread_join(pthread_t thread, void **retval);
 *      thread: 要被回收的子线程的线程ID
 *      retval: 二级指针, 指向一级指针的地址, 是一个传出参数, 这个地址中存储了pthread_exit() 传递出的数据，
 *              如果不需要这个参数，可以指定为NULL
 * 
 * 通过函数pthread_exit(void *retval);可以得知，子线程退出的时候，需要将数据记录到一块内存中，通过参数传出的是
 * 存储数据的内存的地址，而不是具体数据，由因为参数是void*类型，所有这个万能指针可以指向任意类型的内存地址
 * 
 * 注意不能使用子线程栈来传出参数，因为在退出时，子线程资源会销毁，返回出来的地址会是一个野指针
 * 推荐使用全局变量或者主线程资源来传出退出值
 * 
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <iostream>

using namespace std;

//定义结构体
struct Person
{
    int id;
    int age;
};

struct Person p;	// 定义全局变量

//子线程处理函数
void* working1(void* arg)
{
    cout << "son thread id:" << pthread_self() << endl;

    for (int i = 0; i < 9; i++)
    {
        cout << "son working:" << i << endl;
        if (i == 6)
        {
            //这里不能使用栈空间，退出时栈空间被释放，主线程无法获得地址，返回野指针
            p.age = 11;
            p.id = 110;
            //带参退出，将p的地址传给主线程的pthread_join
            pthread_exit(&p);
        }
    }
    
    return NULL;
}

//使用全局变量退出
void TestExitGlobal()
{
    //创建子线程
    pthread_t tid;
    pthread_create(&tid, NULL, working1, NULL);
    cout << "pthread_create success tid:" << tid << " main id:" << pthread_self() << endl;
    for (int i = 0; i < 3; i++)
    {
        cout << " main i:" << i << endl;
    }

    //阻塞等待子进程退出
    void* ptr = NULL;
    // ptr是一个传出参数, 在函数内部让这个指针指向一块有效内存
    // 这个内存地址就是pthread_exit() 参数指向的内存
    pthread_join(tid, &ptr);
    //打印子进程退出消息
    struct Person* p = (struct Person*)ptr;
    cout << "son thread exit, id:" << p->id << " age:" << p->age << endl;

    cout << "son recycle success" << endl;
}

void* working2(void* arg)
{
    struct Person* pp = (struct Person*)arg;
    cout << "son thread id:" << pthread_self() << endl;

    for (int i = 0; i < 9; i++)
    {
        cout << "son working:" << i << endl;
        if (i == 6)
        {
            //这里不能使用栈空间，退出时栈空间被释放，主线程无法获得地址，返回野指针
            pp->age = 22;
            pp->id = 120;
            //带参退出，将p的地址传给主线程的pthread_join
            pthread_exit(pp);
        }
    }
    
    return NULL;
}

//使用主线程栈退出
void TestExitMain()
{
    //创建子线程
    pthread_t tid;
    struct Person pp;
    pthread_create(&tid, NULL, working2, &pp);
    cout << "pthread_create success tid:" << tid << " main id:" << pthread_self() << endl;
    for (int i = 0; i < 3; i++)
    {
        cout << " main i:" << i << endl;
    }

    //阻塞等待子进程退出
    void* ptr = NULL;
    // ptr是一个传出参数, 在函数内部让这个指针指向一块有效内存
    // 这个内存地址就是pthread_exit() 参数指向的内存
    pthread_join(tid, &ptr);
    //打印子进程退出消息
    struct Person* p = (struct Person*)ptr;
    cout << "son thread exit, id:" << p->id << " age:" << p->age << endl;

    cout << "son recycle success" << endl;
}

int main()
{
    TestExitGlobal();
    // TestExitMain();
    return 0;
}