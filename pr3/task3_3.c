#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

void handle_sigxfsz(int sig) {
    printf("\nFile size limit exceeded. Stopping program.\n");
    exit(1);
}

int main() {
    signal(SIGXFSZ, handle_sigxfsz);

    FILE *file = fopen("dice_results.txt", "w");
    if (!file) {
        perror("Cannot open file");
        return 1;
    }

    srand(time(NULL));

    while (1) {
        int roll = rand() % 6 + 1;
        fprintf(file, "%d\n", roll);
        fflush(file);
    }

    fclose(file);
    return 0;
}
