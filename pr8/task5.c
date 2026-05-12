#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void) {
    const char *dup_file = "dup_output.txt";
    const char *dup2_file = "dup2_output.txt";

    int fd_dup = open(dup_file, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd_dup == -1) {
        die("open dup_output.txt");
    }

    int fd_dup2 = open(dup2_file, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd_dup2 == -1) {
        die("open dup2_output.txt");
    }

    int stdout_backup = dup(STDOUT_FILENO);
    if (stdout_backup == -1) {
        die("dup stdout backup");
    }

    fprintf(stderr, "[dup demo] close(STDOUT_FILENO), then dup(fd_dup)\n");
    fflush(stdout);

    if (close(STDOUT_FILENO) == -1) {
        die("close stdout");
    }

    int dup_result = dup(fd_dup);
    if (dup_result == -1) {
        die("dup(fd_dup)");
    }

    printf("This line is redirected by dup() to %s\n", dup_file);
    fflush(stdout);
    fprintf(stderr, "dup(fd_dup) returned descriptor: %d\n", dup_result);

    if (dup2(stdout_backup, STDOUT_FILENO) == -1) {
        die("restore stdout after dup");
    }

    fprintf(stderr, "[dup2 demo] dup2(fd_dup2, STDOUT_FILENO) without closing stdout\n");
    fflush(stdout);

    int dup2_result = dup2(fd_dup2, STDOUT_FILENO);
    if (dup2_result == -1) {
        die("dup2(fd_dup2, STDOUT_FILENO)");
    }

    printf("This line is redirected by dup2() to %s\n", dup2_file);
    fflush(stdout);
    fprintf(stderr, "dup2(fd_dup2, STDOUT_FILENO) returned descriptor: %d\n", dup2_result);

    if (dup2(stdout_backup, STDOUT_FILENO) == -1) {
        die("restore stdout after dup2");
    }

    close(stdout_backup);
    close(fd_dup);
    close(fd_dup2);

    printf("Done. Check files: %s and %s\n", dup_file, dup2_file);
    return 0;
}
