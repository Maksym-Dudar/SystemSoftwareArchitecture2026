#include <stdio.h>
#include <ctype.h>
#include <math.h>

int main(void)
{
    char arr[100];
    char arr2[100];
    int i = 0, p = 0, system, c;
    double sum = 0;

    printf("Введіть систему числення: ");
    system = getchar() - '0';

    while (getchar() != '\n')
        ;

    printf("Введіть число: ");
    while ((c = getchar()) != '\n' && i < 99)
    {
        if (c == ',' || p !=0)
        {
            arr2[p++] = (char)c;
        }
        else
        {
            arr[i++] = (char)c;
        }
    }
    arr[i] = '\0';
    arr2[p] = '\0';
    i--;

    for (int o = 0; i >= 0; i--, o++)
    {
        if (isdigit(arr[i]))
        {
            sum = sum + ((arr[i] - '0') * pow(system, o));
        }
    }
    for (int o = 1; o < p; o++)
    {
        if (isdigit(arr2[o]))
        {
            sum = sum + ((arr2[o] - '0') * pow(system, -o));
        }
    }

    printf("%f", sum);

    return 0;
}