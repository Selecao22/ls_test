//
// Created by nikky on 05.08.2020.
//

#include "file_info.h"

#define MODE_STRING_SIZE 11
#define MOD_TIME_STRING_SIZE 30

void get_attrs_string(mode_t mode, char* res_string)
{
    char *string = res_string;

    memset(string, '-', (MODE_STRING_SIZE - 1) * sizeof(char ));

    switch (mode & S_IFMT) {
        case S_IFSOCK:
            string[0] = 's';
            break;
        case S_IFDIR:
            string[0] = 'd';
            break;
        case S_IFBLK:
            string[0] = 'b';
            break;
        case S_IFCHR:
            string[0] = 'c';
            break;
        case S_IFREG:
            string[0] = '-';
            break;
        case S_IFLNK:
            string[0] = 'l';
            break;
        case S_IFIFO:
            string[0] = 'p';
            break;
    }

    for (unsigned int rights = mode & (S_IRWXU | S_IRWXG | S_IRWXO), i = 0; i < 3; i++) {
        if ((rights & 0400 >> i * 3) != 0){
            string[1 + i * 3] = 'r';
        }

        if ((rights & 0200 >> i * 3) != 0){
            string[2 + i * 3] = 'w';
        }

        if ((rights & 0100 >> i * 3) != 0){
            string[3 + i * 3] = 'x';
        }
    }

    if ((mode & S_ISUID) != 0){
        string[3] = 's';
    }

    if ((mode & S_ISGID) != 0){
        string[6] = 's';
    }

    if ((mode & S_ISVTX) != 0){
        string[9] = 't';
    }

}


char* get_user_by_uid(uid_t uid)
{
    struct passwd *pd;
    pd = getpwuid(uid);
    if (pd == NULL){
        return NULL;
    }

    return pd->pw_name;
}


char* get_group_by_gid(gid_t gid)
{
    struct group *gr;
    gr = getgrgid(gid);
    if (gr == NULL){
        return NULL;
    }
    return gr->gr_name;
}


void get_mod_time_string(struct timespec ts, char* buf, int buf_count)
{

    char current_year[5];
    struct timespec real_time;

    if (buf == NULL || buf_count == 0){
        return;
    }

    clock_gettime(CLOCK_REALTIME, &real_time);
    strftime(current_year, 5, "%Y", gmtime(&real_time.tv_sec));
    strftime(buf, buf_count, "%Y", gmtime(&ts.tv_sec));
    if (!strcmp(current_year, buf)) {
        strftime(buf, buf_count, "%b %2e %H:%M", gmtime(&ts.tv_sec));
    } else {
        strftime(buf, buf_count, "%b %2e %5Y", gmtime(&ts.tv_sec));
    }
}


int is_file_hidden(char* file_name)
{
    char* symbol_entry;

    symbol_entry = strchr(file_name, '.');
    if (symbol_entry == NULL || symbol_entry != file_name){
        return 0;
    }

    return 1;
}


char* get_symlink_path(char* link_path)
{
    char* symlink_path;

    symlink_path = calloc(100, sizeof(char));
    if (symlink_path == NULL){
        return NULL;
    }

    if(readlink(link_path, symlink_path, 100) != -1) {
        return symlink_path;
    }

    return NULL;
}


char* get_file_name(char* full_path){
    char* ptr;
    char* slash_ptr = full_path;

    ptr = full_path;

    for (int i = 0; ptr[i] != 0; ++i) {
        if (ptr[i] == '/' && ptr[i + 1] != 0){
            slash_ptr = ptr + i + 1;
        }
    }

    return slash_ptr;
}


int sort_files(const void* file1, const void* file2){
    struct file_data *f1 = (struct file_data*)file1;
    struct file_data *f2 = (struct file_data*)file2;

    return strcmp(f1->name, f2->name);
}


struct file_data get_file_info(struct stat file_stat, const char* file_name){
    char* mode;
    char* user;
    char* group;
    char* mod_time;
    struct file_data file_info;

    mode = calloc(MODE_STRING_SIZE, sizeof(char ));
    mod_time = calloc(MOD_TIME_STRING_SIZE, sizeof(char ));

    get_attrs_string(file_stat.st_mode, mode);
    user = get_user_by_uid(file_stat.st_uid);
    group = get_group_by_gid(file_stat.st_gid);
    get_mod_time_string(file_stat.st_mtim, mod_time, MOD_TIME_STRING_SIZE);

    file_info.info = file_stat;
    file_info.name = (char*)file_name;
    file_info.user_string = user;
    file_info.group_string = group;
    file_info.mod_time_str = mod_time;
    file_info.mode_string = mode;

    return file_info;
}


void print_file_info(struct file_data file, struct align_parameters align)
{
    printf("%s %*d %*s %*s %*d %*s %s",
           file.mode_string,
           align.hard_links_align,
           file.info.st_nlink,
           align.user_align,
           file.user_string,
           align.group_align,
           file.group_string,
           align.byte_size_align,
           file.info.st_size,
           align.mod_time_size_align,
           file.mod_time_str,
           file.name);
}