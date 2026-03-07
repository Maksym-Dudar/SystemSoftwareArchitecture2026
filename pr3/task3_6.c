#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void handle_sigsegv(int sig){
    printf("\nStack limit exceeded (SIGSEGV). Program stopped.\n");
    exit(1);
}

void recurse(int depth){
    char buffer[1024];
    for (int i = 0; i < 1024; i++)
        buffer[i] = i % 256;
    printf("Recursion depth: %d\n", depth);
    recurse(depth + 1);
}

int main(){
    signal(SIGSEGV, handle_sigsegv);
    printf("Starting recursion...\n");
    recurse(1);
    return 0;
}
