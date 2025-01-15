#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include "seek.h"

#define PATH_MAX 4096
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_BLUE "\033[34m"

// Function prototypes
void search_directory(const char *dir_path, const char *search_target, bool dir_only, bool file_only, bool execute_flag, int *file_count, int *dir_count, char *matched_file, char *matched_dir);
void print_result(const char *path, bool is_dir);
bool check_permissions(const char *path, bool is_dir);
bool is_matching(const char *name, const char *search_target);

// Function to initiate the seek operation
void seek(char *search_target, bool d, bool e, bool f, char *start_directory) {
    char search_path[PATH_MAX];
    int file_count = 0, dir_count = 0;
    char matched_file[PATH_MAX] = "";
    char matched_dir[PATH_MAX] = "";

    // Handle case where no start directory is specified (use current directory)
    if (start_directory == NULL) {
        getcwd(search_path, sizeof(search_path));  // Get current working directory
    } else {
        // Handle special directories (., ~, .., etc.)
        if (strcmp(start_directory, ".") == 0 || strcmp(start_directory, "..") == 0 || strcmp(start_directory, "~") == 0) {
            realpath(start_directory, search_path);  // Convert to absolute path
        } else {
            strcpy(search_path, start_directory);  // Use specified path
        }
    }

    // Start searching the directory recursively
    search_directory(search_path, search_target, d, f, e, &file_count, &dir_count, matched_file, matched_dir);

    // Handle no match found
    if (file_count == 0 && dir_count == 0) {
        printf("No match found!\n");
        return;
    }

    // If -e flag is enabled, handle file or directory actions
    if (e) {
        if (file_count == 1 && dir_count == 0) {
            // Single file found, print its content
            printf("%s\n", matched_file);
            if (check_permissions(matched_file, false)) {
                FILE *file = fopen(matched_file, "r");
                if (file != NULL) {
                    char line[1024];
                    while (fgets(line, sizeof(line), file)) {
                        printf("%s", line);
                    }
                    fclose(file);
                }
            } else {
                printf("Missing permissions for task!\n");
            }
        } else if (dir_count == 1 && file_count == 0) {
            // Single directory found, change working directory
            printf("%s/\n", matched_dir);
            if (check_permissions(matched_dir, true)) {
                chdir(matched_dir);
                char new_cwd[PATH_MAX];
                getcwd(new_cwd, sizeof(new_cwd));
                printf("<JohnDoe@SYS:%s> ", new_cwd);
            } else {
                printf("Missing permissions for task!\n");
            }
        }
    }
}

// Function to recursively search the directory
void search_directory(const char *dir_path, const char *search_target, bool dir_only, bool file_only, bool execute_flag, int *file_count, int *dir_count, char *matched_file, char *matched_dir) {
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        return;  // Could not open directory
    }

    struct dirent *entry;
    char path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the full path
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat path_stat;
        stat(path, &path_stat);

        // If it's a directory, recurse into it
        if (S_ISDIR(path_stat.st_mode)) {
            if (!file_only && is_matching(entry->d_name, search_target)) {
                (*dir_count)++;
                print_result(path, true);
                if (execute_flag) strcpy(matched_dir, path);
            }
            search_directory(path, search_target, dir_only, file_only, execute_flag, file_count, dir_count, matched_file, matched_dir);
        } else if (S_ISREG(path_stat.st_mode)) {
            if (!dir_only && is_matching(entry->d_name, search_target)) {
                (*file_count)++;
                print_result(path, false);
                if (execute_flag) strcpy(matched_file, path);
            }
        }
    }

    closedir(dir);
}

// Function to check if the file/directory name exactly matches the search target
bool is_matching(const char *name, const char *search_target) {
    return strcmp(name, search_target) == 0;  // Exact match
}

// Function to print the result (colored)
void print_result(const char *path, bool is_dir) {
    if (is_dir) {
        printf(COLOR_BLUE "%s/" COLOR_RESET "\n", path);
    } else {
        printf(COLOR_GREEN "%s" COLOR_RESET "\n", path);
    }
}

// Function to check permissions for file or directory
bool check_permissions(const char *path, bool is_dir) {
    if (is_dir) {
        if (access(path, X_OK) != 0) {
            return false;
        }
    } else {
        if (access(path, R_OK) != 0) {
            return false;
        }
    }
    return true;
}
