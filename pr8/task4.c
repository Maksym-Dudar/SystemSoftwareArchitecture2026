#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    pid_t pid;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }

    printf("fork() return value in process %ld: %d\n", (long)getpid(), (int)pid);

    if (pid == 0) {
        printf("child: fork() returned 0\n");
        fflush(stdout);
        _exit(0);
    }

    printf("parent: fork() returned child PID = %d\n", (int)pid);
    waitpid(pid, NULL, 0);
    return 0;
}
