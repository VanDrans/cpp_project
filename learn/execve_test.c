#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include<sys/types.h>

int main()
{   
    char *name = "张玉豪";
    printf("name = %s pid = %d\n", name, getpid());
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        


    //执行跳转 跳转前后替换的是整个进程映像：包括代码、数据、堆、栈，
    //但 PID、父进程、文件描述符（除非设置了 FD_CLOEXEC）、信号处理等会保留
   /*  
    int execve (const char *__path, char *const __argv[],
		   char *const __envp[]) __THROW __nonnull ((1, 2)); 
    const char *__path      执行程序的路径
    char *const __argv[]    传入的参数
            (1) 第一个参数是程序的名字
            (2) 第二个参数是传入的参数
            (3) 最后一个参数一定是NULL  
    char *const __envp[]    传递的环境变量

    return:
        成功后不会返回,下面的代码也没有意义
        失败返回-1
    */
    char *argv[] = {"./erlou", "参数够了,可以上二楼", NULL};
    int res = execve("./erlou", argv, NULL);
    if (res == -1) {
        perror("execve");
        return 1;
    }
    return 0;
}
