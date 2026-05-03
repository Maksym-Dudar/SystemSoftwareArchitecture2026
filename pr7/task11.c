#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VFS_MAGIC "SVFS"
#define VFS_MAX_FILES 128
#define VFS_NAME_LEN 56

typedef struct {
    char magic[4];
    uint32_t max_files;
    uint32_t data_offset;
} VfsHeader;

typedef struct {
    char name[VFS_NAME_LEN];
    uint32_t offset;
    uint32_t size;
    uint8_t used;
    uint8_t reserved[3];
} VfsEntry;

static int write_initial_layout(FILE *fp) {
    VfsHeader header;
    memcpy(header.magic, VFS_MAGIC, sizeof(header.magic));
    header.max_files = VFS_MAX_FILES;
    header.data_offset = (uint32_t)(sizeof(VfsHeader) + VFS_MAX_FILES * sizeof(VfsEntry));

    if (fseek(fp, 0, SEEK_SET) != 0) {
        return -1;
    }

    if (fwrite(&header, sizeof(header), 1, fp) != 1) {
        return -1;
    }

    VfsEntry empty;
    memset(&empty, 0, sizeof(empty));
    for (uint32_t i = 0; i < VFS_MAX_FILES; ++i) {
        if (fwrite(&empty, sizeof(empty), 1, fp) != 1) {
            return -1;
        }
    }

    return fflush(fp);
}

