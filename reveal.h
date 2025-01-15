#include<stdbool.h>
#ifndef REVEAL_H
#define REVEAL_H

int compare(const void *a, const void *b);
int is_hidden(const char *filename);
int is_regular_file(const char *path);
void print_file_details(const char *path, const char *filename, int show_hidden);
void print_files(const char *directory, bool show_hidden, bool long_format);
void reveal(char *path, bool show_long, bool show_all, char *olddir);

#endif