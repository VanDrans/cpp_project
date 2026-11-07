#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

void *func1(void * arg){
    static int tmp = 0;
    tmp = *(int*) arg;
    tmp += 100;
}