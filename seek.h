#ifndef SEEK_H
#define SEEK_H

void search_directory(const char *dir_path, const char *search_target, bool dir_only, bool file_only, bool execute_flag, int *file_count, int *dir_count, char *matched_file, char *matched_dir);
void print_result(const char *path, bool is_dir);
bool check_permissions(const char *path, bool is_dir);
bool is_matching(const char *name, const char *search_target);
void seek(char *search_target, bool d, bool e, bool f, char *start_directory);
#endif