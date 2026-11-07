#include <pthread.h>
#include <unistd.h>
#include "stdio.h"

long counter = 0;

void *add_counter(void* arg)
{
    for (long i = 0; i < 1000000; i++)
    {

        counter++;
        
    }
}

int main(int argc, char const *argv[])
{
    pthread_t p1, p2;
    pthread_create(&p1, NULL, add_counter, NULL);
    pthread_create(&p2, NULL, add_counter, NULL);
    pthread_join(p1,NULL);
    pthread_join(p2,NULL);
    printf("counter= %ld\n",counter);
    return 0;
}
