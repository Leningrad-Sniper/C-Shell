#ifndef PIPE_H
#define PIPE_H

// Function to execute commands with multiple pipes
void handle_redirection(char *command);
int execute_multiple_pipes(char* commands[], int num_commands, char* log[], int* command_count, char* olddir);

#endif
