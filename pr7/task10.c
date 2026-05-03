#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static double random_0_1(void) {
    return (double)rand() / ((double)RAND_MAX + 1.0);
}

static unsigned int make_seed(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (unsigned int)(ts.tv_nsec ^ ts.tv_sec ^ (long)getpid());
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <n> [count]\n", argv[0]);
        return 1;
    }

    double n = atof(argv[1]);
    int count = 10;

    if (argc == 3) {
        count = atoi(argv[2]);
        if (count <= 0) {
            fprintf(stderr, "count must be positive\n");
            return 1;
        }
    }

    unsigned int seed = make_seed();
    srand(seed);

    printf("Seed: %u\n", seed);

    printf("Range [0.0, 1.0):\n");
    for (int i = 0; i < count; ++i) {
        printf("%.8f\n", random_0_1());
    }

    printf("\nRange [0.0, %.6f):\n", n);
    for (int i = 0; i < count; ++i) {
        printf("%.8f\n", random_0_1() * n);
    }

    return 0;
}
