#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include "char_vector.h"
#include "net_structs.h"
#include "commands.h"

const int PATH_SIZE = 4096;

void send_response(int fd, uint16_t response, uint16_t call,
                   uint16_t length, uint64_t left, char *data) {
    struct Payload payload;
    init_pay(&payload);

    if (response != -1) {
        payload.response = response;
    }

    if (call != -1) {
        payload.call = call;
    }

    if (length != -1) {
        payload.length = length;
    }

    if (left != -1) {
        payload.left = left;
    }

    if (data != NULL) {
        for (int i = 0; i < length; i++) {
            payload.data[i] = data[i];
        }
    }

    /*fprintf(stderr, "%d\n%d\n%d\n%ld\n%s\n\n\n", payload.response,*/
            /*payload.call, payload.length, payload.left, payload.data);*/
    write(fd, (char *)&payload, sizeof(struct Payload));
}

int get_files(struct Payload *request, int fd) {
    CVector *args = to_args(request->data);

    char path_name[PATH_SIZE];
    getcwd(path_name, PATH_SIZE);


    for (int nfile = 1; nfile < args->used; nfile++) {
        int found = 0;

        fprintf(stderr, "%s\n", args->vector[nfile]);

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

            if (!strcmp(args->vector[nfile], curr_file->d_name)) {
                int dlfd = open(curr_file->d_name, O_RDONLY);
                if (dlfd == -1) {
                    /* Report server error to client */
                    perror("Get");
                    return 1;
                }
                
                found = 1;

                off_t total_bytes = lseek(dlfd, 0, SEEK_END);
                send_response(fd, 0, -1, -1, total_bytes, NULL);

                char response_buf[RESPONSE_SIZE];
                off_t offset = lseek(dlfd, 0, SEEK_SET);
                while(offset < total_bytes) {
                    int nread = read(dlfd, &response_buf, RESPONSE_SIZE);
                    if (!nread) {
                        break;
                    }
                    offset += nread;
                    send_response(fd, 1, -1, nread,
                                  total_bytes - offset, response_buf);
                }
                break;
            }
        }

        if (!found) {
            send_response(fd, 0, -1, -1, 0, NULL);
        }

        send_response(fd, 2, -1, -1, -1, NULL);
    }
    return 0;
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
    send_response(fd, 2, -1, -1, -1, NULL);
    return 0;
}

int change_dir(struct Payload *request, int fd) {
    CVector *args = to_args(request->data);
    if (args->used != 2) {
        /* Add error message */
        return 1;
    }

    send_response(fd, 2, -1, -1, -1, NULL);
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
    if (request->call <= c_size) {
        status = (*Commandfun[request->call - 1])(request, fd);
    }

    return status;
}