static int load_header(FILE *fp, VfsHeader *header) {
    if (fseek(fp, 0, SEEK_SET) != 0) {
        return -1;
    }

    if (fread(header, sizeof(*header), 1, fp) != 1) {
        return -1;
    }

    if (memcmp(header->magic, VFS_MAGIC, sizeof(header->magic)) != 0) {
        errno = EINVAL;
        return -1;
    }

    if (header->max_files != VFS_MAX_FILES) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

static int load_entries(FILE *fp, VfsEntry entries[VFS_MAX_FILES]) {
    if (fseek(fp, (long)sizeof(VfsHeader), SEEK_SET) != 0) {
        return -1;
    }

    if (fread(entries, sizeof(VfsEntry), VFS_MAX_FILES, fp) != VFS_MAX_FILES) {
        return -1;
    }

    return 0;
}

static int save_entries(FILE *fp, const VfsEntry entries[VFS_MAX_FILES]) {
    if (fseek(fp, (long)sizeof(VfsHeader), SEEK_SET) != 0) {
        return -1;
    }

    if (fwrite(entries, sizeof(VfsEntry), VFS_MAX_FILES, fp) != VFS_MAX_FILES) {
        return -1;
    }

    return fflush(fp);
}

static int command_init(const char *image_path) {
    FILE *fp = fopen(image_path, "wb+");
    if (fp == NULL) {
        perror(image_path);
        return 1;
    }

    if (write_initial_layout(fp) != 0) {
        perror("init layout");
        fclose(fp);
        return 1;
    }

    fclose(fp);
    printf("Initialized VFS image: %s\n", image_path);
    return 0;
}

static int command_list(FILE *fp) {
    VfsEntry entries[VFS_MAX_FILES];
    if (load_entries(fp, entries) != 0) {
        perror("load entries");
        return 1;
    }

    printf("Stored files:\n");
    int found = 0;
    for (uint32_t i = 0; i < VFS_MAX_FILES; ++i) {
        if (!entries[i].used) {
            continue;
        }
        printf("- %s (size=%u bytes, offset=%u)\n", entries[i].name, entries[i].size, entries[i].offset);
        found = 1;
    }

    if (!found) {
        printf("(empty)\n");
    }

    return 0;
}

static uint32_t find_end_offset(const VfsHeader *header, const VfsEntry entries[VFS_MAX_FILES]) {
    uint32_t end = header->data_offset;
    for (uint32_t i = 0; i < VFS_MAX_FILES; ++i) {
        if (!entries[i].used) {
            continue;
        }
        uint32_t candidate = entries[i].offset + entries[i].size;
        if (candidate > end) {
            end = candidate;
        }
    }
    return end;
}

static int read_whole_file(const char *path, unsigned char **buffer, size_t *size) {
    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        perror(path);
        return -1;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }

    long len = ftell(fp);
    if (len < 0) {
        fclose(fp);
        return -1;
    }

    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return -1;
    }

    unsigned char *data = NULL;
    if (len > 0) {
        data = malloc((size_t)len);
        if (data == NULL) {
            fclose(fp);
            return -1;
        }

        if (fread(data, 1, (size_t)len, fp) != (size_t)len) {
            free(data);
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    *buffer = data;
    *size = (size_t)len;
    return 0;
}

static int command_create(FILE *fp, const VfsHeader *header, const char *host_file, const char *vfs_name) {
    if (strlen(vfs_name) >= VFS_NAME_LEN) {
        fprintf(stderr, "Name too long (max %d chars)\n", VFS_NAME_LEN - 1);
        return 1;
    }

    VfsEntry entries[VFS_MAX_FILES];
    if (load_entries(fp, entries) != 0) {
        perror("load entries");
        return 1;
    }

    int free_index = -1;
    for (uint32_t i = 0; i < VFS_MAX_FILES; ++i) {
        if (entries[i].used && strcmp(entries[i].name, vfs_name) == 0) {
            fprintf(stderr, "File '%s' already exists in VFS\n", vfs_name);
            return 1;
        }
        if (!entries[i].used && free_index == -1) {
            free_index = (int)i;
        }
    }

    if (free_index == -1) {
        fprintf(stderr, "No free file slots in VFS\n");
        return 1;
    }

    unsigned char *data = NULL;
    size_t size = 0;
    if (read_whole_file(host_file, &data, &size) != 0) {
        perror("read host file");
        return 1;
    }

    if (size > UINT32_MAX) {
        fprintf(stderr, "File too large for this VFS format\n");
        free(data);
        return 1;
    }

    uint32_t offset = find_end_offset(header, entries);
    if (fseek(fp, (long)offset, SEEK_SET) != 0) {
        perror("fseek data");
        free(data);
        return 1;
    }

    if (size > 0 && fwrite(data, 1, size, fp) != size) {
        perror("write data");
        free(data);
        return 1;
    }
    free(data);

    VfsEntry *slot = &entries[free_index];
    memset(slot, 0, sizeof(*slot));
    strncpy(slot->name, vfs_name, VFS_NAME_LEN - 1);
    slot->offset = offset;
    slot->size = (uint32_t)size;
    slot->used = 1;

    if (save_entries(fp, entries) != 0) {
        perror("save entries");
        return 1;
    }

    printf("Added '%s' from '%s' (%u bytes)\n", vfs_name, host_file, slot->size);
    return 0;
}

static int command_read(FILE *fp, const char *vfs_name, const char *output_file) {
    VfsEntry entries[VFS_MAX_FILES];
    if (load_entries(fp, entries) != 0) {
        perror("load entries");
        return 1;
    }

    const VfsEntry *found = NULL;
    for (uint32_t i = 0; i < VFS_MAX_FILES; ++i) {
        if (entries[i].used && strcmp(entries[i].name, vfs_name) == 0) {
            found = &entries[i];
            break;
        }
    }

    if (found == NULL) {
        fprintf(stderr, "No such file in VFS: %s\n", vfs_name);
        return 1;
    }

    if (fseek(fp, (long)found->offset, SEEK_SET) != 0) {
        perror("fseek");
        return 1;
    }

    unsigned char *data = NULL;
    if (found->size > 0) {
        data = malloc(found->size);
        if (data == NULL) {
            perror("malloc");
            return 1;
        }

        if (fread(data, 1, found->size, fp) != found->size) {
            perror("fread");
            free(data);
            return 1;
        }
    }

    FILE *out = fopen(output_file, "wb");
    if (out == NULL) {
        perror(output_file);
        free(data);
        return 1;
    }

    if (found->size > 0 && fwrite(data, 1, found->size, out) != found->size) {
        perror("fwrite");
        free(data);
        fclose(out);
        return 1;
    }

    free(data);
    fclose(out);

    printf("Extracted '%s' to '%s' (%u bytes)\n", vfs_name, output_file, found->size);
    return 0;
}

static int command_delete(FILE *fp, const char *vfs_name) {
    VfsEntry entries[VFS_MAX_FILES];
    if (load_entries(fp, entries) != 0) {
        perror("load entries");
        return 1;
    }

    for (uint32_t i = 0; i < VFS_MAX_FILES; ++i) {
        if (entries[i].used && strcmp(entries[i].name, vfs_name) == 0) {
            memset(&entries[i], 0, sizeof(entries[i]));
            if (save_entries(fp, entries) != 0) {
                perror("save entries");
                return 1;
            }
            printf("Deleted '%s' from VFS index\n", vfs_name);
            return 0;
        }
    }

    fprintf(stderr, "No such file in VFS: %s\n", vfs_name);
    return 1;
}

static void print_usage(const char *prog) {
    fprintf(stderr,
            "Usage:\n"
            "  %s <image> init\n"
            "  %s <image> list\n"
            "  %s <image> create <host_file> [vfs_name]\n"
            "  %s <image> read <vfs_name> <output_file>\n"
            "  %s <image> delete <vfs_name>\n",
            prog, prog, prog, prog, prog);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *image_path = argv[1];
    const char *cmd = argv[2];

    if (strcmp(cmd, "init") == 0) {
        if (argc != 3) {
            print_usage(argv[0]);
            return 1;
        }
        return command_init(image_path);
    }

    FILE *fp = fopen(image_path, "rb+");
    if (fp == NULL) {
        perror(image_path);
        return 1;
    }

    VfsHeader header;
    if (load_header(fp, &header) != 0) {
        perror("Invalid or unreadable VFS image");
        fclose(fp);
        return 1;
    }

    int result = 0;

    if (strcmp(cmd, "list") == 0) {
        if (argc != 3) {
            print_usage(argv[0]);
            result = 1;
        } else {
            result = command_list(fp);
        }
    } else if (strcmp(cmd, "create") == 0) {
        if (argc != 4 && argc != 5) {
            print_usage(argv[0]);
            result = 1;
        } else {
            const char *host_file = argv[3];
            const char *vfs_name = (argc == 5) ? argv[4] : argv[3];
            result = command_create(fp, &header, host_file, vfs_name);
        }
    } else if (strcmp(cmd, "read") == 0) {
        if (argc != 5) {
            print_usage(argv[0]);
            result = 1;
        } else {
            result = command_read(fp, argv[3], argv[4]);
        }
    } else if (strcmp(cmd, "delete") == 0) {
        if (argc != 4) {
            print_usage(argv[0]);
            result = 1;
        } else {
            result = command_delete(fp, argv[3]);
        }
    } else {
        print_usage(argv[0]);
        result = 1;
    }

    fclose(fp);
    return result;
}
