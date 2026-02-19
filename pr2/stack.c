#include <stdio.h>
void f() {}
int array[100] = {1, 2, 3};
int bss;

int main()
{
    int i;
    f();
    int array2[100] = {1,2,3,4};
    printf("The stack top is near %p\n", &i);
    printf("The text stack top is near%p\n", (void*)f);
    printf("The data stack top is near %p\n", &array);
    printf("The bss stack top is near %p\n", &bss);
    return 0;
}
