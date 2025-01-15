#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include "signal.h"

// Function to send a signal to a process
void ping(pid_t pid, int signal_number) {
    // Take signal_number modulo 32
    signal_number %= 32;

    // Try to send the signal to the process
    if (kill(pid, signal_number) == 0) {
        // Success, signal sent
        printf("Sent signal %d to process with pid %d\n", signal_number, pid);
    } else {
        // If the process doesn't exist or other error
        if (errno == ESRCH) {
            printf("No such process found\n");
        } else {
            perror("Failed to send signal");
        }
    }
}
