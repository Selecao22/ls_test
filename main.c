#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>

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


int main(int argc, char** argv) {
    char** files;
    int status;

    if (argc == 1) {
        printf("usage: %s PATH1...PATHn\n", argv[0]);
        return 0;
    }
    else {
        files = argv + 1;
    }

    for (int i = 0; i < argc - 1; ++i) {
        struct stat file_info;
        status = stat(files[i], &file_info);
        if (status != 0){
            perror("");
            continue;
        }

        print_file_info(&file_info, files[i]);

        if (S_ISDIR(file_info.st_mode)){
            DIR *dir = opendir(files[i]);
            if (dir == NULL) {
                perror("");
                continue;
            }
            closedir(dir);
        }

    }

    return 0;
}
