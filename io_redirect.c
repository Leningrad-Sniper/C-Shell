#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "io_redirect.h"

// Function to handle I/O redirection
int handle_io_redirection(char **args) {
    int i = 0;
    int in_fd = -1, out_fd = -1;
    int io_redirect = 0;

    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0) {
            // Output redirection (overwrite)
            io_redirect = 1;
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Expected filename after >\n");
                return -1;
            }
            out_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd < 0) {
                perror("open failed");
                return -1;
            }
            dup2(out_fd, STDOUT_FILENO);  // Redirect stdout to the file
            close(out_fd);
            args[i] = NULL;  // Remove redirection arguments
        } else if (strcmp(args[i], ">>") == 0) {
            // Output redirection (append)
            io_redirect = 1;
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Expected filename after >>\n");
                return -1;
            }
            out_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (out_fd < 0) {
                perror("open failed");
                return -1;
            }
            dup2(out_fd, STDOUT_FILENO);  // Redirect stdout to the file
            close(out_fd);
            args[i] = NULL;  // Remove redirection arguments
        } else if (strcmp(args[i], "<") == 0) {
            // Input redirection
            io_redirect = 1;
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Expected filename after <\n");
                return -1;
            }
            in_fd = open(args[i + 1], O_RDONLY);
            if (in_fd < 0) {
                perror("open failed");
                fprintf(stderr, "No such input file found!\n");
                return -1;
            }
            dup2(in_fd, STDIN_FILENO);  // Redirect stdin from the file
            close(in_fd);
            args[i] = NULL;  // Remove redirection arguments
        }
        i++;
    }
    return io_redirect;
}
