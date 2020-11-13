#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include "char_vector.h"
#include "net_structs.h"
#include "commands.h"

const int PATH_SIZE = 4096;

void send_end(int fd) {
    struct Payload payload;
    init_pay(&payload);
    payload.response = 2;
    write(fd, (char *)&payload, sizeof(struct Payload));
}

int get_files(struct Payload *request, int fd) {
    /*for (int i = 1; i < args->used; i++) {*/
        /*char path_name[PATH_SIZE];*/
        /*getcwd(path_name, PATH_SIZE);*/

        /*DIR *dir = opendir(path_name);*/
        /*if (dir == NULL) {*/
            /*perror("dir");*/
            /*return 1;*/
        /*}*/

        /*[> Do not free curr_file, may be staticaly allocated <]*/
        /*struct dirent *curr_file;*/
        /*while (curr_file = readdir(dir)) {*/
            /*[> Dont show hidden files <]*/
            /*if (curr_file->d_name[0] == '.') {*/
                /*continue;*/
            /*}*/

            /*if (!strcmp(args->vector[i], curr_file->d_name)) {*/
                /*fprintf(stderr, "File found\n");*/

                /*struct stat statbuf;*/
                /*stat(curr_file->d_name, &statbuf);*/

                /*dprintf(fd, "%ld\n", statbuf.st_size);*/
                /*fprintf(stderr, "Bytes printed\n");*/
                
                /*int expected = strlen(get_accept_message);*/
                /*char buff[expected + 1];*/
                /*int n_read = read(fd, buff, expected);*/
                /*buff[n_read] = '\0';*/

                /*if (!strcmp(buff, get_accept_message)) {*/
                    /*printf("%s\n", buff);*/
                /*} else {*/
                    /*printf("Get Accept Error Message\n");*/
                /*}*/
            /*}*/
        /*}*/
    /*}*/
    return 1;
}

int list_files(struct Payload *request, int fd) {
    char path_name[PATH_SIZE];
    getcwd(path_name, PATH_SIZE);

    DIR *dir = opendir(path_name);
    if (dir == NULL) {
        perror("dir");
        return 1;
    }

    /* Do not free curr_file, may be staticaly allocated */
    struct dirent *curr_file;
    while ((curr_file = readdir(dir))) {
        /* Dont show hidden files */
        if (curr_file->d_name[0] == '.') {
            continue;
        }

        struct Payload payload;
        init_pay(&payload);
        payload.response  = 1;
        payload.length = strlen(curr_file->d_name);
        strcpy(payload.data, curr_file->d_name);

        write(fd, (char *)&payload, sizeof(struct Payload));
    }
    send_end(fd);
    return 0;
}

int change_dir(struct Payload *request, int fd) {
    CVector *args = to_args(request->data);
    if (args->used != 2) {
        /* Add error message */
        return 1;
    }

    send_end(fd);
    int return_code = chdir(args->vector[1]);
    if (return_code == -1) {
        perror("Chdir");
        return 1;
    }

    freeCVector(args);
    return 0;
}

int Commands[] = {
    1,
    2,
    3
};

int (*Commandfun[])(struct Payload*, int) = {
    list_files,
    change_dir,
    get_files
};

int execute(struct Payload *request, int fd) {
    int c_size = sizeof(Commands) / sizeof(int);
    int status = -1;
    if (request->call < c_size) {
        status = (*Commandfun[request->call - 1])(request, fd);
    }

    return status;
}
