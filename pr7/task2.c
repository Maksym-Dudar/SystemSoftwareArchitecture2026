#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static void mode_to_string(mode_t mode, char out[11]) {
    out[0] = S_ISDIR(mode) ? 'd' :
             S_ISLNK(mode) ? 'l' :
             S_ISCHR(mode) ? 'c' :
             S_ISBLK(mode) ? 'b' :
             S_ISFIFO(mode) ? 'p' :
             S_ISSOCK(mode) ? 's' : '-';

    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    out[3] = (mode & S_IXUSR) ? 'x' : '-';
    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    out[6] = (mode & S_IXGRP) ? 'x' : '-';
    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    out[9] = (mode & S_IXOTH) ? 'x' : '-';
    out[10] = '\0';
}

int main(void) {
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

        struct stat st;
        if (lstat(entry->d_name, &st) != 0) {
            perror(entry->d_name);
            continue;
        }

        char perms[11];
        mode_to_string(st.st_mode, perms);

        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);

        char timebuf[32];
        struct tm tm_value;
        localtime_r(&st.st_mtime, &tm_value);
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", &tm_value);

        printf("%s %2lu %-8s %-8s %8lld %s %s", perms,
               (unsigned long)st.st_nlink,
               pw ? pw->pw_name : "unknown",
               gr ? gr->gr_name : "unknown",
               (long long)st.st_size,
               timebuf,
               entry->d_name);

        if (S_ISLNK(st.st_mode)) {
            char link_target[PATH_MAX];
            ssize_t n = readlink(entry->d_name, link_target, sizeof(link_target) - 1);
            if (n >= 0) {
                link_target[n] = '\0';
                printf(" -> %s", link_target);
            }
        }

        putchar('\n');
    }

    if (closedir(dir) != 0) {
        perror("closedir");
        return 1;
    }

    return 0;
}
