#include <stdio.h>
#include <stdlib.h>

int main(){
    printf("Allocating memory...\n");

    while (1){
        int *arr = malloc(1024 * 1024);
        if (!arr){
            perror("Memory allocation failed");
            break;
        }
        printf("Allocated 1 MB\n");
    }

    printf("Program finished.\n");
    return 0;
}
