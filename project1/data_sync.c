#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_PATH 256
#define LOG_FILE "/tmp/data_sync.log"
#define SYNC_DIR "/tmp/sync_target"

void write_log(const char *message) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        time_t now = time(NULL);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(log, "[%s] %s\n", timestamp, message);
        fclose(log);
    }
}

int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

void sync_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) {
        write_log("ERROR: Could not open source file");
        return;
    }

    FILE *out = fopen(dst, "wb");
    if (!out) {
        write_log("ERROR: Could not open destination file");
        fclose(in);
        return;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        fwrite(buffer, 1, bytes, out);
    }

    fclose(in);
    fclose(out);
    write_log("File synced successfully");
}

void create_sync_dir() {
    if (!file_exists(SYNC_DIR)) {
        mkdir(SYNC_DIR, 0755);
        write_log("Created sync directory");
    }
}

int main(int argc, char *argv[]) {
    printf("data_sync v1.0 - File Synchronization Tool\n");
    printf("Initializing sync process...\n");

    write_log("data_sync started");
    create_sync_dir();

    if (argc < 2) {
        printf("Usage: %s <source_file> [destination]\n", argv[0]);
        printf("Defaulting to demo mode...\n");

        // Demo: create a test file and sync it
        const char *demo_src = "/tmp/demo_source.txt";
        FILE *demo = fopen(demo_src, "w");
        if (demo) {
            fprintf(demo, "Demo file for synchronization test\n");
            fclose(demo);
        }

        char dst_path[MAX_PATH];
        snprintf(dst_path, sizeof(dst_path), "%s/demo_source.txt", SYNC_DIR);
        sync_file(demo_src, dst_path);
        printf("Demo sync completed. Check %s\n", SYNC_DIR);
    } else {
        char dst_path[MAX_PATH];
        if (argc >= 3) {
            strncpy(dst_path, argv[2], MAX_PATH - 1);
        } else {
            const char *basename = strrchr(argv[1], '/');
            basename = basename ? basename + 1 : argv[1];
            snprintf(dst_path, sizeof(dst_path), "%s/%s", SYNC_DIR, basename);
        }
        sync_file(argv[1], dst_path);
        printf("Sync complete: %s -> %s\n", argv[1], dst_path);
    }

    write_log("data_sync finished");
    printf("Sync operation complete.\n");
    return 0;
}
