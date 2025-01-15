#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include "hop.h"

#define PATH_MAX 4096

// Colors
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BLUE "\033[34m"

// Function to compare filenames for sorting
int compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

// Function to check if a file is hidden (starts with '.')
int is_hidden(const char *filename) {
    return filename[0] == '.';
}

// Function to check if the entry is a regular file using stat()
int is_regular_file(const char *path) {
    struct stat path_stat;
    if (stat(path, &path_stat) != 0) {
        return 0;  // Error with stat
    }
    return S_ISREG(path_stat.st_mode);
}

// Function to print file details in the -l flag format
void print_file_details(const char *path, const char *filename, int show_hidden) {
    struct stat file_stat;
    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, filename);

    if (stat(full_path, &file_stat) != 0) {
        return;  // Error with stat
    }

    // Skip hidden files if not showing hidden
    if (!show_hidden && is_hidden(filename)) {
        return;
    }

    // File type and permissions
    printf((S_ISDIR(file_stat.st_mode)) ? "d" : "-");
    printf((file_stat.st_mode & S_IRUSR) ? "r" : "-");
    printf((file_stat.st_mode & S_IWUSR) ? "w" : "-");
    printf((file_stat.st_mode & S_IXUSR) ? "x" : "-");
    printf((file_stat.st_mode & S_IRGRP) ? "r" : "-");
    printf((file_stat.st_mode & S_IWGRP) ? "w" : "-");
    printf((file_stat.st_mode & S_IXGRP) ? "x" : "-");
    printf((file_stat.st_mode & S_IROTH) ? "r" : "-");
    printf((file_stat.st_mode & S_IWOTH) ? "w" : "-");
    printf((file_stat.st_mode & S_IXOTH) ? "x" : "-");
    printf(" ");

    // Number of links
    printf("%2lu ", file_stat.st_nlink);

    // Owner and group
    printf("%s %s ", getpwuid(file_stat.st_uid)->pw_name, getgrgid(file_stat.st_gid)->gr_name);

    // File size
    printf("%5lu ", file_stat.st_size);

    // Last modification time
    char time_buf[80];
    struct tm *time_info;
    time_info = localtime(&file_stat.st_mtime);
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", time_info);
    printf("%s ", time_buf);

    // File or directory name with color coding
    if (S_ISDIR(file_stat.st_mode)) {
        printf(COLOR_BLUE "%s" COLOR_RESET "\n", filename);  // Directory (blue)
    } else if (file_stat.st_mode & S_IXUSR) {
        printf(COLOR_GREEN "%s" COLOR_RESET "\n", filename);  // Executable (green)
    } else {
        printf(COLOR_WHITE "%s" COLOR_RESET "\n", filename);  // Regular file (white)
    }
}

// Function to print files with or without -a (all files) and -l (extra details)
void print_files(const char *directory, bool show_hidden, bool long_format) {
    struct dirent *de;
    DIR *dr = opendir(directory);

    if (dr == NULL) {
        printf("Could not open directory: %s\n", directory);
        return;
    }

    // Store filenames
    char *files[1000];
    int count = 0;

    // Read files from directory
    while ((de = readdir(dr)) != NULL) {
        // Skip hidden files unless -a is specified
        if (!show_hidden && is_hidden(de->d_name)) {
            continue;
        }

        files[count] = strdup(de->d_name);  // Allocate memory for file name
        count++;
    }

    // Sort the filenames alphabetically
    qsort(files, count, sizeof(char *), compare);

    // Print sorted filenames
    for (int i = 0; i < count; i++) {
        if (long_format) {
            // Print file details with the -l flag
            print_file_details(directory, files[i], show_hidden);
        } else {
            // File or directory name with color coding
            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, files[i]);

            struct stat file_stat;
            stat(full_path, &file_stat);

            if (S_ISDIR(file_stat.st_mode)) {
                printf(COLOR_BLUE "%s" COLOR_RESET "\n", files[i]);  // Directory (blue)
            } else if (file_stat.st_mode & S_IXUSR) {
                printf(COLOR_GREEN "%s" COLOR_RESET "\n", files[i]);  // Executable (green)
            } else {
                printf(COLOR_WHITE "%s" COLOR_RESET "\n", files[i]);  // Regular file (white)
            }
        }
        free(files[i]);  // Free dynamically allocated memory
    }

    closedir(dr);
}

void reveal(char *path, bool show_long, bool show_all, char *olddir) {
    char temp[PATH_MAX];

    // Get current working directory
    if (getcwd(temp, sizeof(temp)) == NULL) {
        perror("getcwd error");
        return;
    }

    // Handle special cases for path: ".", "..", "-", "~"
    if (path == NULL || strcmp(path, ".") == 0) {
        path = temp;  // Current directory
    } else if (strcmp(path, "-") == 0) {
        chdir(olddir);
        getcwd(temp, sizeof(temp));  // Update path
        path = temp;
    } else if (strcmp(path, "~") == 0) {
        homedir(temp);  // Change to home directory
        path = temp;
    } else if (strcmp(path, "..") == 0) {
        chdir("..");
        getcwd(temp, sizeof(temp));  // Update path
        path = temp;
    }

    // Check if the path is valid and exists
    struct stat path_stat;
    if (stat(path, &path_stat) != 0 || !S_ISDIR(path_stat.st_mode)) {
        printf("Invalid path: %s\n", path);
        return;
    }

    // Print files in the specified directory
    print_files(path, show_all, show_long);
}
