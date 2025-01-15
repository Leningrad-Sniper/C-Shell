#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define PATH_MAX 4096

// Function to get the status from a status character
const char* get_status_string(char state) {
    switch (state) {
        case 'R':
            return "Running";
        case 'S':
            return "Sleeping";
        case 'Z':
            return "Zombie";
        default:
            return "Unknown";
    }
}

// Function to check if a process is in the foreground
int is_foreground(int pid) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return 0;
    }
    
    char buffer[4096];
    read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    
    buffer[sizeof(buffer) - 1] = '\0';
    
    int pid_read, ppid, pgrp, session, tty_nr;
    sscanf(buffer, "%d %*s %*c %d %d %d %d", &pid_read, &ppid, &pgrp, &session, &tty_nr);
    
    // If tty_nr is not 0, it's a foreground process
    return tty_nr != 0;
}

// Function to print process information
void proclore(int pid) {
    char path[PATH_MAX];
    char buffer[4096];
    int fd;

    // Open and read the /proc/[pid]/stat file
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening stat file");
        return;
    }

    read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    int pid_read, ppid, pgrp, session, tty_nr;
    unsigned long vsize;
    char state;
    char comm[1024];

    sscanf(buffer, "%d %s %c %d %d %d %d %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lu", 
           &pid_read, comm, &state, &ppid, &pgrp, &session, &tty_nr, &vsize);

    // Get the executable path
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    char exec_path[PATH_MAX];
    ssize_t len = readlink(path, exec_path, sizeof(exec_path) - 1);
    if (len != -1) {
        exec_path[len] = '\0';
    } else {
        strcpy(exec_path, "Unknown");
    }

    // Get the username of the process owner
    struct passwd *pw = getpwuid(geteuid());

    // Print the process information
    printf("pid: %d\n", pid_read);
    printf("Process Status: %c%s (%s)\n", state, is_foreground(pid_read) ? "+" : "", get_status_string(state));
    printf("Process Group: %d\n", pgrp);
    printf("Virtual Memory: %lu\n", vsize);
    printf("Executable Path: %s\n", exec_path);
    printf("Username: %s\n", pw->pw_name);
}

// Command to handle proclore
void proclore_command(char *pid_str) {
    int pid;

    if (pid_str == NULL) {
        // If no PID is provided, use the current process's PID
        pid = getpid();
    } else {
        // Convert PID string to an integer
        pid = atoi(pid_str);
    }

    proclore(pid);
}