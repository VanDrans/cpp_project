#include <pthread.h>
#include <unistd.h>
#include "stdio.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *add_counter(void *arg)
{

    long *ptr = (long *)arg;
    for (long i = 0; i < 1000000; i++)
    {

        (*ptr)++;
    }
}

int main(int argc, char const *argv[])
{
    pthread_t p1, p2;
    long l1 = 0, l2 = 0;
    pthread_create(&p1, NULL, add_counter, &l1);
    pthread_create(&p2, NULL, add_counter, &l2);
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    printf("counter= %ld\n", l1 + l2);
    return 0;
}