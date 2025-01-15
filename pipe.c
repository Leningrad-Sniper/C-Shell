#include "pipe.h"
#include "hop.h"
#include "reveal.h"
#include "seek.h"
#include "proclore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// Handle redirection inside a command
void handle_redirection(char *command) {
    char *input_file = NULL;
    char *output_file = NULL;
    int output_append = 0;

    // Check for input redirection
    char *input_pos = strchr(command, '<');
    if (input_pos) {
        *input_pos = '\0';  // Terminate the command before '<'
        input_pos++;
        while (*input_pos == ' ') input_pos++;  // Skip spaces
        input_file = strtok(input_pos, " ");  // Get the input file
    }

    // Check for output redirection (either '>' or '>>')
    char *output_pos = strchr(command, '>');
    if (output_pos) {
        *output_pos = '\0';  // Terminate the command before '>'
        output_pos++;
        if (*output_pos == '>') {  // If it's '>>', move the pointer
            output_append = 1;
            output_pos++;
        }
        while (*output_pos == ' ') output_pos++;  // Skip spaces
        output_file = strtok(output_pos, " ");  // Get the output file
    }

    // Perform input redirection
    if (input_file) {
        int fd_in = open(input_file, O_RDONLY);
        if (fd_in < 0) {
            perror("Input redirection failed");
            exit(EXIT_FAILURE);
        }
        dup2(fd_in, STDIN_FILENO);
        close(fd_in);
    }

    // Perform output redirection
    if (output_file) {
        int fd_out;
        if (output_append) {
            fd_out = open(output_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
        } else {
            fd_out = open(output_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
        }
        if (fd_out < 0) {
            perror("Output redirection failed");
            exit(EXIT_FAILURE);
        }
        dup2(fd_out, STDOUT_FILENO);
        close(fd_out);
    }
}

// Forward declaration of execute function
extern void execute(char* input, char* log[], int* commands, char* olddir);

// Function to handle multiple pipes with I/O redirection
int execute_multiple_pipes(char* commands[], int num_commands, char* log[], int* command_count, char* olddir) {
    int pipefds[2 * (num_commands - 1)];  // Array to store pipe file descriptors

    // Create pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("Pipe creation failed");
            return -1;
        }
    }

    int commandc = 0;
    while (commandc < num_commands) {
        pid_t pid = fork();

        if (pid == 0) {  // Child process
            // Handle redirection for the first and last commands
            if (commandc == 0 || commandc == num_commands - 1) {
                handle_redirection(commands[commandc]);  // Perform I/O redirection if any
            }

            // If it's not the first command, redirect input from the previous pipe
            if (commandc > 0) {
                if (dup2(pipefds[(commandc - 1) * 2], STDIN_FILENO) < 0) {
                    perror("dup2 input failed");
                    exit(EXIT_FAILURE);
                }
            }

            // If it's not the last command, redirect output to the next pipe
            if (commandc < num_commands - 1) {
                if (dup2(pipefds[commandc * 2 + 1], STDOUT_FILENO) < 0) {
                    perror("dup2 output failed");
                    exit(EXIT_FAILURE);
                }
            }

            // Close all pipe file descriptors in the child process
            for (int i = 0; i < 2 * (num_commands - 1); i++) {
                close(pipefds[i]);
            }

            // Execute the current command
            execute(commands[commandc], log, command_count, olddir);
            exit(EXIT_SUCCESS);

        } else if (pid < 0) {  // Fork failed
            perror("Fork failed");
            return -1;
        }

        // Move to the next command
        commandc++;
    }

    // Parent closes all pipe file descriptors
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefds[i]);
    }

    // Parent waits for all child processes to complete
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }

    return 0;
}
