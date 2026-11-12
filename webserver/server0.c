#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080

int main(void)
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有网卡
    addr.sin_port = htons(8080);              // 注意字节序转换

    bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, 128);
    printf("listening on %d...\n", PORT);

    while (1)
    {
        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);
        int client_fd = accept(listen_fd, (struct sockaddr *)&cli, &len);
        printf("新连接来自 %s:%d,fd = %d\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port), client_fd);

        char buf[4096] = {0};
        ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        printf("收到 %zd 字节,原始请求:\n%s\n", n, buf);

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
    }
}
