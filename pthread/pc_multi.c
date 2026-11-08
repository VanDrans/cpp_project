#include "pthread.h"
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "time.h"
#include "stdint.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

#define QUEUE_CAPACITY 8
int ring[QUEUE_CAPACITY];
int head = 0;  /* 消费者取的位置 */
int tail = 0;  /* 生产者放的位置 */
int count = 0; /* 当前产品数 */
int product = 1;

void *producer(void *arg)
{
    unsigned int seed = (unsigned int)(time(NULL) ^ pthread_self()); // 线程专属种子
    for (int i = 1; i <= 10; i++)
    {
        pthread_mutex_lock(&mutex);
        while (count == QUEUE_CAPACITY)
            pthread_cond_wait(&not_full, &mutex);

        int id = product++;
        ring[tail] = id;
        tail = (tail + 1) % QUEUE_CAPACITY;
        count++;
        printf("[生产者%d] 放入产品 %d\n", (int)(intptr_t)arg, id);

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
        usleep((rand_r(&seed) % 20) * 1000);
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

        int item = ring[head];                                 // 先取真实数据
        printf("[消费者%d] 取走产品 %d\n", (int)(intptr_t)arg, item); // 打印取到的值
        ring[head] = 0;
        head = (head + 1) % QUEUE_CAPACITY; // 再推进 head
        count--;
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        usleep( (50 + rand_r(&seed) % 50) * 1000);
    }
    return NULL;
}

int main()
{

    pthread_t p1, p2, p3, p4;
    pthread_create(&p1, NULL, producer, (void*)0);
    pthread_create(&p3, NULL, producer, (void*)1);
    pthread_create(&p2, NULL, consumer, (void*)0);
    pthread_create(&p4, NULL, consumer, (void*)1);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
    pthread_join(p4, NULL);

    printf("count==%d product==%d\n",count,product);
    for(int i = 0 ;i<8;i++){
        printf("ring[%d] = %d\n",i,ring[i]);
    }

    return 0;
}