#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static int cmp_strings(const void *a, const void *b) {
    const char * const *sa = (const char * const *)a;
    const char * const *sb = (const char * const *)b;
    return strcmp(*sa, *sb);
}

int main(void) {
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    size_t capacity = 16;
    size_t count = 0;
    char **names = malloc(capacity * sizeof(char *));
    if (names == NULL) {
        perror("malloc");
        closedir(dir);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        struct stat st;
        if (lstat(entry->d_name, &st) != 0) {
            perror(entry->d_name);
            continue;
        }

        if (!S_ISDIR(st.st_mode)) {
            continue;
        }

        if (count == capacity) {
            capacity *= 2;
            char **tmp = realloc(names, capacity * sizeof(char *));
            if (tmp == NULL) {
                perror("realloc");
                for (size_t i = 0; i < count; ++i) {
                    free(names[i]);
                }
                free(names);
                closedir(dir);
                return 1;
            }
            names = tmp;
        }

        names[count] = strdup(entry->d_name);
        if (names[count] == NULL) {
            perror("strdup");
            for (size_t i = 0; i < count; ++i) {
                free(names[i]);
            }
            free(names);
            closedir(dir);
            return 1;
        }
        count++;
    }

    if (closedir(dir) != 0) {
        perror("closedir");
    }

    qsort(names, count, sizeof(char *), cmp_strings);
    for (size_t i = 0; i < count; ++i) {
        printf("%s\n", names[i]);
        free(names[i]);
    }
    free(names);

    return 0;
}
