#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <iostream>

using namespace std;

int main()
{
    //以只读方式打开管道
    int rfd = open("./testfifo", O_RDONLY);
    if (rfd == -1)
    {
        perror("open error");
        exit(0);
    }

    //循环读取
    while(1)
    {
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        //读是阻塞的，如果管道中没有数据，read自动阻塞
        int len = read(rfd, buf, sizeof(buf));
        cout << "read buf: " << buf << endl;

        if (len == 0)
        {
            //写端关闭
            printf("read end\n");
            break;
        }
    }
    
    close(rfd);

    return 0;
}