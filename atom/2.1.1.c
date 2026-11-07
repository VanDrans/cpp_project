#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
    char buf[1024];
    int fd1, fd2;
    int ret;

    fd1 = open("./src_file", O_RDONLY);
    if (fd1 == -1)
        return fd1;

    fd2 = open("./dest_file", O_WRONLY);
    if (fd2 == -1)
    {
        ret = fd2;
        goto out1;
    }

    ret = read(fd1,buf,sizeof(buf));
    if(-1 == ret)
        goto out2;

    ret = write(fd2,buf,sizeof(buf));
    if(ret == -1) goto out2;

    ret = 0;

    out2:
        close(fd2);

    out1:
        close(fd1);
        return ret;
}