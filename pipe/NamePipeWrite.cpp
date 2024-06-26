/**
 * 有名管道
 * 有名管道拥有管道的所有特性，之所以称之为有名是因为管道在磁盘上有实体文件, 文件类型为p ，
 * 有名管道文件大小永远为0，因为有名管道也是将数据存储到内存的缓冲区中，打开这个磁盘上的管道文件
 * 就可以得到操作有名管道的文件描述符，通过文件描述符读写管道存储在内核中的数据。
 * 有名管道也可以称为 fifo (first in first out)，使用有名管道既可以进行有血缘关系的进程间通信
 * ，也可以进行没有血缘关系的进程间通信。创建有名管道的方式有两种，一种是通过命令，一种是通过函数。
 * 
 * 创建：
 * mkfifo 有名管道的名字
 * 
 * #include <sys/types.h>
 * #include <sys/stat.h>
 * int mkfifo(const char *pathname, mode_t mode);
 * 
 * pathname: 要创建的有名管道的名字
 * mode: 文件的操作权限, 和open()的第三个参数一个作用，最终权限: (mode & ~umask)
 * 返回值：创建成功返回 0，失败返回 -1
 * 
 * 不管是有血缘关系还是没有血缘关系，使用有名管道实现进程间通信的方式是相同的，就是在两个进程中分别以读、
 * 写的方式打开磁盘上的管道文件，得到用于读管道、写管道的文件描述符，就可以调用对应的read()、write()函数进行读写操作了。
 * 
 * 有名管道操作需要通过 open() 操作得到读写管道的文件描述符，如果只是读端打开了或者只是写端打开了，
 * 进程会阻塞在这里不会向下执行，直到在另一个进程中将管道的对端打开。
 * 
 * 只读open要阻塞到某个进程为写而打开这个FIFO，
 * 同样只写open要阻塞到阻塞某个进程为度而打开它。
 * 
 * open
 * 在没有读者的情况下，非阻塞写操作不能成功
 * 如果有读者在阻塞等待，那么非阻塞写将成功，读者也能成功读到数据
 * 
*/

#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


using namespace std;

int main()
{
    //创建有名管道
    int ret = mkfifo("./testfifo", 0664);
    if (ret == -1)
    {
        perror("mkfifo error");
        exit(0);
    }

    cout << "mkfifo success" << endl;

    //以只写的权限打开管道文件,会阻塞在这里
    //只读open要阻塞到某个进程为写而打开这个FIFO，
    //同样只写open要阻塞到阻塞某个进程为读而打开它。
    int wfd = open("./testfifo", O_WRONLY);
    if (wfd == -1)
    {
        perror("open error");
        exit(0);
    }

    cout << "open write success" << endl;

    //循环写入管道
    int i = 0;
    while(i < 100)
    {
        char buf[1024];
        sprintf(buf, "hello, fifo, writing:%d\n", i);
        cout << "write buff:" << buf << endl;
        write(wfd, buf, strlen(buf));
        i++;
        sleep(1);
    }

    //关闭管道文件
    close(wfd);
    
    return 0;
}