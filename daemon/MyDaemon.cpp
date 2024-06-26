/**
 * 守护进程
 * 守护进程（Daemon Process），也就是通常说的 Daemon 进程（精灵进程），
 * 是 Linux 中的后台服务进程。它是一个生存期较长的进程，通常独立于控制终端并且周期性地执行某种任务
 * 或等待处理某些发生的事件。一般采用以d结尾的名字。
 * 
 * 
 * 进程组
 * 多个进程的集合就是进程组, 这个组中必须有一个组长, 组长就是进程组中的第一个进程，
 * 组长以外的都是普通的成员，每个进程组都有一个唯一的组ID，进程组的ID和组长的PID是一样的。
 * 
 * 进程组中的成员是可以变动的，如果当前进程组中的成员被转移到了其他组，或者所有进程都退出了，
 * 那么进程组也就不存在了。如果进程组中组长死了, 但是当前进程组中有其他进程，这个进程组还是继续存在的。
 * 
 * 获得进程组id
 * pid_t getpgrp(void);
 * 获得指定进程所在进程组的组ID
 * pid_t getpgid(pid_t pid);
 * pid为指定进程的PID
 * 
 * 创建新的组或者移动某个进程
 * int setpgid(pid_t pid, pid_t pgid);
 * pid: 某个进程的进程ID
 * pgid: 某个进程组的组ID
 *      如果pgid对应的进程组存在，pid对应的进程会移动到这个组中, pid != pgid
 *      如果pgid对应的进程组不存在，会创建一个新的进程组, 因此要求 pid == pgid, 当前进程就是组长了
 * 
 * 
 * 会话(session)是由一个或多个进程组组成的，一个会话可以对应一个控制终端, 也可以没有。
 * 一个普通的进程可以调用 setsid 函数使自己成为新 session 的领头进程（会长）
 * ，并且这个 session 领头进程还会被放入到一个新的进程组中
 * 
 * 
 * #include <unistd.h>
 * // 获取某个进程所属的会话ID
 * pid_t getsid(pid_t pid);
 * // 将某个进程变成会话 =>> 得到一个守护进程
 * pid_t setsid(void);
 * 
 * 创建守护进程步骤
 * 1、创建子进程, 让父进程退出
 *      因为父进程有可能是组长进程，不符合条件，也没有什么利用价值，退出即可
 *      子进程没有任何职务, 目的是让子进程最终变成一个会话, 最终就会得到守护进程
 * 
 * 2、通过子进程创建新的会话，调用函数 setsid，脱离控制终端, 变成守护进程
 * 
 * 3、改变当前进程的工作目录（非必须）
 *      修改当前进程的工作目录需要调用函数 chdir
 *      int chdir(const char *path);
 * 
 * 4、重新设置文件的掩码（非必须）
 *      掩码: umask, 在创建新文件的时候需要和这个掩码进行运算, 去掉文件的某些权限
 *      mode_t umask(mode_t mask);
 * 
 * 5、关闭/重定向文件描述符（非必须）
 *      标准输入，标准输出，标准错误，这三个文件描述符对应的都是当前终端，由于执行 setsid调用，
 *      当前进程已经脱离了终端，因此关联的文件描述符也就没用了，可以关闭
 *          close(STDIN_FILENO);
 *          close(STDOUT_FILENO);
 *          close(STDERR_FILENO);
 *      重定向文件描述符(和关闭二选一): 改变文件描述符关联的默认文件, 让他们指向一个特殊的文件/dev/null，
 *      只要把数据扔到这个特殊的设备文件中, 数据被被销毁了
 *          int fd = open("/dev/null", O_RDWR);
 *          dup2(fd, STDIN_FILENO);
 *          dup2(fd, STDOUT_FILENO);
 *          dup2(fd, STDERR_FILENO);
 * 
 * 
*/

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

using namespace std;

// 信号的处理动作
void writeFile(int num) 
{
    // 得到系统时间
    time_t seconds = time(NULL);
    // 时间转换, 总秒数 -> 可以识别的时间字符串
    struct tm* loc = localtime(&seconds);
    // sprintf();
    char* curtime = asctime(loc);
    // 文件权限 0664 & ~022
    int fd = open("./time.log", O_WRONLY|O_CREAT|O_APPEND, 0664);
    // 写入文件
    write(fd, curtime, strlen(curtime));
    // 关闭文件
    close(fd);
}


int main()
{
    pid_t pid = fork();
    if (pid > 0)
    {
        //父进程退出
        exit(0);
    }

    //子进程设置为守护进程
    setsid();

    //修改进程的工作目录
    chdir("/home/lizan");

    //设置掩码，在进程中创建文件的时候这个掩码就起作用了
    umask(022);

    //重定向和终端关联的文件描述符
    int fd = open("/dev/null", O_RDWR);
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);


    //委托内核捕捉并处理将来发生的信号SIGALRM(14)
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = writeFile;

    sigemptyset(&act.sa_mask);
    sigaction(SIGALRM, &act, NULL);

    //设置定时器
    struct itimerval value;
    value.it_value.tv_sec = 2;
    value.it_value.tv_usec = 0;
    value.it_interval.tv_sec = 2;
    value.it_interval.tv_usec = 0;

    //现在的系统中很多程序不再使用alarm调用，而是使用setitimer调用来设置定时器，定时器到的时候会发出SIGALRM信号
    //触发上面的信号捕捉函数
    setitimer(ITIMER_REAL, &value, NULL);

    // 守护进程的无限循环
    while (1)
    {
        sleep(10000);
    }
    

    return 0;
}