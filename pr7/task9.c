#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static long long elapsed_ms(const struct timespec *start, const struct timespec *end) {
    long long sec = (long long)(end->tv_sec - start->tv_sec);
    long long nsec = (long long)(end->tv_nsec - start->tv_nsec);
    return sec * 1000LL + nsec / 1000000LL;
}

int main(int argc, char *argv[]) {
    long long iterations = 100000000;
    if (argc > 1) {
        iterations = atoll(argv[1]);
        if (iterations <= 0) {
            fprintf(stderr, "Iterations must be positive\n");
            return 1;
        }
    }

    struct timespec start, end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        perror("clock_gettime start");
        return 1;
    }

    volatile unsigned long long sink = 0;
    for (long long i = 0; i < iterations; ++i) {
        sink += (unsigned long long)i;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
        perror("clock_gettime end");
        return 1;
    }

    printf("Result guard value: %llu\n", sink);
    printf("Elapsed time: %lld ms\n", elapsed_ms(&start, &end));

    return 0;
}
