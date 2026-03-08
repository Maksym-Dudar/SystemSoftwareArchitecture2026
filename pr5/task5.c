#include <stdio.h>

int main(){
    int secret = 12345;
    char buf[8];
    printf("Before overflow: secret = %d\n", secret);
    for (int i = 0; i < 12; i++){
        buf[i] = 'A';
    }
    printf("After overflow: secret = %d\n", secret);
    return 0;
}
