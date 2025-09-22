#include "proc_reader.h"

// function to list all the process directories in /proc
int list_process_directories(void) {
    DIR *dir;                   // directory pointer
    struct dirent *entry;       // used to hold entries (files/folders) from /proc
    int count = 0;              // keep track of how many processes we find

    // try to open /proc
    dir = opendir("/proc");
    if (dir == NULL) {
        perror("could not open /proc (sad)");
        return -1;
    }

    // headers for the table
    printf("Process directories in /proc:\n");
    printf("%-8s %-20s\n", "PID", "Type");
    printf("%-8s %-20s\n", "---", "----");

    // loop through everything in /proc
    while ((entry = readdir(dir)) != NULL) {
        // only print if the folder name is a number (that means it's a PID)
        if (is_number(entry->d_name)) {
            printf("%-8s %-20s\n", entry->d_name, "process");
            count++;
        }
    }

    // close it when we're done
    if (closedir(dir) == -1) {
        perror("could not close /proc (double sad)");
        return -1;
    }

    // print how many we found
    printf("Found %d process directories\n", count);
    return 0;
}

// show info for a specific process (status + cmdline)
int read_process_info(const char* pid) {
    char filepath[256];   // buffer to build paths like /proc/[pid]/status

    // build path for the status file
    snprintf(filepath, sizeof(filepath), "/proc/%s/status", pid);

    printf("\n--- Process Information for PID %s ---\n", pid);

    // use our helper that reads files with system calls
    if (read_file_with_syscalls(filepath) == -1) {
        fprintf(stderr, "could not read %s\n", filepath);
        return -1;
    }

    // now build path for the cmdline file
    snprintf(filepath, sizeof(filepath), "/proc/%s/cmdline", pid);

    printf("\n--- Command Line ---\n");

    if (read_file_with_syscalls(filepath) == -1) {
        fprintf(stderr, "could not read %s\n", filepath);
        return -1;
    }

    printf("\n"); // extra space for nice formatting
    return 0;
}

// show system info (cpu + memory) but only first 10 lines
int show_system_info(void) {
    const int MAX_LINES = 10;    // we don’t want to spam everything
    char line[256];              // line buffer
    FILE *file;                  // file pointer

    // open cpuinfo
    printf("\n--- CPU Information (first %d lines) ---\n", MAX_LINES);
    file = fopen("/proc/cpuinfo", "r");
    if (!file) {
        perror("could not open /proc/cpuinfo");
        return -1;
    }

    for (int i = 0; i < MAX_LINES && fgets(line, sizeof(line), file); i++) {
        printf("%s", line);
    }
    fclose(file);

    // open meminfo
    printf("\n--- Memory Information (first %d lines) ---\n", MAX_LINES);
    file = fopen("/proc/meminfo", "r");
    if (!file) {
        perror("could not open /proc/meminfo");
        return -1;
    }

    for (int i = 0; i < MAX_LINES && fgets(line, sizeof(line), file); i++) {
        printf("%s", line);
    }
    fclose(file);

    return 0;
}

// compare system calls vs library calls for reading a file
void compare_file_methods(void) {
    const char* test_file = "/proc/version";

    printf("Comparing file reading methods for: %s\n\n", test_file);

    // method 1
    printf("=== Method 1: Using System Calls ===\n");
    read_file_with_syscalls(test_file);

    // method 2
    printf("\n=== Method 2: Using Library Functions ===\n");
    read_file_with_library(test_file);

    // friendly reminder
    printf("\nNOTE: Run this program with strace to see the difference!\n");
    printf("Example: strace -e trace=openat,read,write,close ./lab2\n");
}

// actually read a file using syscalls (open/read/close)
int read_file_with_syscalls(const char* filename) {
    int fd;
    char buffer[1024];       // chunk buffer
    ssize_t bytes_read;      // how many bytes we got back

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        return -1;
    }

    // read the file until there's nothing left
    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';   // null terminate like a good C programmer
        printf("%s", buffer);
    }

    if (bytes_read == -1) {
        perror("read failed (oops)");
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        perror("close failed (super oops)");
        return -1;
    }

    return 0;
}

// same thing but using fancy C library functions (fopen/fgets/fclose)
int read_file_with_library(const char* filename) {
    FILE *file;
    char buffer[256];

    file = fopen(filename, "r");
    if (!file) {
        perror("fopen failed");
        return -1;
    }

    // fgets reads one line at a time
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }

    if (fclose(file) == EOF) {
        perror("fclose failed (who even fails closing!?)");
        return -1;
    }

    return 0;
}

// helper function to check if a string is just digits
int is_number(const char* str) {
    if (!str || *str == '\0') return 0;   // empty? not a number

    while (*str) {
        if (!isdigit((unsigned char)*str)) return 0; // not a digit
        str++;
    }

    return 1;   // if we made it here, it’s all numbers
}
