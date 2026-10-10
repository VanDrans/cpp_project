#include <fcntl.h>
#include <stdio.h>
#include <mqueue.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(void)
{
    struct mq_attr attr;
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = 100;
    attr.mq_flags   = 0;
    attr.mq_curmsgs = 0;

    const char *mq_name = "/p_c_mq";
    mq_unlink(mq_name);   
    mqd_t mqd = mq_open(mq_name, O_CREAT | O_RDWR, 0666, &attr);
    if (mqd == (mqd_t)-1) {
        perror("mq_open");
        return -1;
    }

    char write_buf[100];
    while (1) {
        memset(write_buf, 0, sizeof(write_buf));
        ssize_t read_count = read(STDIN_FILENO, write_buf, sizeof(write_buf) - 1);

        if (read_count == -1) {
            perror("read");
            continue;
        }
        if (read_count == 0) {          /* Ctrl+D 到达 EOF */
            printf("read EOF\n");
            mq_send(mqd, write_buf, 0, 0);   /* 发送 0 长度消息作为结束信号 */
            break;
        }


        if (mq_send(mqd, write_buf, (size_t)read_count, 0) == -1) {
            perror("mq_send");
            continue;
        }
        printf("send: %s\n", write_buf);
    }

    mq_close(mqd);
    /* 如果由 producer 负责清理，也可以在这里 mq_unlink；见下方说明 */
    return 0;
}
