#include "pthread.h"
#include "unistd.h"
#include "stdio.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "string.h"
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

#define PORT 8080
#define QCAP 16

char root_real[512];

// 环形队列的长度
int task_q[QCAP];
int q_head = 0, q_tail = 0, q_count = 0;

pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

static int total_served = 0;
pthread_mutex_t stat_mutex = PTHREAD_MUTEX_INITIALIZER;
int stat_total = 0, stat_200 = 0, stat_404 = 0;

volatile sig_atomic_t running = 1;

void handle_sigint(int sig)
{
    (void)sig;
    running = 0;
}

void *handle_client(void *arg)
{
    pthread_mutex_lock(&stat_mutex);
    stat_total++;
    pthread_mutex_unlock(&stat_mutex);

    int client_fd = (int)(long)arg;
    char buf[4096] = {0};

    ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
    if (n <= 0)
    {
        close(client_fd);
        return NULL;
    }
    fprintf(stderr, "收到 %zd 字节,原始请求:\n%s\n", n, buf);

    char method[16], path[256];
    if (sscanf(buf, "%15s %255s", method, path) != 2)
    {
        close(client_fd);
        return NULL;
    }
    fprintf(stderr, "方法=%s 路径=%s\n", method, path);

    char filepath[512];
    if (strcmp(path, "/") == 0)
        snprintf(filepath, sizeof(filepath), "./wwwroot/index.html");
    else
        snprintf(filepath, sizeof(filepath), "./wwwroot%s", path);


    
    char real[512];
    // realpath 把一个可能包含 .、..、符号链接、重复斜杠的相对路径
    // 解析成唯一的、规范化的绝对路径。
    if (realpath(filepath, real) == NULL)
    {
        pthread_mutex_lock(&stat_mutex);
        stat_404++;
        pthread_mutex_unlock(&stat_mutex);
        char body404[] = "<h1>404 Not Found</h1>";
        char resp[512];
        int rlen = snprintf(resp, sizeof(resp),
                            "HTTP/1.1 404 Not Found\r\n"
                            "Content-Type: text/html\r\n"
                            "Content-Length: %zu\r\n"
                            "\r\n"
                            "%s",
                            strlen(body404), body404);
        send(client_fd, resp, rlen, 0);
        close(client_fd);
        return NULL;
    }
    /* 前缀校验:真实路径必须以根目录开头,注意 root_real 末尾补 '/' 防边界欺骗 */
    size_t rlen = strlen(root_real);
    if (strncmp(real, root_real, rlen) != 0||real[rlen] != '/')
    {

        char body403[] = "<h1>403 Not Found</h1>";
        char resp[512];
        int rlen = snprintf(resp, sizeof(resp),
                            "HTTP/1.1 403 Forbidden\r\n"
                            "Content-Type: text/html\r\n"
                            "Content-Length: %zu\r\n"
                            "\r\n"
                            "%s",
                            strlen(body403), body403);
        send(client_fd, resp, rlen, 0);
        close(client_fd);
        return NULL;
    }

    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL)
    {
        pthread_mutex_lock(&stat_mutex);
        stat_404++;
        pthread_mutex_unlock(&stat_mutex);
        char body404[] = "<h1>404 Not Found</h1>";
        char resp[512];
        int rlen = snprintf(resp, sizeof(resp),
                            "HTTP/1.1 404 Not Found\r\n"
                            "Content-Type: text/html\r\n"
                            "Content-Length: %zu\r\n"
                            "\r\n"
                            "%s",
                            strlen(body404), body404);
        send(client_fd, resp, rlen, 0);
    }
    else
    {

        pthread_mutex_lock(&stat_mutex);
        stat_200++;
        pthread_mutex_unlock(&stat_mutex);

        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        rewind(fp);

        const char *ctype = "application/octet-stream";
        if (strstr(filepath, ".html"))
            ctype = "text/html";
        else if (strstr(filepath, ".css"))
            ctype = "text/css";
        else if (strstr(filepath, ".png"))
            ctype = "image/png";

        /* 响应头单独发 */
        char header[512];
        int hlen = snprintf(header, sizeof(header),
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: %s\r\n"
                            "Content-Length: %ld\r\n"
                            "\r\n",
                            ctype, fsize);
        send(client_fd, header, hlen, 0);

        char chunk[4096];
        size_t rn;
        while ((rn = fread(chunk, 1, sizeof(chunk), fp)) > 0)
            send(client_fd, chunk, rn, 0);
        fclose(fp);
    }
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
                pthread_cond_signal(&not_full);
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
    realpath("./wwwroot", root_real);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有网卡
    addr.sin_port = htons(8080);              // 注意字节序转换

    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 128);
    fprintf(stderr, "listening on %d...\n", PORT);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    pthread_t p1, p2, p3;
    pthread_create(&p1, NULL, worker, NULL);
    pthread_create(&p2, NULL, worker, NULL);
    pthread_create(&p3, NULL, worker, NULL);

    while (1)
    {
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int client_fd = accept(listen_fd, (struct sockaddr *)&cli, &len);
        if (client_fd < 0)
        {
            if (errno == EINTR && !running) break;
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
    fprintf(stderr, "stat_total=%d stat_200=%d stat_404=%d\n", stat_total, stat_200, stat_404);

    return 0;
}