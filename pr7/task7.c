#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int ends_with_c(const char *name) {
    size_t len = strlen(name);
    return len > 2 && strcmp(name + len - 2, ".c") == 0;
}

int main(void) {
    uid_t me = getuid();
    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (!ends_with_c(entry->d_name)) {
            continue;
        }

        struct stat st;
        if (lstat(entry->d_name, &st) != 0) {
            perror(entry->d_name);
            continue;
        }

        if (!S_ISREG(st.st_mode) || st.st_uid != me) {
            continue;
        }

        printf("File: %s\n", entry->d_name);
        printf("Grant read permission for group/others? [y/N]: ");
        fflush(stdout);

        int c = getchar();
        while (c == ' ' || c == '\t') {
            c = getchar();
        }

        int answer_yes = (c == 'y' || c == 'Y');
        while (c != '\n' && c != EOF) {
            c = getchar();
        }

        if (!answer_yes) {
            continue;
        }

        mode_t new_mode = st.st_mode | S_IRGRP | S_IROTH;
        if (chmod(entry->d_name, new_mode) != 0) {
            perror(entry->d_name);
            continue;
        }

        printf("Updated: %s\n", entry->d_name);
    }

    if (closedir(dir) != 0) {
        perror("closedir");
        return 1;
    }

    return 0;
}
