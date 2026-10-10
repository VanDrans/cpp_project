#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

void signint_handler(int signnum){
    printf("收到%d信号,停止程序\n",signnum);
    exit(signnum);
}

int main(int argc, char const *argv[])
{
    if(signal(SIGINT,signint_handler)==SIG_ERR){
        perror("signal");
        return 1;
    }
    while(1){
        sleep(1);
        printf("hello\n");
    }
    return 0;
}
