#define _GNU_SOURCE

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
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        die("pipe");
    }

    int flags = fcntl(pipefd[1], F_GETFL);
    if (flags == -1) {
        die("fcntl(F_GETFL)");
    }
    if (fcntl(pipefd[1], F_SETFL, flags | O_NONBLOCK) == -1) {
        die("fcntl(F_SETFL)");
    }

    int pipe_size = 65536;
#ifdef F_GETPIPE_SZ
    {
        int detected = fcntl(pipefd[1], F_GETPIPE_SZ);
        if (detected != -1) {
            pipe_size = detected;
        }
    }
#endif

    char chunk[4096];
    memset(chunk, 'A', sizeof(chunk));

    for (;;) {
        ssize_t w = write(pipefd[1], chunk, sizeof(chunk));
        if (w == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            die("write(fill pipe)");
        }
    }

    size_t drained = (size_t)pipe_size / 4;
    char *drain_buf = malloc(drained);
    if (!drain_buf) {
        die("malloc(drain_buf)");
    }

    ssize_t r = read(pipefd[0], drain_buf, drained);
    if (r <= 0) {
        die("read(drain)");
    }

    size_t nbytes = (size_t)pipe_size / 2;
    char *buffer = malloc(nbytes);
    if (!buffer) {
        die("malloc(buffer)");
    }
    memset(buffer, 'B', nbytes);

    ssize_t count = write(pipefd[1], buffer, nbytes);
    if (count == -1) {
        perror("write(test)");
        free(buffer);
        free(drain_buf);
        close(pipefd[0]);
        close(pipefd[1]);
        return EXIT_FAILURE;
    }

    printf("Pipe capacity: %d bytes\n", pipe_size);
    printf("Freed before test write: %zd bytes\n", r);
    printf("Requested nbytes: %zu\n", nbytes);
    printf("write() returned count: %zd\n", count);

    if ((size_t)count != nbytes) {
        printf("Partial write confirmed: count != nbytes\n");
    } else {
        printf("In this run count == nbytes (repeat with larger nbytes if needed).\n");
    }

    free(buffer);
    free(drain_buf);
    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}
