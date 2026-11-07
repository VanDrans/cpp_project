/* fork_demo.c — 亲手看一个退出码的"死亡之旅" */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    pid_t pid = fork(); /* TODO-1: 生个孩子 */

    if (pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        /* ---- 子进程区 ---- */
        printf("[子 %d] 我出生了\n", /* TODO-2: 自己的pid */ getpid());

        /* TODO-3: 玩点花样，故意留一条没换行的printf，比如：
         *     printf("[子] 这行没有\\n");
         * 退出用 exit(42)，观察这行输出什么时候吐出来
         */
        printf("[子] 这行没有\\n");
        exit(42); /* 遗言：退出码 42 */
    }

    /* ---- 父进程区 ---- */
    printf("[父 %d] 孩子是 %d，我等他死\n", getpid(), pid);

    /* TODO-4: 等+收+解码，三连：
     *   int st;
     *   pid_t dead = waitpid( ??? , &st, 0);        // 阻塞等那个孩子
     *   然后分情况打印：
     *   - 正常死 → WIFEXITED / WEXITSTATUS 打出 42
     *   - 被信号杀 → WIFSIGNALED / WTERMSIG 打出几号
     *   waitpid 返回值也要检查（-1=出错）
     */
    int st;
    pid_t dead = waitpid(pid, &st, 0);
    if (dead == -1)
    {
        perror("waitpid");
    }
    else if (WIFEXITED(st))
    {
        printf("st = %d，退出码 = %d\n", st, WEXITSTATUS(st));
    }
    else if (WIFSIGNALED(st))
    {
        printf("被信号 %d 杀死\n", WTERMSIG(st));
    }
    printf("[父] 收完尸，我下班\n");
    return 0;
}
