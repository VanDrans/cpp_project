#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void * working(void * arg){
    printf("我是子线程,线程id:%ld\n",pthread_self());
    for(int i = 0;i<9;i++){
        printf("child == i :=%d\n",i);
    }
    return NULL;
}

int main(){
    pthread_t tid;
    pthread_create(&tid,NULL,working,NULL);

    printf("子进程创建成功,线程id:%ld\n",tid);
    printf("我是主线程,线程id:%ld\n",pthread_self());

    for(int i = 0;i<3;++i){
        printf("i = %d\n",i);
    }
    sleep(1);
    return 0;
}