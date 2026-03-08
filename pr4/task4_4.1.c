
#include <stdio.h>
#include <stdlib.h>

int main(){
    int n = 10;
    void *ptr = NULL;
    for (int i = 0; i < 3; i++){
        if (!ptr){
            ptr = malloc(n);
        }
        printf("Iteration %d: ptr = %p\n", i, ptr);
        char *p = (char *)ptr;
        for (int j = 0; j < n; j++){
            p[j] = (char)j;
        }
        free(ptr);
        printf("Iteration %d: ptr freed\n", i);
    }
    return 0;
}
