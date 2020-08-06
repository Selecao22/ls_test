//
// Created by nikky on 05.08.2020.
//

#ifndef MY_LS_FILE_INFO_H
#define MY_LS_FILE_INFO_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <time.h>

struct file_data {
    char *name;
    struct stat info;
    char* user_string;
    char* group_string;
    char* mod_time_str;
    char* mode_string;
};


struct align_parameters{
    int hard_links_align;
    int user_align;
    int group_align;
    int byte_size_align;
    int mod_time_size_align;
};


int is_file_hidden(char* file_name);


char* get_symlink_path(char* link_path);


char* get_file_name(char* full_path);


int sort_files(const void* file1, const void* file2);


struct file_data get_file_info(struct stat file_stat, const char* file_name);


void print_file_info(struct file_data file, struct align_parameters align);

#endif //MY_LS_FILE_INFO_H
