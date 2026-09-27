#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>


int main(int argc, char const *argv[])
{   
    int fd = open("1.txt",O_CREAT | O_WRONLY | O_APPEND,0644);
    pid_t pid = fork();
    if(pid<0){
        printf("创建失败");
        return 1;
    }else if(pid == 0){
        printf("子进程:%d\n",getpid());
    }else{
        printf("父进程:%d\n",getpid());
    }
    return 0;
}


