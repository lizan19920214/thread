/**
 * 孤儿进程：
 * 在一个启动的进程中创建子进程，这时候父子进程同时运行，
 * 但是父进程由于某种原因先退出了，子进程还在运行，这时候这个子进程就可以被称之为孤儿进程
 * 操作系统当检测到某一个进程变成了孤儿进程，
 * 这时候系统中就会有一个固定的进程领养这个孤儿进程（有干爹了）。
 * 如果使用Linux没有桌面终端，这个领养孤儿进程的进程就是 init 进程（PID=1）也就是第一个进程，
 * 如果有桌面终端，这个领养孤儿进程就是桌面进程。
 * 系统为什么要领养这个孤儿进程呢？在子进程退出的时候,
 *  进程中的用户区可以自己释放, 但是进程内核区的pcb资源自己无法释放，
 * 必须要由父进程来释放子进程的pcb资源，孤儿进程被领养之后，这件事儿干爹就可以代劳了，这样可以避免系统资源的浪费。
 * 
 * 
 * 僵尸进程：
 * 在一个启动的进程中创建子进程，这时候就有了父子两个进程，父进程正常运行, 子进程先与父进程结束, 子进程无法释放自己的PCB资源
 * , 需要父进程来做这个件事儿, 但是如果父进程也不管, 这时候子进程就变成了僵尸进程。
 * 僵尸进程不能将它看成是一个正常的进程，这个进程已经死亡了，用户区资源已经被释放了，只是还占用着一些内核资源（PCB）。
 * 消灭僵尸进程的方法是，杀死这个僵尸进程的父进程，这样僵尸进程的资源就被系统回收了。
 * 通过kill -9 僵尸进程PID的方式是不能消灭僵尸进程的，这个命令只对活着的进程有效，僵尸进程已经死了，是不能解决问题的。
 * 
 * 为了避免僵尸进程的产生，一般我们会在父进程中进行子进程的资源回收，
 * 回收方式有两种，一种是阻塞方式wait()，一种是非阻塞方式waitpid()。
 * 
 * wait是阻塞函数，如果没有子进程退出, 函数会一直阻塞等待, 当检测到子进程退出了, 该函数阻塞解除回收子进程资源。
 * 这个函数被调用一次, 只能回收一个子进程的资源，如果有多个子进程需要资源回收, 函数需要被调用多次。
 * #include <sys/wait.h>
 * pid_t wait(int *status);
 * status 是一个指向整数的指针，用于存储子进程的终止状态。如果不关心子进程的终止状态，可以将该参数设置为 NULL。
 * wait 函数返回已终止子进程的进程ID（PID），如果出错，则返回 -1。
 * 
 * 
 * waitpid() 函数可以看做是 wait() 函数的升级版，通过该函数可以控制回收子进程资源的方式是阻塞还是非阻塞，
 * 另外还可以通过该函数进行精准打击，可以精确指定回收某个或者某一类或者是全部子进程资源
 * #include <sys/wait.h>
 * pid_t waitpid(pid_t pid, int *status, int options);
 * 
 * 
 * pid:
 * -1：回收所有的子进程资源, 和wait()是一样的, 无差别回收，并不是一次性就可以回收多个, 也是需要循环回收的
 * 大于0：指定回收某一个进程的资源 ，pid是要回收的子进程的进程ID
 * 0：回收当前进程组的所有子进程ID
 * 小于 -1：pid 的绝对值代表进程组ID，表示要回收这个进程组的所有子进程资源
 * 
 * status: NULL, 和wait的参数是一样的
 * 
 * options: 控制函数是阻塞还是非阻塞
 * 0: 函数是行为是阻塞的 ==> 和wait一样
 * WNOHANG: 函数是行为是非阻塞的
 * 
 * 
 * 返回值:
 *  如果函数是非阻塞的, 并且子进程还在运行, 返回0
 *  成功: 得到子进程的进程ID
 *  失败: -1
 *      没有子进程资源可以回收了, 函数如果是阻塞的, 阻塞会解除, 直接返回-1
 *      回收子进程资源的时候出现了异常
 * 
 * 
 * 
 * 
 * 
*/
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <stdlib.h>

using namespace std;

void GuerThread()
{
    //创建子进程
    pid_t pid = fork();
    if(pid > 0)
    {
        printf("this is parent process, pid = %d, child pid = %d\n", getpid(), pid);
    }
    else if(pid == 0)
    {
        //休眠1秒等父进程退出，造成孤儿进程的产生
        sleep(1);
        printf("this is child process, pid = %d, ppid = %d\n", getpid(), getppid());

    }
}

void JiangshiThread()
{
    pid_t pid;
    //创建子进程
    for (int i = 0; i < 5; i++)
    {
        pid = fork();
        if(pid == 0)
        {
            break;
        }
    }
    
    //父进程
    if (pid > 0)
    {
        //父进程一直运行不退出，也不做回收，产生僵尸进程
        while(1)
        {
            cout << "pid:" << getpid() << endl;
            sleep(1);
        }
    }
    else if (pid == 0)
    {
        cout << "son out pid:" << getpid() << " parent:" << getppid() << endl;
    }
    
}

void waitThread()
{
    pid_t pid;
    int status;
    //创建子进程
    pid = fork();

    if (pid < 0)
    {
        cout << "fork error" << endl;
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        //等待5s，让父进程先进入wait阻塞等待
        sleep(5);
        //子进程执行代码
        cout << "child out pid:" << getpid() << " parent:" << getppid() << endl;
        exit(EXIT_SUCCESS);
    }
    else
    {
        //父进程等待子进程退出
        cout << "wait son out" << endl;
        wait(&status);
        cout << "child exit pid:" << pid << endl;
    }
}

void waitpidThread()
{
    pid_t pid;
    //创建子进程
    for (int i = 0; i < 5; i++)
    {
        pid = fork();
        if(pid == 0)
        {
            break;
        }
    }
    
    // 父进程
    if(pid > 0) 
    {
        while(1) 
        {
            int status;
            pid_t ret = waitpid(-1, &status, WNOHANG);  // 非阻塞
            if(ret > 0)
            {
                printf("成功回收了子进程资源, 子进程PID: %d\n", ret);
                if(WIFEXITED(status)) 
                {
                    printf("子进程退出时候的状态码: %d\n", WEXITSTATUS(status));
                }
                if(WIFSIGNALED(status)) 
                {
                    printf("子进程是被这个信号杀死的: %d\n", WTERMSIG(status));
                }
            }
            else if(ret == 0) 
            {
                printf("子进程还没有退出, 不做任何处理...\n");
            }
            else 
            {
                printf("回收失败, 或者是已经没有子进程了...\n");
                break;
            }
            printf("我是父进程, pid=%d\n", getpid());
        }
    }
    // 子进程
    else if(pid == 0) 
    {
        printf("===我是子进程, pid=%d, 父进程ID: %d\n", getpid(), getppid());
    }
    
}

int main()
{
    // GuerThread();
    // JiangshiThread();
    // waitThread();
    waitpidThread();
    return 0;
}