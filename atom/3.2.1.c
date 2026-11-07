
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <errno.h> 
#include <string.h> 
 
int main(void)
{
    int fd;

    fd = open("./dwd",O_RDONLY);

    if(fd == -1){
        printf("Error:%s\n",strerror(errno));
        perror("open error"); 
        return -1;
    }

    close(fd);
    return 0;
}