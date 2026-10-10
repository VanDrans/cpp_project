#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char const *argv[])
{   
    char *pipe_path = "/tmp/my_fifo";
    if(mkfifo(pipe_path, 0666) == -1)
    {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
    open(pipe_path, O_RDWR);
    
    return 0;
}
