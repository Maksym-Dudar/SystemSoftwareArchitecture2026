#include <stdio.h>
#include <time.h>

int main(){
    time_t t;
    int bits = sizeof(t) * 8;
    long long max_seconds = (1LL << (bits - 1)) - 1;
    printf("bits: %d\n", bits);
    printf("max seconds: %lld\n", max_seconds);

    return 0;
}
