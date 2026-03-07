#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

void handle_sigxcpu(int sig){
    printf("\nCPU time limit exceeded. Program stopped.\n");
    exit(1);
}

int main(){
    signal(SIGXCPU, handle_sigxcpu);
    printf("Starting CPU-intensive loop...\n");
    volatile unsigned long i = 0;
    while (1){
        i++; 
        if (i % 100000000 == 0){
            printf("i = %lu\n", i);
        }
        if (i % 1000000000 == 0)
        {
            return 0;
        }
    }
    return 0;
}
