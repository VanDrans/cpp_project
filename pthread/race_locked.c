#include <pthread.h>
#include <unistd.h>
#include "stdio.h"


long counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* add_counter(void* arg)
{   
    static long counter = 0;
    for (long i = 0; i < 1000000; i++)
    {

        counter++;

    }
    pthread_exit(&counter);
}

int main(int argc, char const *argv[])
{
    pthread_t p1, p2;
    long l1,l2;
    pthread_create(&p1, NULL, add_counter, NULL);
    pthread_create(&p2, NULL, add_counter, NULL);
    void* val1 = NULL;
    void* val2 = NULL;
    pthread_join(p1,&val1);
    pthread_join(p2,&val2);
    printf("counter= %ld\n",*(long*)val1+*(long*)val2);
    return 0;
}
