#define _GNU_SOURCE
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAX_FDS   64
#define MAX_FILES 32

static int registered_fds[MAX_FDS];
static volatile sig_atomic_t registered_fd_count = 0;

static const char *registered_files[MAX_FILES];
static volatile sig_atomic_t registered_file_count = 0;

static void wr(const char *s) {
    size_t len = 0;
    while (s[len] != '\0')
        len++;

    while (len > 0) {
        ssize_t r = write(STDERR_FILENO, s, len);

        if (r <= 0)
            return;

        s += r;
        len -= (size_t)r;
    }
}

int register_fd(int fd) {
    if (registered_fd_count >= MAX_FDS)
        return -1;

    registered_fds[registered_fd_count++] = fd;
    return 0;
}

int register_temp_file(const char *path) {
    if (registered_file_count >= MAX_FILES)
        return -1;

    registered_files[registered_file_count++] = path;
    return 0;
}

static void emergency_cleanup(void) {
    for (sig_atomic_t i = 0; i < registered_fd_count; ++i) {
        if (registered_fds[i] >= 0) {
            close(registered_fds[i]);
        }
    }

    for (sig_atomic_t i = 0; i < registered_file_count; ++i) {
        if (registered_files[i] != NULL) {
            unlink(registered_files[i]);
        }
    }
}

static void crash_handler(int sig) {
    int saved_errno = errno;

    wr("\n[FATAL] crash detected\n");

    emergency_cleanup();

    wr("[FATAL] resources released\n");

    errno = saved_errno;

    _exit(128 + sig);
}

static void install_handlers(void) {
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = crash_handler;
    sigemptyset(&sa.sa_mask);

    sa.sa_flags = SA_RESETHAND;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGABRT, &sa, NULL);
    sigaction(SIGBUS,  &sa, NULL);
    sigaction(SIGILL,  &sa, NULL);
    sigaction(SIGFPE,  &sa, NULL);
}

static void make_crash(void) {
    volatile int *p = NULL;
    *p = 1;
}

int main(void) {
    install_handlers();

    int fd = open("temp.bin", O_CREAT | O_RDWR, 0644);

    if (fd >= 0) {
        register_fd(fd);
    }

    register_temp_file("temp.bin");

    wr("Program started\n");
    register_fd(fd);
    make_crash();

    return 0;
}
