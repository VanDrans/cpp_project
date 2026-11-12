#include "pthread.h"
#include "unistd.h"
#include "stdio.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "string.h"
#include <signal.h>
#include <errno.h>

#define PORT 8080
#define QCAP 16
int task_q[QCAP];
int q_head = 0, q_tail = 0, q_count = 0;
int total_served = 0;

pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

volatile sig_atomic_t running = 1;

void handle_sigint(int sig)
{
    (void)sig;
    running = 0;
}

void *handle_client(void *arg)
{
    int client_fd = (int)(long)arg;
    char buf[4096] = {0};
    ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
    fprintf(stderr, "收到 %zd 字节,原始请求:\n%s\n", n, buf);
    if (n <= 0)
    {
        close(client_fd);
        return NULL;
    }

    char body[] = "<h1>Hello Web</h1>";
    char response[4096];
    int resp_len = snprintf(response, sizeof(response),
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html\r\n"
                            "Content-Length: %zu\r\n"
                            "\r\n"
                            "%s",
                            strlen(body), body);
    send(client_fd, response, resp_len, 0);

    close(client_fd);
    return NULL;
}

void *worker(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&q_mutex);
        while (q_count == 0)
        {
            if (!running)
            {
                pthread_mutex_unlock(&q_mutex);
                pthread_cond_broadcast(&not_full);
                fprintf(stderr, "work %ld 退出\n", pthread_self());
                return NULL;
            }
            pthread_cond_wait(&not_empty, &q_mutex);
        }
        int fd = task_q[q_head];
        q_head = (q_head + 1) % QCAP;
        q_count--;
        total_served++;
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&q_mutex);

        handle_client((void *)(long)fd);
    }
}

int main()
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有网卡
    addr.sin_port = htons(8080);              // 注意字节序转换

    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 128);
    fprintf(stderr, "listening on %d...\n", PORT);

    pthread_t p1, p2, p3;
    pthread_create(&p1, NULL, worker, NULL);
    pthread_create(&p2, NULL, worker, NULL);
    pthread_create(&p3, NULL, worker, NULL);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    while (1)
    {
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int client_fd = accept(listen_fd, (struct sockaddr *)&cli, &len);
        if (client_fd < 0)
        {
            if (errno == EINTR && !running)
                break;
            continue;
        }
        fprintf(stderr, "新连接来自 %s:%d,fd = %d\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port), client_fd);

        pthread_mutex_lock(&q_mutex);
        while (q_count == QCAP && running)
            pthread_cond_wait(&not_full, &q_mutex);
        if (!running)
        {
            pthread_mutex_unlock(&q_mutex);
            break;
        }
        task_q[q_tail] = client_fd;
        q_tail = (q_tail + 1) % QCAP;
        q_count++;
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&q_mutex);
    }

    pthread_mutex_lock(&q_mutex);
    pthread_cond_broadcast(&not_empty);
    pthread_mutex_unlock(&q_mutex);
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
    close(listen_fd);
    fprintf(stderr, "total_served = %d\n", total_served);

    return 0;
}