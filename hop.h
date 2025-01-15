#ifndef HOP_H
#define HOP_H

char* hop(char* olddir,char *token);
void home();
char* parentdir(char* olddir);
char* prevdir(char* olddir);
char* nameddir(char* olddir,char* target);
char* homedir(char* olddir);

#endif 