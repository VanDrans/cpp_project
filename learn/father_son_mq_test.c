#include <fcntl.h>
#include <stdio.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 100;
    attr.mq_flags = 0;
    attr.mq_curmsgs = 0;

    char *mq_name = "/my_mq";
    mqd_t mqd = mq_open(mq_name, O_CREAT | O_RDWR, 0666, &attr);

    if(mqd == (mqd_t)-1)
    {
        perror("mq_open");
        return -1;
    }
    pid_t pid = fork();
    if(pid < 0){
        perror("fork");
        return -1;
    }
    else if(pid == 0)
    {
        char recv_buf[100] = {0};
        struct timespec ts;
        for(size_t i = 0; i < 10; i++)
        {
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 5;

            ssize_t ret = mq_timedreceive(mqd, recv_buf, sizeof(recv_buf), NULL, &ts);
            if(ret == -1)
            {
                perror("mq_timedreceive");
                return -1;
            }
            printf("recv: %s\n", recv_buf);
        }
    }
    else
    {
        char send_buf[100] = {0};
        struct timespec ts;
        for(size_t i = 0; i < 10; i++)
        {
            sprintf(send_buf, "hello %zu", i);
            printf("send: %s\n", send_buf);
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec += 5;

            mq_timedsend(mqd,send_buf,strlen(send_buf),0,&ts);
        }
    }
    mq_close(mqd);
    unlink(mq_name);
    return 0;
}
