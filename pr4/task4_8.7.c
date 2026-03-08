#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE (64 * 1024 * 1024)

double test_stride(size_t stride)
{
    char *arr = malloc(ARRAY_SIZE);
    if (!arr)
    {
        perror("malloc");
        exit(1);
    }

    clock_t start = clock();

    for (size_t k = 0; k < ARRAY_SIZE; k++)
    {
        size_t i = (k * stride) % ARRAY_SIZE;
        arr[i]++;
    }

    clock_t end = clock();

    free(arr);

    return (double)(end - start) / CLOCKS_PER_SEC;
}

int main()
{
    size_t strides[] = {1, 4, 16, 64, 4096};

    for (int i = 0; i < 5; i++)
    {
        double t = test_stride(strides[i]);
        printf("Stride %zu bytes: %f seconds\n", strides[i], t);
    }

    return 0;
}
