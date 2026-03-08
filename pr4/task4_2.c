#include <stdio.h>
#include <stdlib.h>

int main(){
    long int xa = 999999999999999999;
    long int xb = 999999999999999999;
    int num = xa * xb;
    printf("num = %d\n", num);
    void *p = malloc(num);
    if (p == NULL){
        printf("malloc failed\n");
    } else {
        printf("malloc succeeded\n");
        free(p);
    }
    return 0;
}
