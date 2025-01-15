#ifndef PROCLORE_H
#define PROCLORE_H

const char* get_status_string(char state);
int is_foreground(int pid);
void proclore(int pid);
void proclore_command(char *pid_str);


#endif

