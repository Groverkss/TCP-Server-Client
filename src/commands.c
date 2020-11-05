#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include "char_vector.h"

const int PATH_SIZE = 4096;

int get_files(CVector *args, int fd) {
    return 0;
}

int list_files(CVector *args, int fd) {
    if (args->used != 1) {
        /* Report error */
        return 1;
    }

    char path_name[PATH_SIZE];
    getcwd(path_name, PATH_SIZE);

    DIR *dir = opendir(path_name);
    if (dir == NULL) {
        perror("dir");
        return 1;
    }

    // Do not free curr_file, may be staticaly allocated
    struct dirent *curr_file;
    while (curr_file = readdir(dir)) {
        /* Dont show hidden files */
        if (curr_file->d_name[0] == '.') {
            continue;
        }

        char *file_path =
            malloc(strlen(path_name) + strlen(curr_file->d_name) + 2);

        sprintf(file_path, "%s/%s", path_name, curr_file->d_name);
        dprintf(fd, "%s\n", curr_file->d_name);

        free(file_path);
    }

    return 0;
}

int change_dir(CVector *args, int fg) {
    if (args->used != 2) {
        // Add error message
        return 1;
    }

    int return_code = chdir(args->vector[1]); 
    if (return_code == -1) {
        perror("Chdir");
        return 1;
    }

    return 0;
}

char *Commands[] = {
    "get",
    "ls",
    "cd"
};

int (*Commandfun[])(CVector*, int) = {
    get_files,
    list_files,
    change_dir
};

int execute(CVector *args, int fd) {
    /* Empty check */
    if (args->used == 0) {
        return 1;
    }

    int c_size = sizeof(Commands) / sizeof(char *);

    /* Validity check */
    for (int i = 0; i < c_size; i++) {
        if (!strcmp(Commands[i], args->vector[0])) {
            return (*Commandfun[i])(args, fd);
        }
    }

    fprintf(stderr, "Command %s not found\n", args->vector[0]);
    return 1;
}
