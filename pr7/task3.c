#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <word> <file>\n", argv[0]);
        return 1;
    }

    const char *word = argv[1];
    const char *filename = argv[2];

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror(filename);
        return 1;
    }

    char line[4096];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, word) != NULL) {
            fputs(line, stdout);
            size_t len = strlen(line);
            if (len == 0 || line[len - 1] != '\n') {
                putchar('\n');
            }
        }
    }

    if (ferror(fp)) {
        perror("fgets");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    return 0;
}
