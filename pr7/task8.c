#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    int delete_all = 0;
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

        if (S_ISDIR(st.st_mode)) {
            continue;
        }

        int should_delete = delete_all;

        if (!delete_all) {
            printf("Delete '%s'? [y]es/[n]o/[a]ll/[q]uit: ", entry->d_name);
            fflush(stdout);

            int c = getchar();
            while (c == ' ' || c == '\t') {
                c = getchar();
            }

            if (c == 'q' || c == 'Q') {
                while (c != '\n' && c != EOF) {
                    c = getchar();
                }
                break;
            } else if (c == 'a' || c == 'A') {
                delete_all = 1;
                should_delete = 1;
            } else if (c == 'y' || c == 'Y') {
                should_delete = 1;
            }

            while (c != '\n' && c != EOF) {
                c = getchar();
            }
        }

        if (should_delete) {
            if (unlink(entry->d_name) != 0) {
                perror(entry->d_name);
            } else {
                printf("Deleted: %s\n", entry->d_name);
            }
        }
    }

    if (closedir(dir) != 0) {
        perror("closedir");
        return 1;
    }

    return 0;
}
