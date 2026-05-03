#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define THREAD_COUNT 3
#define MAIN_HEAP_SMALL (256 * 1024)
#define MAIN_HEAP_BIG (24 * 1024 * 1024)
#define THREAD_HEAP_SIZE (2 * 1024 * 1024)
#define THREAD_MMAP_SIZE (4 * 1024 * 1024)
#define FILE_MMAP_SIZE (64 * 1024)

typedef struct {
    int tid;
    size_t heap_size;
    size_t mmap_size;
} ThreadArgs;

typedef struct {
    size_t total_regions;
    size_t total_bytes;
    size_t heap_regions;
    size_t stack_regions;
    size_t anon_regions;
    size_t file_regions;
    size_t executable_regions;
    size_t readwrite_private_regions;
    size_t vdso_regions;
    size_t vvar_regions;
    size_t vsyscall_regions;
} MapStats;

static void touch_pages(uint8_t *ptr, size_t size, uint8_t seed) {
    long page = sysconf(_SC_PAGESIZE);
    if (page <= 0) {
        page = 4096;
    }
    for (size_t i = 0; i < size; i += (size_t)page) {
        ptr[i] = (uint8_t)(seed + (i / (size_t)page));
    }
    if (size > 0) {
        ptr[size - 1] = seed;
    }
}

