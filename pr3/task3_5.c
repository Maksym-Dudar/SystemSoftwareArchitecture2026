#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define BUFFER_SIZE 1024

void handle_sigxfsz(int sig)
{
    printf("\nFile size limit exceeded. Copying stopped.\n");
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Program need two arguments\n");
        return 1;
    }

    char *source_file = argv[1];
    char *dest_file = argv[2];

    signal(SIGXFSZ, handle_sigxfsz);

    FILE *src = fopen(source_file, "rb");
    if (!src)
    {
        printf("Cannot open file %s for reading\n", source_file);
        return 1;
    }

    FILE *dst = fopen(dest_file, "wb");
    if (!dst)
    {
        printf("Cannot open file %s for writing\n", dest_file);
        fclose(src);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src)) > 0)
    {
        if (fwrite(buffer, 1, bytes_read, dst) != bytes_read)
        {
            printf("Error writing to file %s\n", dest_file);
            fclose(src);
            fclose(dst);
            return 1;
        }
    }

    fclose(src);
    fclose(dst);

    printf("File copied successfully.\n");
    return 0;
}
