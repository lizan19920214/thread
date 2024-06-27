/**
 * 线程取消函数
 * 线程取消的意思就是在某些特定情况下在一个线程中杀死另一个线程。使用这个函数杀死一个线程需要分两步：
 * 在线程A中调用线程取消函数pthread_cancel，指定杀死线程B，这时候线程B是死不了的
 * 在线程B中进行一次系统调用（从用户区切换到内核区），否则线程B可以一直运行。
 * 
 * 有一个解释很形象：这其实和七步断肠散、含笑半步癫的功效是一样的，吃了毒药不动或者不笑也没啥事儿
 * #include <pthread.h>
 * // 参数是子线程的线程ID
 * int pthread_cancel(pthread_t thread);
 * thread：要杀死的线程的线程ID
 * 
 * 关于系统调用有两种方式：
 * 直接调用Linux系统函数
 * 调用标准C库函数，为了实现某些功能，在Linux平台下标准C库函数会调用相关的系统函数
 * 
 * 线程比较：
 * 在Linux中线程ID本质就是一个无符号长整形，因此可以直接使用比较操作符比较两个线程的ID，但是线程库是可以跨平台使用的，在某些平台上 pthread_t可能不是一个单纯的整形，这中情况下比较两个线程的ID必须要使用比较函数，函数原型如下：
 * #include <pthread.h>
 * int pthread_equal(pthread_t t1, pthread_t t2);
 * 
 * t1 和 t2 是要比较的线程的线程ID
 * 返回值：如果两个线程ID相等返回非0值，如果不相等返回0
 * 
 * 
 * 
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <iostream>

using namespace std;

//子线程处理函数
void* working(void* arg)
{
    //该函数会调用系统函数，因此是一个间接的系统调用
    printf("我是子线程, 线程ID: %ld\n", pthread_self());
    for (int i = 0; i < 9; i++)
    {
        printf(" child i: %d\n", i);
    }
    
    return NULL;
}

int main()
{

    //创建一个子线程
    pthread_t tid;
    pthread_create(&tid, NULL, working, NULL);
    cout << "child success tid:" << tid << endl;

    //主线程继续执行下面代码
    cout << "main id:" << pthread_self() << endl;

    for (int i = 0; i < 3; i++)
    {
        cout << "main i:" << i << endl;
    }

    //杀死子线程，如果子线程中做系统调用，子线程就结束了
    pthread_cancel(tid);
    
    //主线程自己退出
    pthread_exit(NULL);

    return 0;
}