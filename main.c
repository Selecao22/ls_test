#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <time.h>

#define MODE_STRING_SIZE 11
#define MOD_TIME_STRING_SIZE 30

char* get_attrs_string(mode_t mode)
{
    char *string = calloc(MODE_STRING_SIZE, sizeof(char ));

    if (string == NULL) {
        return NULL;
    }

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

    return string;

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
        strftime(buf, buf_count, "%b %d %H:%M", gmtime(&ts.tv_sec));
    } else {
        strftime(buf, buf_count, "%b %d %Y", gmtime(&ts.tv_sec));
    }
}


void print_file_info(const struct stat* file_info, const char* file_name, const char* path)
{
    char* mode;
    char* user;
    char* group;
    char* mod_time;
    char link_path[100] = {0};
    char file_path[200] = {0};
    mod_time = calloc(MOD_TIME_STRING_SIZE, sizeof(char ));

    mode = get_attrs_string(file_info->st_mode);
    user = get_user_by_uid(file_info->st_uid);
    group = get_group_by_gid(file_info->st_gid);
    get_mod_time_string(file_info->st_mtim, mod_time, MOD_TIME_STRING_SIZE);
    if (S_ISLNK(file_info->st_mode) && path != NULL){
        if(readlink(path, link_path, 100) != -1){
            sprintf(file_path, "%s -> %s", file_name, link_path);
        }
    }
    printf("%s %lu %s %s %ld %s %s \n",
           mode,
           file_info->st_nlink,
           user,
           group,
           file_info->st_size,
           mod_time,
           S_ISLNK(file_info->st_mode) ? file_path : file_name);

    free(mode);
    free(mod_time);
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


void print_directory_info(const char* dir_path)
{
    DIR* dir;
    struct dirent* ent;
    struct stat file_info;
    char* full_path;
    int status;

    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("");
        return;
    }
    while ((ent = readdir(dir)) != NULL){
        full_path = calloc(strlen(dir_path) + strlen(ent->d_name) + 1, sizeof(char));
        sprintf(full_path, "%s/%s", dir_path, ent->d_name);
        status = lstat(full_path, &file_info);
        if (status != 0){
            perror("");
            continue;
        }
        if (is_file_hidden(ent->d_name)){
            continue;
        }
        print_file_info(&file_info, ent->d_name, full_path);
    }
    closedir(dir);
}


void parse_arguments(int argc, char** argv, char*** parsed_args, int* args_count)
{
    char** args_buffer;
    char* buffer;

    *args_count = argc == 1 ? 1 : argc - 1;
    args_buffer = calloc(*args_count, sizeof(char*));
    if (argc == 1){
        buffer = calloc(100, sizeof(char));
        args_buffer[0] = getcwd(buffer, 100);
    } else {
        for (int i = 0; i < *args_count; ++i) {
            args_buffer[i] = argv[i + 1];
        }
    }

    *parsed_args = args_buffer;
}


int main(int argc, char** argv) {
    char** files;
    struct stat file_info;
    int status;
    int args_count;

    parse_arguments(argc, argv, &files, &args_count);

    for (int i = 0; i < args_count; ++i) {
        status = lstat(files[i], &file_info);
        if (status != 0){
            perror("");
            continue;
        }

        if ((S_ISDIR(file_info.st_mode)) != 0){
            print_directory_info(files[i]);
        }
        else {
            print_file_info(&file_info, files[i], NULL);
        }
    }

    if (argc == 1) {
        free(files[0]);
    }
    free(files);

    return 0;
}
