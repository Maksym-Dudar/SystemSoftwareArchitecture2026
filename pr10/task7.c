#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int n = 5;
    pid_t pid;

    printf("Кореневий процес (PID: %d) починає роботу\n", getpid());

    for (int i = 1; i <= n; i++) {
        pid = fork();

        if (pid < 0) {
            fprintf(stderr, "Помилка fork()");
            exit(1);
        } 
        else if (pid == 0) {
            printf("Процес №%d створено (PID: %d, Батько: %d)\n", i, getpid(), getppid());
            
            if (i == n) {
                printf("--- Останній процес (%d) завершує генерацію ---\n", getpid());
                exit(0); 
            }
        } 
        else {
            wait(NULL);
            printf("Процес %d завершився. Батько %d продовжує...\n", pid, getpid());
            exit(0);
        }
    }

    return 0;
}
