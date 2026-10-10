#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

typedef struct
{
    char *p;
    int len;
} Result;
void *func1(void *arg)
{
}
void *func2(void *arg)
{
}
int main(int argc, char const *argv[])
{
    pthread_t p1, p2;
    pthread_create(&p1, NULL, func1, NULL);

    pthread_create(&p2, NULL, func2, NULL);

    pthread_join(p1, NULL);

    pthread_join(p2, NULL);

    return 0;
}
