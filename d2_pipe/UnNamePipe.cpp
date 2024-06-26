/**
 * 匿名管道
 * 管道的是进程间通信（IPC - InterProcess Communication）的一种方式，管道的本质其实就是内核
 * 中的一块内存(或者叫内核缓冲区)，这块缓冲区中的数据存储在一个环形队列中，因为管道在内核里边，
 * 因此我们不能直接对其进行任何操作。
 * 因为管道数据是通过队列来维护的，我们先来分析一个管道中数据的特点：
 *      管道对应的内核缓冲区大小是固定的，默认为4k（也就是队列最大能存储4k数据）
 *      管道分为两部分：读端和写端（队列的两端），数据从写端进入管道，从读端流出管道
 *      管道中的数据只能读一次，做一次读操作之后数据也就没有了（读数据相当于出队列）
 *      管道是单工的：数据只能单向流动, 数据从写端流向读端
 *      对管道的操作（读、写）默认是阻塞的
 *      可以使用fcntl函数来修改管道容量
 * 
 * 匿名管道：
 *      匿名管道只能在具有亲缘关系的进程间使用，也就是父子进程
 * fd[0]:读 fd[1]:写
 * 写数据时要关闭读端
 * 读数据时要关闭写端

*/

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

using namespace std;

int main()
{
    int pipefd[2];
    char buf[1024];

    //创建管道
    if (pipe(pipefd) == -1)
    {
        cout << "pipe error" << endl;
        return -1;
    }

    cout << "parent process id:" << getpid() << endl;

    for (int i = 0; i < 8; i++)
    {
        //创建子进程
        pid_t pid = fork();
        if(pid == -1)
        {
            perror("fork error\n");
            return -1;
        }
        else if(pid == 0)
        {
            //子进程关闭写端
            close(pipefd[1]);
            //从管道读取数据
            memset(buf, 0, 1024);
            //从读端读数据
            read(pipefd[0], buf, sizeof(buf));
            //打印数据
            cout << "child process:" << getpid() << " read data: " << buf << endl;
            //子进程关闭读端
            close(pipefd[0]);
            //子进程退出
            return 1;
        }
    }

    //父进程继续执行下面代码
    cout << "process id:" << getpid() << endl;
    //父进程关闭读端
    close(pipefd[0]);
    //向管道写入数据
    const char* message = "hello from parent process";
    for (int i = 0; i < 8; i++)
    {
        write(pipefd[1], message, strlen(message));
        sleep(1);
    }
    
    //父进程关闭写端
    close(pipefd[1]);

    return 0;
}