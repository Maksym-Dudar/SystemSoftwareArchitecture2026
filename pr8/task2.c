#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void) {
    const char *path = "task8_2_data.bin";
    uint8_t data[] = {4, 5, 2, 2, 3, 3, 7, 9, 1, 5};

    int fd = open(path, O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd == -1) {
        die("open");
    }

    ssize_t written = write(fd, data, sizeof(data));
    if (written != (ssize_t)sizeof(data)) {
        die("write");
    }

    if (lseek(fd, 3, SEEK_SET) == (off_t)-1) {
        die("lseek");
    }

    uint8_t buffer[4] = {0};
    ssize_t read_bytes = read(fd, buffer, sizeof(buffer));
    if (read_bytes != (ssize_t)sizeof(buffer)) {
        if (read_bytes == -1) {
            die("read");
        }
        fprintf(stderr, "Expected 4 bytes, got %zd\n", read_bytes);
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Bytes in buffer after lseek(fd, 3, SEEK_SET); read(fd, buffer, 4):\n");
    for (size_t i = 0; i < sizeof(buffer); ++i) {
        printf("%u%s", buffer[i], (i + 1 == sizeof(buffer)) ? "\n" : " ");
    }

    close(fd);
    return 0;
}
