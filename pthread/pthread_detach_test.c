#include<pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    pthread_t tid;
    pthread_create(&tid,NULL,task,NULL);

    pthread_detach(tid);
    return 0;
}
