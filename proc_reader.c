#include "proc_reader.h"

int list_process_directories(void) {
    // Open the /proc directory using opendir()
    DIR *dir = opendir("/proc");

    // Check if opendir() failed and print error message
    if (dir == NULL) {
        perror("opendir(/proc)");
        return -1;
    }

    // Declare a struct dirent pointer for directory entries
    struct dirent *entry;

    // Initialize process counter to 0
    int count = 0;

    printf("Process directories in /proc:\n");
    printf("%-8s %-20s\n", "PID", "Type");
    printf("%-8s %-20s\n", "---", "----");

    // Read directory entries using readdir() in a loop
    while ((entry = readdir(dir)) != NULL) {

        // For each entry, check if the name is a number using is_number()
        if (is_number(entry->d_name)) {

            // If it's a number, print it as a PID and increment counter
            printf("%-8s %-20s\n", entry->d_name, "process");
            count++;
        }
    }

    // Close the directory using closedir()
    if (closedir(dir) != 0) {
        perror("closedir(/proc)");
        return -1;
    }

    // Print the total count of process directories found
    printf("Found %d process directories\n", count);

    return 0;
}

int read_process_info(const char* pid) {
    char filepath[256];

    // Create the path to /proc/[pid]/status using snprintf()
    snprintf(filepath, sizeof(filepath), "/proc/%s/status", pid);

    printf("\n--- Process Information for PID %s ---\n", pid);

    // Call read_file_with_syscalls() to read the status file
    if (read_file_with_syscalls(filepath) != 0) {
        fprintf(stderr, "Failed to read %s\n", filepath);
        return -1;
    }

    // Create the path to /proc/[pid]/cmdline using snprintf()
    snprintf(filepath, sizeof(filepath), "/proc/%s/cmdline", pid);

    printf("\n--- Command Line ---\n");

    // Special handling for cmdline because it is NUL-separated
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("open cmdline");
        return -1;
    }

    char buffer[4096];
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);

    if (bytes < 0) {
        perror("read cmdline");
        close(fd);
        return -1;
    }

    close(fd);

    if (bytes == 0) {
        printf("(empty)\n");
    } else {
        buffer[bytes] = '\0';

        // Replace internal null characters with spaces
        for (ssize_t i = 0; i < bytes; i++) {
            if (buffer[i] == '\0') {
                buffer[i] = ' ';
            }
        }

        printf("%s\n", buffer);
    }

    printf("\n"); // Add extra newline for readability

    return 0;
}

int show_system_info(void) {
    int line_count = 0;
    const int MAX_LINES = 10;

    printf("\n--- CPU Information (first %d lines) ---\n", MAX_LINES);

    // Open /proc/cpuinfo using fopen() with "r" mode
    FILE *cpu = fopen("/proc/cpuinfo", "r");

    // Check if fopen() failed
    if (cpu == NULL) {
        perror("fopen(/proc/cpuinfo)");
        return -1;
    }

    // Declare a char array for reading lines
    char line[256];

    // Read lines using fgets() in a loop, limit to MAX_LINES
    while (line_count < MAX_LINES && fgets(line, sizeof(line), cpu) != NULL) {
        printf("%s", line);
        line_count++;
    }

    // Close the file using fclose()
    if (fclose(cpu) != 0) {
        perror("fclose(/proc/cpuinfo)");
        return -1;
    }

    printf("\n--- Memory Information (first %d lines) ---\n", MAX_LINES);

    // Open /proc/meminfo using fopen() with "r" mode
    FILE *mem = fopen("/proc/meminfo", "r");

    // Check if fopen() failed
    if (mem == NULL) {
        perror("fopen(/proc/meminfo)");
        return -1;
    }

    // Reset line count
    line_count = 0;

    // Read lines using fgets() in a loop, limit to MAX_LINES
    while (line_count < MAX_LINES && fgets(line, sizeof(line), mem) != NULL) {
        printf("%s", line);
        line_count++;
    }

    // Close the file using fclose()
    if (fclose(mem) != 0) {
        perror("fclose(/proc/meminfo)");
        return -1;
    }

    return 0;
}

void compare_file_methods(void) {
    const char* test_file = "/proc/version";

    printf("Comparing file reading methods for: %s\n\n", test_file);

    printf("=== Method 1: Using System Calls ===\n");
    read_file_with_syscalls(test_file);

    printf("\n=== Method 2: Using Library Functions ===\n");
    read_file_with_library(test_file);

    printf("\nNOTE: Run this program with strace to see the difference!\n");
    printf("Example: strace -e trace=openat,read,write,close ./lab2\n");
}

int read_file_with_syscalls(const char* filename) {
    // Declare variables
    int fd;
    char buffer[1024];
    ssize_t bytes_read;

    // Open the file using open() with O_RDONLY flag
    fd = open(filename, O_RDONLY);

    // Check if open() failed
    if (fd == -1) {
        perror("open");
        return -1;
    }

    // Read the file in a loop using read()
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {

        // Null-terminate the buffer after each read
        buffer[bytes_read] = '\0';

        // Print each chunk of data read
        printf("%s", buffer);
    }

    // Handle read() errors
    if (bytes_read == -1) {
        perror("read");
        close(fd);
        return -1;
    }

    // Close the file using close()
    if (close(fd) == -1) {
        perror("close");
        return -1;
    }

    return 0;
}

int read_file_with_library(const char* filename) {
    // Declare variables
    FILE *file;
    char buffer[256];

    // Open the file using fopen() with "r" mode
    file = fopen(filename, "r");

    // Check if fopen() failed
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    // Read the file using fgets() in a loop
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    // Close the file using fclose()
    if (fclose(file) != 0) {
        perror("fclose");
        return -1;
    }

    return 0;
}

int is_number(const char* str) {
    // Handle empty strings
    if (str == NULL || *str == '\0') {
        return 0;
    }

    // Check if the string contains only digits
    while (*str != '\0') {
        if (!isdigit((unsigned char)*str)) {
            return 0;
        }
        str++;
    }

    return 1;
}

