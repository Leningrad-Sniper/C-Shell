
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sys/wait.h>
#include<signal.h>
#include<time.h>
#include "reveal.h"
#include "proclore.h"
#include "seek.h"
#include "io_redirect.h"
#include "hop.h"
#include "neonate.h"
#include "pipe.h"
#include "signal.h"
#define PATH_MAX 4096
#define LOG_SIZE 15  // Maximum number of commands to store in the log
#define MAX_PROCESSES 100  // Maximum number of background processes

typedef struct {
    pid_t pid;
    char command[1024];  // Store command name
    bool is_running;     // true if running, false if stopped
} Process;



Process processes[MAX_PROCESSES];  // Array to store the processes
int process_count = 0;

void print_details()
{
    char *blue = "\033[36m";
    char *reset = "\033[0m";
    char *str;
    str = getlogin(); 
    char hostname[1024];  // Buffer to store the hostname
    gethostname(hostname, sizeof(hostname)); 
    printf("%s%s%s", blue, str, reset);
    printf("@");
    printf("%s", hostname);  // Print the hostname
    printf(":~>");
}
// Comparison function for qsort
int compare_processes(const void *a, const void *b) {
    return strcmp(((Process *)a)->command, ((Process *)b)->command);
}

// Function to display all processes
void display_activities() {
    if (process_count == 0) {
        printf("No processes found\n");
        return;
    }

    // Sort processes by command name
    qsort(processes, process_count, sizeof(Process), compare_processes);

    // Display each process
    for (int i = 0; i < process_count; i++) {
        printf("[%d] : %s - %s\n", processes[i].pid, processes[i].command,
               processes[i].is_running ? "Running" : "Stopped");
    }
}

void fg(int pid) {
    int found = 0;
    printf("fg\n");
    // Search for the process in the list
    for (int i = 0; i < process_count; i++) {
        if (processes[i].pid == pid) {
            found = 1;

            // Move the process to the foreground
            tcsetpgrp(STDIN_FILENO, pid);

            // Resume the process if it was stopped
            kill(pid, SIGCONT);

            // Wait for the process to complete or stop
            int status;
            waitpid(pid, &status, WUNTRACED);

            // After the process finishes or is stopped, return terminal control to the shell
            tcsetpgrp(STDIN_FILENO, getpid());

            // Update process status in the list
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                processes[i].is_running = false;
                printf("Process %d finished\n", pid);
            } else if (WIFSTOPPED(status)) {
                processes[i].is_running = false;
                printf("Process %d stopped\n", pid);
            }

            break;
        }
    }

    if (!found) {
        printf("No such process found\n");
    }
}

void bg(int pid) {
    int found = 0;

    // Search for the process in the list
    for (int i = 0; i < process_count; i++) {
        if (processes[i].pid == pid) {
            found = 1;

            // Resume the process in the background
            kill(pid, SIGCONT);

            // Update the process state
            processes[i].is_running = true;
            printf("Process %d resumed in background\n", pid);

            break;
        }
    }

    if (!found) {
        printf("No such process found\n");
    }
}

// Signal handler for background processes
void sigchld_handler(int signum) {
    int status;
    pid_t pid;

    // Reap all finished child processes
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        for (int i = 0; i < process_count; i++) {
            if (processes[i].pid == pid) {
                if (WIFEXITED(status)) {
                    printf("\nProcess %d: %s exited normally\n", pid, processes[i].command);
                    processes[i].is_running = false;
                } else if (WIFSIGNALED(status)) {
                    printf("\nProcess %d: %s exited abnormally\n", pid, processes[i].command);
                    processes[i].is_running = false;
                } else if (WIFSTOPPED(status)) {
                    printf("\nProcess %d: %s stopped\n", pid, processes[i].command);
                    processes[i].is_running = false;
                }
            }
        }
        // Print new prompt after background process finishes
        print_details();
        fflush(stdout);
    }
}


// Function to execute system commands
void execute_system_command(char *input, int background) {
    pid_t pid;
    time_t start_time, end_time;

    // Tokenize input into command and arguments
    char *args[100];
    int i = 0;
    char *token = strtok(input, " ");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // Fork a new process
    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return;
    }

    if (pid == 0) {
        // Child process

        // Handle I/O redirection
        handle_io_redirection(args);

        // Execute the command
        if (execvp(args[0], args) == -1) {
            perror("Command execution failed");
            exit(EXIT_FAILURE);
        }
    } else {
        if (background) {
            // Background process: don't wait, just print the PID and add it to process list
            printf("Started background process with PID %d\n", pid);

            // Add to process list
            processes[process_count].pid = pid;
            strcpy(processes[process_count].command, args[0]);  // Store the command name
            processes[process_count].is_running = true;         // Mark as running
            process_count++;

        } else {
            // Foreground process: wait for the process to complete and measure the time
            start_time = time(NULL);
            waitpid(pid, NULL, 0);  // Wait for the child to finish
            end_time = time(NULL);

            // Check if it took more than 2 seconds
            int time_taken = (int)difftime(end_time, start_time);
            if (time_taken > 2) {
                printf("Foreground process took %ds to execute\n", time_taken);
            }
        }
    }
}


