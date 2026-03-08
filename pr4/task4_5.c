#include <stdio.h>
#include <stdlib.h>

int main(){
    size_t size = 1024;
    char *ptr = malloc(size);
    if (!ptr){
        printf("Initial malloc failed\n");
        return 1;
    }
    printf("Initial allocation: %zu bytes\n", size);
    size = (size_t)1024 * 1024 * 1024 * 1024; 
    char *tmp = realloc(ptr, size);
    if (!tmp){
        printf("realloc failed\n");
        printf("Old pointer still valid: %p\n", ptr);
        free(ptr); 
    } else {
        ptr = tmp;
        printf("realloc succeeded\n");
        free(ptr);
    }
    return 0;
}
