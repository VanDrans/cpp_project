#include "pthread.h"
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "time.h"
#include "semaphore.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t empty;
sem_t full;

#define QUEUE_CAPACITY 8
int ring[QUEUE_CAPACITY];
int head = 0;  /* 消费者取的位置 */
int tail = 0;  /* 生产者放的位置 */
int count = 0; /* 当前产品数 */

void *producer(void *arg)
{
    unsigned int seed = (unsigned int)(time(NULL) ^ pthread_self()); // 线程专属种子
    for (int i = 1; i <= 20; i++)
    {
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        ring[tail] = i;
        tail = (tail + 1) % QUEUE_CAPACITY;
        count++;
        printf("[生产者] 放入产品 %d\n", i);

        pthread_mutex_unlock(&mutex);
        sem_post(&full);
        usleep((rand_r(&seed) % 20) * 1000);
    }
    return NULL;
}

void *consumer(void *arg)
{
    unsigned int seed = (unsigned int)(time(NULL) ^ pthread_self()); // 线程专属种子
    for (int i = 1; i <= 20; i++)
    {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);

        int item = ring[head];                  // 先取真实数据
        printf("[消费者] 取走产品 %d\n", item); // 打印取到的值
        ring[head] = 0;
        head = (head + 1) % QUEUE_CAPACITY; // 再推进 head
        count--;
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        usleep((rand_r(&seed) % 100) * 1000);
    }
    return NULL;
}

int main()
{
    sem_init(&empty, 0, 8);
    sem_init(&full, 0, 0);

    pthread_t p1, p2;
    pthread_create(&p1, NULL, producer, NULL);
    pthread_create(&p2, NULL, consumer, NULL);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    sem_destroy(&empty);
    sem_destroy(&full);
}