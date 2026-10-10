// 管道的数据是单向流动的:
// 操作管道的是两个进程, 进程A读管道, 需要关闭管道的写端, 进程B写管道, 需要关闭管道的读端
// 如果不做上述的操作, 会对程序的结果造成一些影响, 对管道的操作无法结束
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main()
{
    // 1. 创建匿名管道, 得到两个文件描述符
    int fd[2];
    int ret = pipe(fd);
    if(ret == -1)
    {
        perror("pipe");
        exit(0);
    }
    // 2. 创建子进程 -> 能够操作管道的文件描述符被复制到子进程中
    pid_t pid = fork();
    if(pid == 0)
    {
        // 关闭读端
        close(fd[0]);
        // 3. 在子进程中执行 execlp("ps", "ps", "aux", NULL);
        // 在子进程中完成输出的重定向, 原来输出到终端现在要写管道
        // 进程打印数据默认输出到终端, 终端对应的文件描述符: stdout_fileno
        // 标准输出 重定向到 管道的写端
        dup2(fd[1], STDOUT_FILENO);
        execlp("ps", "ps", "aux", NULL);
        perror("execlp");
    }

    // 4. 父进程读管道
    else if(pid > 0)
    {
        // 关闭管道的写端
        close(fd[1]);
        // 5. 父进程打印读到的数据信息
        char buf[4096];
        // 读管道
        // 如果管道中没有数据, read会阻塞
        // 有数据之后, read解除阻塞, 直接读数据
        // 需要循环读数据, 管道是有容量的, 写满之后就不写了
        // 数据被读走之后, 继续写管道, 那么就需要再继续读数据
        while(1)
        {
            memset(buf, 0, sizeof(buf));
            int len = read(fd[0], buf, sizeof(buf));
            if(len == 0)
            {
                // 管道的写端关闭了, 如果管道中没有数据, 管道读端不会阻塞
                // 没数据直接返回0, 如果有数据, 将数据读出, 数据读完之后返回0
                break;
            }
            printf("%s, len = %d\n", buf, len);
        }
        close(fd[0]);

        // 回收子进程资源
        wait(NULL);
    }
    return 0;
}


