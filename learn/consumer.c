#include <fcntl.h>
#include <stdio.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(void)
{
    const char *mq_name = "/p_c_mq";
    mqd_t mqd = mq_open(mq_name, O_RDWR);
    if (mqd == (mqd_t)-1) {
        perror("mq_open");
        return -1;
    }

    char read_buf[100];
    while (1) {
        ssize_t recv_count = mq_receive(mqd, read_buf, sizeof(read_buf), NULL);
        if (recv_count == -1) {
            perror("mq_receive");
            continue;
        }
        if (recv_count == 0) {          /* 0 长度消息 = EOF 信号 */
            printf("recv EOF\n");
            break;
        }
        read_buf[recv_count] = '\0';
        printf("recv: %s\n", read_buf);
    }

    mq_close(mqd);
    mq_unlink(mq_name);                 /* 正确的 unlink */
    return 0;
}

