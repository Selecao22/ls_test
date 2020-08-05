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

static const char CURRENT_DIRECTORY[] = ".";

struct file_data {
    char *name;
    struct stat info;
};

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

    if ((mode & S_ISVTX) != 0){
        string[9] = 't';
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


void print_file_info(struct stat* file_info, char* file_name)
{
    char* mode;
    char* user;
    char* group;
    char* mod_time;
    mod_time = calloc(MOD_TIME_STRING_SIZE, sizeof(char ));

    mode = get_attrs_string(file_info->st_mode);
    user = get_user_by_uid(file_info->st_uid);
    group = get_group_by_gid(file_info->st_gid);
    get_mod_time_string(file_info->st_mtim, mod_time, MOD_TIME_STRING_SIZE);
    printf("%s %lu %s %s %ld %s %s",
           mode,
           file_info->st_nlink,
           user,
           group,
           file_info->st_size,
           mod_time,
           file_name);

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


void print_directory_files(const char* dir_path)
{
    DIR* dir;
    struct dirent* ent;
    struct stat file_info;
    struct file_data* dir_files = NULL;
    char* full_path;
    int files_count = 0;
    int status;
    char* link_path;

    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("");
        return;
    }

    while ((ent = readdir(dir)) != NULL){
        if (is_file_hidden(ent->d_name)){
            continue;
        }
        full_path = calloc(strlen(dir_path) + strlen(ent->d_name) + 2, sizeof(char));
        sprintf(full_path, "%s/%s", dir_path, ent->d_name);
        status = lstat(full_path, &file_info);
        if (status != 0){
            perror("");
            continue;
        }

        files_count++;
        dir_files = realloc(dir_files, sizeof(*dir_files) * files_count);
        dir_files[files_count - 1].name = full_path;
        dir_files[files_count - 1].info = file_info;
    }

    qsort(dir_files, files_count, sizeof(*dir_files), sort_files);

    for (int i = 0; i < files_count; ++i) {
        print_file_info(&dir_files[i].info, get_file_name(dir_files[i].name));
        if (S_ISLNK(dir_files[i].info.st_mode)){
            link_path = get_symlink_path(dir_files[i].name);
            if (link_path != NULL){
                printf(" -> %s\n", link_path);
                free(link_path);
            }
        } else {
            printf("\n");
        }
        if (dir_files[i].name != NULL){
            free(dir_files[i].name);
        }
    }

    free(dir_files);
    closedir(dir);
}


void parse_arguments(int argc, char** argv, struct file_data** parsed_args, int* files_count)
{
    struct file_data* args_buffer;
    int status;
    int args_count = 0;

    *files_count = argc == 1 ? 1 : argc - 1;
    args_buffer = calloc(*files_count, sizeof(struct file_data));
    if (args_buffer == NULL){
        *files_count = 0;
        *parsed_args = NULL;
        return;
    }

    if (argc == 1){
        status = lstat(".", &args_buffer[0].info);
        if (status != 0) {
            perror("");
            args_count = 0;
            free(args_buffer);
            args_buffer = NULL;
            return;
        }
        args_buffer[0].name = (char*)CURRENT_DIRECTORY;
        args_count++;
    } else {
        for (int i = 0; i < *files_count; ++i) {
            status = lstat(argv[i + 1], &args_buffer[args_count].info);
            if (status != 0){
                perror("");
                continue;
            }
            args_buffer[i].name = argv[i + 1];
            args_count++;
        }
    }

    qsort(args_buffer, args_count, sizeof(*args_buffer), sort_files);

    *parsed_args = args_buffer;
    *files_count = args_count;
}


int main(int argc, char** argv) {
    struct file_data* files;
    int args_count;

    parse_arguments(argc, argv, &files, &args_count);

    if (args_count == 0){
        return 1;
    }

    if (args_count == 1){
        print_directory_files(files[0].name);
    } else {
        for (int i = 0; i < args_count; ++i) {
            if (S_ISDIR(files[i].info.st_mode)){
                continue;
            }
            print_file_info(&files[i].info, files[i].name);
            printf("\n");
        }

        for (int j = 0; j < args_count; ++j) {
            if (!S_ISDIR(files[j].info.st_mode)){
                continue;
            }
            printf("\n%s:\n", files[j].name);
            print_directory_files(files[j].name);
        }
    }

    free(files);

    return 0;
}
