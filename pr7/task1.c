#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    FILE *from = popen("rwho", "r");
    if (from == NULL) {
        perror("popen rwho");
        return 1;
    }

    FILE *to = popen("more", "w");
    if (to == NULL) {
        perror("popen more");
        pclose(from);
        return 1;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), from) != NULL) {
        if (fputs(buffer, to) == EOF) {
            perror("write to more");
            pclose(from);
            pclose(to);
            return 1;
        }
    }

    if (ferror(from)) {
        perror("read from rwho");
        pclose(from);
        pclose(to);
        return 1;
    }

    int status_from = pclose(from);
    int status_to = pclose(to);
    if (status_from == -1 || status_to == -1) {
        perror("pclose");
        return 1;
    }

    return 0;
}
