#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    //1.创建共享内存对象
    char shm_name[100] = {0};
    sprintf(shm_name, "/my_shm_%d", getpid());
    int fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    shm_open();








    //释放共享内存对象
    shm_unlink();

    return 0;
}
