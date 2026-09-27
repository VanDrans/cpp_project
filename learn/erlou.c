#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
int main(int argc, char const *argv[])
{
    if(argc < 2){
        printf("参数不够,不能上二楼\n");
        return 1;
    }
    printf("%s %d\n",argv[1],getpid());
    return 0;
}
