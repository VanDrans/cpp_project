#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


int main(){
    int fd = open("be.txt",O_RDONLY);
    if(fd == -1){
        printf("打开文件失败\n");
        exit(EXIT_FAILURE);
    }
    char buffer[1024];

    //ssize_t read (int __fd, void *__buf, size_t __nbytes)
    int byte_read = 0;
    while ((byte_read = read(fd,buffer,sizeof(buffer)))>0)
    {   

        write(STDOUT_FILENO,buffer,byte_read);
    }


    return 0;
}