#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#define MODE_STRING_COUNT 11

char* get_attrs_string(mode_t mode)
{
    char *string = calloc(MODE_STRING_COUNT, sizeof(char ));
    char file_mode = '-';
    char access_rights[10] = "---------";

    if (string == NULL)
        return NULL;

    switch (mode & S_IFMT) {
        case S_IFSOCK:
            file_mode = 's';
            break;
        case S_IFDIR:
            file_mode = 'd';
            break;
        case S_IFBLK:
            file_mode = 'b';
            break;
        case S_IFCHR:
            file_mode = 'c';
            break;
        case S_IFREG:
            file_mode = '-';
            break;
        case S_IFLNK:
            file_mode = 'l';
            break;
        case S_IFIFO:
            file_mode = 'p';
            break;
    }

    for (unsigned int rights = mode & (S_IRWXU | S_IRWXG | S_IRWXO), i = 0; i < 3; i++) {
        if ((rights & 0400 >> i * 3) != 0)
            access_rights[0 + i * 3] = 'r';

        if ((rights & 0200 >> i * 3) != 0)
            access_rights[1 + i * 3] = 'w';

        if ((rights & 0100 >> i * 3) != 0)
            access_rights[2 + i * 3] = 'x';
    }

    string[0] = file_mode;
    strcat(string, access_rights);

    return string;

}


char* get_user_by_uid(uid_t uid)
{
    struct passwd *pd;

    pd = getpwuid(uid);
    return pd->pw_name;
}


char* get_group_by_gid(gid_t gid)
{
    struct group *gr;
    gr = getgrgid(gid);

    return gr->gr_name;
}


void print_file_info(const struct stat* file_info, const char* file_name)
{
    char* mode;
    char* user;
    char* group;

    mode = get_attrs_string(file_info->st_mode);
    user = get_user_by_uid(file_info->st_uid);
    group = get_group_by_gid(file_info->st_gid);
    printf("%s %lu %s %s %ld %s \n", mode, file_info->st_nlink, user, group, file_info->st_size, file_name);

    free(mode);
}

void print_directory_info(const char* dir_path)
{
    DIR* dir;
    struct dirent* ent;
    struct stat file_info;
    int status;

    dir = opendir(dir_path);
    if (dir == NULL) {
        perror("");
        return;
    }
    while ((ent = readdir(dir)) != NULL){
        status = stat(ent->d_name, &file_info);
        if (status != 0){
            continue;
        }
        print_file_info(&file_info, ent->d_name);
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
        status = stat(files[i], &file_info);
        if (status != 0){
            perror("");
            continue;
        }

        if ((S_ISDIR(file_info.st_mode)) != 0){
            print_directory_info(files[i]);
        }
        else {
            print_file_info(&file_info, files[i]);
        }
    }

    if (argc == 1) {
        free(files[0]);
    }
    free(files);

    return 0;
}
