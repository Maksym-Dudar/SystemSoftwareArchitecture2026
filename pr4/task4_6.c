#include <stdio.h>
#include <stdlib.h>

int main() {

    printf("Case 1: realloc(NULL, size)\n");
    int *p = realloc(NULL, 10 * sizeof(int));

    if (p == NULL) {
        printf("Allocation failed\n");
        return 1;
    }

    for (int i = 0; i < 10; i++)
        p[i] = i;

    printf("Memory allocated and used\n");


    printf("\nCase 2: realloc(ptr, 0)\n");
    int *q = realloc(p, 0);

    if (q == NULL)
        printf("Memory freed, realloc returned NULL\n");
    else
        printf("realloc returned non-NULL pointer\n");

    return 0;
}