// Function to execute commands and maintain the log
void execute(char* input, char* log[], int* commands, char* olddir) {
    char* token;

    // Duplicate the input before tokenization to preserve the full command for the log
    char* original_input = strdup(input);
    
    char* pipe_pos = strstr(input, "|");
    if (pipe_pos != NULL) {
        // Split the input command by the pipe symbol '|'
        char* command_list[100];  // To store the list of commands between pipes
        int num_commands = 0;

        char* command = strtok(input, "|");
        while (command != NULL) {
            // Trim leading/trailing spaces
            while (*command == ' ') command++;
            while (command[strlen(command) - 1] == ' ') command[strlen(command) - 1] = '\0';

            if (strlen(command) == 0) {
                printf("Invalid use of pipe\n");
                free(original_input);
                return;
            }

            command_list[num_commands++] = strdup(command);
            command = strtok(NULL, "|");
        }

        // Log the full input command
        log[*commands % LOG_SIZE] = original_input;  // Store the full command in the log
        *commands += 1;  // Increment command count

        // Execute the multiple pipe command
        execute_multiple_pipes(command_list, num_commands, log, commands, olddir);

        // Free memory used for commands
        for (int i = 0; i < num_commands; i++) {
            free(command_list[i]);
        }

        free(original_input);
        return;
    }

    // Tokenize the input
    token = strtok(input, " ");
    
    // Handle custom commands
    if (strcmp(token, "hop") == 0 || strcmp(token, "reveal") == 0 || strcmp(token, "log") == 0 || strcmp(token, "proclore") == 0 || strcmp(token, "seek") == 0 || strcmp(token, "activities") == 0  || strcmp(token, "neonate") == 0 || strcmp(token, "exit") == 0  || strcmp(token, "fg") == 0 || strcmp(token, "bg") == 0 || strcmp(token, "ping") == 0) {
        // Log the command
        log[*commands % LOG_SIZE] = original_input;  // Store the full command in the log
        *commands += 1;  // Increment command count

        if (strcmp(token, "hop") == 0) {
            token = strtok(NULL, " ");
            while (token != NULL) {
                olddir = hop(olddir, token);
                token = strtok(NULL, " ");
            }
            // (existing code)
        } else if (strcmp(token, "reveal") == 0) {
            bool l = false, a = false;
            token = strtok(NULL, " ");
            while (token != NULL && *token != '\0') {
                if (strchr(token, 'l')) l = true;
                if (strchr(token, 'a')) a = true;
                token = strtok(NULL, " ");
            }
            reveal(token, l, a, olddir);
            // (existing code)
        } else if (strcmp(token, "log") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                // Default log: print the log history (most recent first)
                int start = (*commands > LOG_SIZE) ? *commands - LOG_SIZE : 0;
                for (int i = *commands - 1; i >= start; i--) {
                    printf("%d: %s\n", *commands - i, log[i % LOG_SIZE]);
                }
            } else if (strcmp(token, "purge") == 0) {
                // Purge the log
                *commands = 0;
                printf("Log purged.\n");
            } else if (strcmp(token, "execute") == 0) {
                // Execute a specific command from the log
                token = strtok(NULL, " ");
                if (token != NULL) {
                    int index = atoi(token);
                    int total_entries = (*commands > LOG_SIZE) ? LOG_SIZE : *commands;
                    if (index < 1 || index > total_entries) {
                        printf("Invalid log index. There are only %d commands in the log.\n", total_entries);
                    } else {
                        char* cmd_to_execute = log[(*commands - index) % LOG_SIZE];
                        printf("Executing command: %s\n", cmd_to_execute);
                        execute(strdup(cmd_to_execute), log, commands, olddir);
                    }
                } else {
                    printf("Usage: log execute <index>\n");
                }
            }
            return;  // Don't log the 'log' command itself
        
        
        // Store the full command in the log
    

            
            // (existing code)
        } else if (strcmp(token, "proclore") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                proclore(getpid());
            }
            else {
                proclore_command(token);
            }
            // (existing code)
        } else if (strcmp(token, "seek") == 0) {
            bool d = false, e = false, f = false;
            char *search_file = NULL;
            char *start_directory = NULL;

            token = strtok(NULL, " ");
            
            // Parse flags and search target
            while(token != NULL && *token != '\0') {
                if (strchr(token, 'd')) d = true;
                if (strchr(token, 'e')) e = true;
                if (strchr(token, 'f')) f = true;
                if (token[0] != '-') {
                    search_file = strdup(token);  // Store the search target
                }
                token = strtok(NULL, " ");
            }

            // Parse the optional directory
            token = strtok(NULL, " ");
            if (token != NULL) {
                start_directory = strdup(token);  // Store the directory to search
            }

            // Handle invalid flags case
            if (d && f) {
                printf("Invalid flags\n");
                free(search_file);
                free(start_directory);
                return;
            }

            // Call the seek function
            seek(search_file, d, e, f, start_directory);

            // Free allocated memory
            free(search_file);
            free(start_directory);
            // (existing code)
        } else if (strcmp(token, "activities") == 0) {
            // Call the activities function to display processes
            display_activities();
        } else if (strcmp(token, "neonate") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL || strcmp(token, "-n") != 0) {
                printf("Usage: neonate -n <time_interval>\n");
            } else {
                token = strtok(NULL, " ");
                if (token != NULL) {
                    int time_arg = atoi(token);
                    if (time_arg > 0) {
                        start_neonate(time_arg);  // Call the function from neonate.c
                    } else {
                        printf("Invalid time interval\n");
                    }
                } else {
                    printf("Usage: neonate -n <time_interval>\n");
                }
            }
        } else if(strcmp(token,"fg")==0 || strcmp(token,"bg")==0){
            char* temp=(char*)malloc(PATH_MAX);
            strcpy(temp,token);
            log[*commands % LOG_SIZE] = original_input;  // Store the full command in the log
            *commands += 1;  // Increment command count

        // Handle fg and bg commands
            token = strtok(NULL, " ");
            if (token == NULL) {
                printf("Usage: %s <pid>\n", strcmp(token, "fg") == 0 ? "fg" : "bg");
            } else {
                int pid = atoi(token);
                if (strcmp(temp, "fg") == 0) {
                    fg(pid);
                } else if (strcmp(temp, "bg") == 0) {
                    bg(pid);
                }
            }

        } else if (strcmp(token, "ping") == 0) {
        // Handle the ping command to send signals to processes
            token = strtok(NULL, " ");
            if (token == NULL) {
                printf("Invalid command format. Usage: ping <pid> <signal_number>\n");
                free(original_input);
                return;
            }

            // Extract the PID
            pid_t pid = atoi(token);

            // Extract the signal number
            token = strtok(NULL, " ");
            if (token == NULL) {
                printf("Invalid command format. Usage: ping <pid> <signal_number>\n");
                free(original_input);
                return;
            }

            int signal_number = atoi(token);

            // Call the ping function to send the signal
            ping(pid, signal_number);

            // Log the ping command
            log[*commands % LOG_SIZE] = original_input;  // Store the full command in the log
            *commands += 1;  // Increment command count
            return;
        }else if(strcmp(token, "exit") == 0) {
            exit(0);
        }
        
    }   
    // Handle system commands (foreground or background)
    else {
        int background = 0;

        // Check if the input ends with '&' (background process)
        int len = strlen(original_input);
        if (original_input[len - 1] == '&') {
            background = 1;
            original_input[len - 1] = '\0';  // Remove '&' from the input
        }

        log[*commands % LOG_SIZE] = original_input;  // Store the full command in the log
        *commands += 1;  // Increment command count

        // Execute the system command
        execute_system_command(original_input, background);
    }

    // Free the original_input if it was not stored in the log (handled above)
}


int main() {
    // Install the SIGCHLD handler for background processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;  // Restart interrupted system calls, don't signal for stopped children
    sigaction(SIGCHLD, &sa, NULL);

    // Main loop
    char *blue = "\033[36m";
    char *reset = "\033[0m";
    char *str;
    int commands = 0;  // Command counter
    char* log[LOG_SIZE];  // Log array with a fixed size of 15 for circular buffer

    str = getlogin(); 
    printf("%s%s%s", blue, str, reset);
    printf("@");

    char hostname[1024];  // Buffer to store the hostname
    gethostname(hostname, sizeof(hostname)); 
    
    char home_dir[PATH_MAX];
    char* olddir = (char*)malloc(PATH_MAX * sizeof(char));  // Allocate memory for olddir

    // Execute the hop command to initialize the home directory
    home();

    // Input buffer
    char input[4096];  // Larger input buffer to handle long commands

    while (1) {
        print_details();
        
        // Get the input from the user and remove the newline
        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("fgets error");
            break;
        }
        
        // Remove trailing newline character from the input
        input[strcspn(input, "\n")] = 0;

        // Call the execute function to process the input and update the log
        execute(input, log, &commands, olddir);
    }

    // Free allocated memory (if program ever terminates)
    free(olddir);
}