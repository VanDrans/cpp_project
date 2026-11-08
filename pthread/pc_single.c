#include "pthread.h"
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "time.h"

int buffer;
int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

void *producer(void *arg)
{
    unsigned int seed = (unsigned int)(time(NULL) ^ pthread_self()); // 线程专属种子
    for (int i = 1; i <= 10; i++)
    {
        pthread_mutex_lock(&mutex);
        while (count == 1)
            pthread_cond_wait(&not_full, &mutex);

        buffer = i;
        count = 1;
        printf("[生产者] 放入产品 %d\n", buffer);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
        usleep((rand_r(&seed) % 100) * 1000);
    }
    return NULL;
}

void *consumer(void *arg)
{
    unsigned int seed = (unsigned int)(time(NULL) ^ pthread_self()); // 线程专属种子
    for (int i = 1; i <= 10; i++)
    {

        pthread_mutex_lock(&mutex);
        while (count == 0)
            pthread_cond_wait(&not_empty, &mutex);

        count = 0;
        printf("[消费者] 取走产品 %d\n", buffer);
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
        
        usleep((rand_r(&seed) % 100) * 1000);
    }
    return NULL;
}

int main()
{


    pthread_t p1, p2;
    pthread_create(&p1, NULL, producer, NULL);
    pthread_create(&p2, NULL, consumer, NULL);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
}