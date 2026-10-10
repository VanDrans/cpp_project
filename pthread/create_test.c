#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUF_LEN 1024

char *buf;
int write_pos = 0;          // 读线程写到哪
int read_pos = 0;           // 写线程读到哪
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *read_input(void *arg)
{
    int c;
    while (1)
    {
        c = fgetc(stdin);
        if (c == EOF)
            break;

        pthread_mutex_lock(&lock);
        while (write_pos >= BUF_LEN)   // 缓冲区满，等
        {
            pthread_mutex_unlock(&lock);
            usleep(1000);
            pthread_mutex_lock(&lock);
        }
        buf[write_pos++] = (char)c;
        pthread_cond_signal(&cond);    // 通知写线程有新数据
        pthread_mutex_unlock(&lock);

        if (c == '\n')
            break;
    }
    return NULL;
}

void *write_output(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&lock);
        while (read_pos >= write_pos)  // 没数据可读，等
        {
            pthread_cond_wait(&cond, &lock);
        }
        char c = buf[read_pos++];
        pthread_mutex_unlock(&lock);

        printf("%c", c);
        fflush(stdout);

        if (c == '\n')
            break;
    }
    return NULL;
}

int main(void)
{
    buf = (char *)malloc(BUF_LEN);
    if (!buf) return 1;
    memset(buf, 0, BUF_LEN);   // 清零，避免垃圾值

    pthread_t pid_input, pid_output;
    pthread_create(&pid_input, NULL, read_input, NULL);
    pthread_create(&pid_output, NULL, write_output, NULL);

    pthread_join(pid_input, NULL);
    pthread_join(pid_output, NULL);

    free(buf);
    return 0;
}