#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    int subprocess;
    printf("APEX\n");

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return 1;
    }
    else if (pid == 0)
    {
        char *args[] = {"/usr/bin/ping", "-c","30`", "www.baidu.com", NULL};
        char *envs[] = {NULL};
        printf("%d what\n", getpid());
        execve(args[0], args, envs);
    }
    else
    {
        printf("%d can\n", getpid());
        waitpid(pid, &subprocess, 0);
    }
    printf("subprocess = %d\n", subprocess);

    if (WIFEXITED(subprocess))
    {
        // 正常退出（调用了 exit / return / _exit）
        printf("退出码: %d\n", WEXITSTATUS(subprocess));
    }
    else if (WIFSIGNALED(subprocess))
    {
        // 被信号杀死
        printf("被信号 %d 杀死\n", WTERMSIG(subprocess));
        if (WCOREDUMP(subprocess))
            printf("产生了 core dump\n");
    }
    else if (WIFSTOPPED(subprocess))
    {
        // 被暂停（比如收到 SIGSTOP）
        printf("被信号 %d 暂停\n", WSTOPSIG(subprocess));
    }
    return 0;
}