static void *thread_worker(void *arg) {
    ThreadArgs *ctx = (ThreadArgs *)arg;

    uint8_t *heap_block = (uint8_t *)malloc(ctx->heap_size);
    if (!heap_block) {
        fprintf(stderr, "[thread %d] malloc failed: %s\n", ctx->tid, strerror(errno));
        return NULL;
    }

    touch_pages(heap_block, ctx->heap_size, (uint8_t)(0x10 + ctx->tid));

    uint8_t *anon_map = (uint8_t *)mmap(
        NULL,
        ctx->mmap_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if (anon_map == MAP_FAILED) {
        fprintf(stderr, "[thread %d] mmap failed: %s\n", ctx->tid, strerror(errno));
        free(heap_block);
        return NULL;
    }

    touch_pages(anon_map, ctx->mmap_size, (uint8_t)(0x80 + ctx->tid));

    printf("[thread %d] heap=%p (%zu bytes), anon_mmap=%p (%zu bytes)\n",
           ctx->tid, (void *)heap_block, ctx->heap_size, (void *)anon_map, ctx->mmap_size);

    usleep(150000);

    if (munmap(anon_map, ctx->mmap_size) != 0) {
        fprintf(stderr, "[thread %d] munmap failed: %s\n", ctx->tid, strerror(errno));
    }
    free(heap_block);

    return NULL;
}

static void classify_map(const char *line, MapStats *st) {
    unsigned long start = 0;
    unsigned long end = 0;
    char perms[5] = {0};
    char path[320] = {0};

    int matched = sscanf(line, "%lx-%lx %4s %*s %*s %*s %319[^\n]", &start, &end, perms, path);
    if (matched < 3) {
        return;
    }

    st->total_regions++;
    st->total_bytes += (size_t)(end - start);

    if (strchr(perms, 'x')) {
        st->executable_regions++;
    }
    if (strcmp(perms, "rw-p") == 0) {
        st->readwrite_private_regions++;
    }

    int has_path = (matched >= 4 && path[0] != '\0');

    if (has_path) {
        while (*path == ' ') {
            memmove(path, path + 1, strlen(path));
        }

        if (strstr(path, "[heap]")) {
            st->heap_regions++;
        } else if (strstr(path, "[stack]")) {
            st->stack_regions++;
        } else if (strstr(path, "[vdso]")) {
            st->vdso_regions++;
        } else if (strstr(path, "[vvar]")) {
            st->vvar_regions++;
        } else if (strstr(path, "[vsyscall]")) {
            st->vsyscall_regions++;
        } else {
            st->file_regions++;
        }
    } else {
        st->anon_regions++;
    }
}

static void print_maps_and_summary(const char *interesting_file_substr) {
    FILE *fp = fopen("/proc/self/maps", "r");
    if (!fp) {
        fprintf(stderr, "Cannot open /proc/self/maps: %s\n", strerror(errno));
        return;
    }

    MapStats stats = {0};
    char line[512];

    puts("\n=== /proc/self/maps: interesting fragments ===");
    while (fgets(line, sizeof(line), fp)) {
        classify_map(line, &stats);

        if (strstr(line, "[heap]") ||
            strstr(line, "[stack]") ||
            strstr(line, "[vdso]") ||
            strstr(line, "[vvar]") ||
            strstr(line, "[vsyscall]") ||
            (interesting_file_substr && strstr(line, interesting_file_substr))) {
            fputs(line, stdout);
        }
    }
    fclose(fp);

    printf("\n=== Summary of /proc/self/maps ===\n");
    printf("Total regions:            %zu\n", stats.total_regions);
    printf("Total virtual size:       %.2f MiB\n", (double)stats.total_bytes / (1024.0 * 1024.0));
    printf("[heap] regions:           %zu\n", stats.heap_regions);
    printf("[stack] regions:          %zu\n", stats.stack_regions);
    printf("Anonymous regions:        %zu\n", stats.anon_regions);
    printf("File-backed regions:      %zu\n", stats.file_regions);
    printf("Executable regions:       %zu\n", stats.executable_regions);
    printf("rw-p regions:             %zu\n", stats.readwrite_private_regions);
    printf("[vdso]/[vvar]/[vsyscall]: %zu / %zu / %zu\n",
           stats.vdso_regions, stats.vvar_regions, stats.vsyscall_regions);
}

int main(void) {
    printf("PID: %d\n", getpid());

    uint8_t *small_heap = (uint8_t *)malloc(MAIN_HEAP_SMALL);
    uint8_t *big_heap = (uint8_t *)calloc(1, MAIN_HEAP_BIG);

    if (!small_heap || !big_heap) {
        fprintf(stderr, "Main heap allocation failed\n");
        free(small_heap);
        free(big_heap);
        return 1;
    }

    touch_pages(small_heap, MAIN_HEAP_SMALL, 0x33);
    touch_pages(big_heap, MAIN_HEAP_BIG, 0x55);

    printf("[main] small malloc=%p (%d KiB), big calloc=%p (%d MiB)\n",
           (void *)small_heap,
           MAIN_HEAP_SMALL / 1024,
           (void *)big_heap,
           MAIN_HEAP_BIG / (1024 * 1024));

    char file_template[] = "/tmp/pr6_mmap_demo_XXXXXX";
    int fd = mkstemp(file_template);
    if (fd < 0) {
        fprintf(stderr, "mkstemp failed: %s\n", strerror(errno));
        free(small_heap);
        free(big_heap);
        return 1;
    }

    if (ftruncate(fd, FILE_MMAP_SIZE) != 0) {
        fprintf(stderr, "ftruncate failed: %s\n", strerror(errno));
        close(fd);
        unlink(file_template);
        free(small_heap);
        free(big_heap);
        return 1;
    }

    uint8_t *file_map = (uint8_t *)mmap(NULL, FILE_MMAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_map == MAP_FAILED) {
        fprintf(stderr, "file mmap failed: %s\n", strerror(errno));
        close(fd);
        unlink(file_template);
        free(small_heap);
        free(big_heap);
        return 1;
    }

    const char *message = "Mapped file says hello from mmap()\n";
    memcpy(file_map, message, strlen(message));
    msync(file_map, FILE_MMAP_SIZE, MS_SYNC);

    printf("[main] file-backed mmap=%p (%d KiB), file=%s\n",
           (void *)file_map, FILE_MMAP_SIZE / 1024, file_template);

    pthread_t threads[THREAD_COUNT];
    ThreadArgs args[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; ++i) {
        args[i].tid = i;
        args[i].heap_size = THREAD_HEAP_SIZE;
        args[i].mmap_size = THREAD_MMAP_SIZE;

        int rc = pthread_create(&threads[i], NULL, thread_worker, &args[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create[%d] failed: %s\n", i, strerror(rc));
            threads[i] = 0;
        }
    }

    for (int i = 0; i < THREAD_COUNT; ++i) {
        if (threads[i] != 0) {
            pthread_join(threads[i], NULL);
        }
    }

    print_maps_and_summary("pr6_mmap_demo_");

    munmap(file_map, FILE_MMAP_SIZE);
    close(fd);
    unlink(file_template);
    free(small_heap);
    free(big_heap);

    return 0;
}
