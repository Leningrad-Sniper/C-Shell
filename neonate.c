#include "neonate.h"
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include<sys/types.h>
#include<fcntl.h>

// Function to get the most recently created process PID
pid_t get_latest_pid() {
    DIR *dir;
    struct dirent *entry;
    pid_t latest_pid = -1;

    dir = opendir("/proc");
    if (!dir) {
        perror("Unable to open /proc directory");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Check if the directory name is entirely numeric, which corresponds to a PID
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            pid_t current_pid = atoi(entry->d_name);
            if (current_pid > latest_pid) {
                latest_pid = current_pid;
            }
        }
    }

    closedir(dir);
    return latest_pid;
}

// Function to detect key press
int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

// Function to monitor and print most recent PID every time_arg seconds
void start_neonate(int time_arg) {
    printf("Monitoring newest processes every %d seconds. Press 'x' to stop.\n", time_arg);

    while (1) {
        pid_t latest_pid = get_latest_pid();
        if (latest_pid != -1) {
            printf("%d\n", latest_pid);
        }

        sleep(time_arg);

        // Check if 'x' is pressed
        if (kbhit() && getchar() == 'x') {
            printf("Exiting...\n");
            break;
        }
    }
}
