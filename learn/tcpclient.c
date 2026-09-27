// 网络编程
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define handle_error(cmd, result) \
    if (result < 0)               \
    {                             \
        perror(cmd);              \
        return -1;                \
    }


void *read_from_server(void *arg){
    char * read_buf = NULL;
    int client_fd = *(int*)arg;
    read_buf = malloc(sizeof(char)*1024);
    ssize_t count = 0;
    if(!read_buf){
        perror("malloc read_buf");
        return NULL;
    }

    while (count = recv(client_fd,read_buf,1024,0))
    {
        fputs(read_buf,stdout);
    }

    printf("客户端请求关闭\n");
    free(read_buf);
    read_buf = NULL;

    return NULL;
    
}

void* write_to_server(void *arg){
    char * write_buf = NULL;
    int client_fd = *(int*)arg;
    write_buf = malloc(sizeof(char)*1024);
    ssize_t count = 0;
    if(!write_buf){
        perror("malloc write_buf");
        return NULL;
    }

    while (fgets(write_buf,1024,stdin))
    {
        send(client_fd,write_buf,strlen(write_buf),0);
    }

    printf("客户端请求关闭\n");
    free(write_buf);
    write_buf = NULL;

    return NULL;

}
int main(void)
{
    struct sockaddr_in server_addr, client_addr;

    // 清空
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));


    client_addr.sin_family = AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&client_addr.sin_addr);
    client_addr.sin_port = htons(8888);

    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr);
    server_addr.sin_port = htons(6666);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    handle_error("socket", sockfd);

    int temp_result =bind(sockfd, (struct sockaddr *)&server_addr, sizeof(client_addr));

    temp_result = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(client_addr));
    handle_error("connect", temp_result);
    


    /*
    pthread_create(&tid,  &attr,  worker,  &arg);
                     │      │       │        │
                     │      │       │        └─ 传给线程函数的参数（void*）
                     │      │       └────────── 线程函数（void* (*)(void*)）
                     │      └────────────────── 线程属性（NULL=默认）
                     └───────────────────────── 传出：新线程 ID
    */
    //创建子线程
    pthread_t pid_read,pid_write;
    pthread_create(&pid_read,NULL,read_from_server,(void*)&sockfd);
    pthread_create(&pid_write,NULL,write_to_server,(void*)&sockfd);

    pthread_join(pid_read,NULL);
    pthread_join(pid_write,NULL);

    printf("释放资源\n");
    close(sockfd);

    return 0;
}         