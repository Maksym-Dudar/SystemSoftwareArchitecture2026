#include <stdio.h>
#include <stdlib.h>

static int prompt_next_page(void) {
    printf("--More-- [Enter=next, q=quit] ");
    fflush(stdout);

    int c = getchar();
    if (c == EOF) {
        return 0;
    }

    while (c != '\n' && c != EOF) {
        if (c == 'q' || c == 'Q') {
            while (c != '\n' && c != EOF) {
                c = getchar();
            }
            putchar('\n');
            return 0;
        }
        c = getchar();
    }

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file1> [file2 ...]\n", argv[0]);
        return 1;
    }

    int line_count = 0;

    for (int i = 1; i < argc; ++i) {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            perror(argv[i]);
            continue;
        }

        if (argc > 2) {
            printf("\n===== %s =====\n", argv[i]);
        }

        char line[4096];
        while (fgets(line, sizeof(line), fp) != NULL) {
            fputs(line, stdout);
            line_count++;

            if (line_count >= 20) {
                if (!prompt_next_page()) {
                    fclose(fp);
                    return 0;
                }
                line_count = 0;
            }
        }

        if (ferror(fp)) {
            perror("fgets");
            fclose(fp);
            return 1;
        }

        fclose(fp);
    }

    return 0;
}
