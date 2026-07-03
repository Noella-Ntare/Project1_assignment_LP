#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>

#define BACKUP_DIR  "/tmp/backup_out"
#define LOG_FILE    "/tmp/backup.log"
#define SOURCE_FILE "/tmp/backup_source.txt"
#define BACKUP_FILE "/tmp/backup_out/backup.txt"

void log_message(const char *msg) {
    FILE *log = fopen(LOG_FILE, "a");
    if (!log) return;
    time_t now = time(NULL);
    char ts[32];
    strftime(ts, sizeof(ts), "%H:%M:%S", localtime(&now));
    fprintf(log, "[%s] %s\n", ts, msg);
    fclose(log);
}

void create_source_file() {
    FILE *f = fopen(SOURCE_FILE, "w");
    if (!f) { perror("create source"); return; }
    fprintf(f, "Important data line 1\n");
    fprintf(f, "Important data line 2\n");
    fprintf(f, "Important data line 3\n");
    fclose(f);
    log_message("Source file created");
}

void perform_backup() {
    // Open source for reading
    FILE *src = fopen(SOURCE_FILE, "r");
    if (!src) { log_message("ERROR: source open failed"); return; }

    // Open destination for writing
    FILE *dst = fopen(BACKUP_FILE, "w");
    if (!dst) { fclose(src); log_message("ERROR: dest open failed"); return; }

    // Read and write
    char line[256];
    int lines = 0;
    while (fgets(line, sizeof(line), src)) {
        fputs(line, dst);
        lines++;
    }

    fclose(src);
    fclose(dst);

    char msg[128];
    snprintf(msg, sizeof(msg), "Backup complete: %d lines copied", lines);
    log_message(msg);
}

void read_backup_verify() {
    FILE *f = fopen(BACKUP_FILE, "r");
    if (!f) { log_message("Verify: backup not found"); return; }

    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), f)) count++;
    fclose(f);

    char msg[128];
    snprintf(msg, sizeof(msg), "Verification: %d lines in backup", count);
    log_message(msg);
    printf("Backup verified: %d lines\n", count);
}

int main() {
    printf("Backup Tool starting...\n");
    log_message("Backup tool started");

    // Ensure backup directory exists
    mkdir(BACKUP_DIR, 0755);

    // Step 1: Create a source file
    create_source_file();
    printf("Source file created: %s\n", SOURCE_FILE);

    // Step 2: Perform backup
    perform_backup();
    printf("Backup written to: %s\n", BACKUP_FILE);

    // Step 3: Read and verify backup
    read_backup_verify();

    log_message("Backup tool finished");
    printf("Done.\n");
    return 0;
}
