#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

int main(int argc, char **argv)
{
    int x = 5;
    void *addr = &x;
    if (mprotect((void *)((uintptr_t)addr & ~0xfff), 4096, PROT_READ | PROT_WRITE) == 0){
        printf("Changed protection!\n");
    }
    else{
        perror("mprotect");
    }
}
