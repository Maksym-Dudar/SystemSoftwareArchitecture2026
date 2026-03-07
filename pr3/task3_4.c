#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

void handle_sigxcpu(int sig)
{
    printf("\nCPU time limit exceeded. Program stopped.\n");
    exit(1);
}

void generate_unique(int arr[], int count, int max)
{
    int used[50] = {0};

    for (int i = 0; i < count; i++)
    {
        int num;
        do
        {
            num = rand() % max + 1;
        } while (used[num]);

        used[num] = 1;
        arr[i] = num;
    }
}

int main()
{

    signal(SIGXCPU, handle_sigxcpu);

    srand(time(NULL));

    int lotto49[7];
    int lotto36[6];

    while (1)
    {

        generate_unique(lotto49, 7, 49);
        generate_unique(lotto36, 6, 36);

        printf("7 from 49: ");
        for (int i = 0; i < 7; i++)
            printf("%d ", lotto49[i]);

        printf(" | 6 from 36: ");
        for (int i = 0; i < 6; i++)
            printf("%d ", lotto36[i]);

        printf("\n");
    }

    return 0;
}
