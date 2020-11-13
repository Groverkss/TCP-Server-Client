#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include "char_vector.h"
#include "net_structs.h"

/**
 * Get protocol:
 * Client sends response 0 with list of files
 * Server reads and sends file data 1 by 1
 * For each file:
 *  Server sends file size
 *  Client reads file size
 *  Server sends data
 *  Client reads data
 *  Server sends close segment
 *  Client closes
 */
int get_files(struct Payload* payload, char *buffer, int fd) {
    payload->call = 3;
    payload->response = 0;
    strcpy(payload->data, buffer);
    write(fd, (char *)payload, sizeof(struct Payload));

    CVector *args = to_args(buffer);

    struct Payload response;

    for (int nfile = 1; nfile < args->used; nfile++) {
        /* Create file */
        uint64_t total_bytes = 0;
        int dlfd = open(args->vector[nfile], O_CREAT | O_WRONLY, 00644);
        if (dlfd == -1) {
            perror("Get");
            return 2;
        }

        int done = 0;
        for (;;) {
            ssize_t nwrote;
            read(fd, (char *)&response, sizeof(struct Payload));
            switch (response.response) {
                case 0:
                    /* Initialize percentage counter */
                    total_bytes = response.left;
                    if (total_bytes == 0) {
                        printf("File not found");
                        fflush(stdout);
                    }
                    break;
                case 1:
                    /* Write to file */
                    nwrote = write(dlfd, response.data, response.length);
                    if (nwrote == -1) {
                        perror("Get");
                        return 2;
                    }
                    break;
                case 2:
                    /* End of stream */
                    printf("\nDownload Complete: %s\n", args->vector[nfile]);
                    fflush(stdout);
                    done = 1;
                    break;
                case 3:
                    /* Error detected */
                    return 1;
                    break;
            }

            if (done) {
                break;
            }

            if (total_bytes != 0) {
                /* Print percentage */
                long double curr_percent =
                    (long double) (total_bytes - response.left) / (long double) total_bytes;
                curr_percent *= 100.0;

                fflush(stdout);
                const char *cmplt = "Percentage Completion: ";
                printf("\r%s", cmplt);
                printf("%Lf%%", curr_percent);
                fflush(stdout);
            }
        }
        close(dlfd);
    }
    return 0;
}

int list_files(struct Payload* payload, char *buffer, int fd) {
    payload->call = 1;
    payload->response = 0;
    write(fd, (char *)payload, sizeof(struct Payload));

    struct Payload response;
    for (;;) {
        read(fd, (char *)&response, sizeof(struct Payload));
        if (response.response == 3) {
            /* Error detected */
            return 1;
        } else if (response.response == 2) {
            /* End of stream */
            return 0;
        }
        write(STDOUT_FILENO, response.data, response.length);
        write(STDOUT_FILENO, "\n", 1);
    }
}

int change_dir(struct Payload* payload, char *buffer, int fd) {
    payload->call = 2;
    payload->response = 0;
    strcpy(payload->data, buffer);
    payload->length = strlen(buffer);
    write(fd, (char *)payload, sizeof(struct Payload));

    struct Payload response;
    read(fd, (char *)&response, sizeof(struct Payload));

    if (response.response == 2) {
        /* Success code */
        return 0;
    } else {
        /* Error dected */
        return 1;
    }
}

char *Commands[] = {
    "get",
    "ls",
    "cd"
};

int (*Commandfun[])(struct Payload*, char *buffer, int) = {
    get_files,
    list_files,
    change_dir
};

int main(int argc, char *argv[]) {
    struct in_addr server_addr;
    in_port_t server_port;

    if (argc != 3) {
        fprintf(stderr, "Usage: IPv4-ADDRESS PORT\n");
        exit(1);
    }

    /*
     * inet_addr converts from IPv4 numbers-and-dots
     * notation into binary data in network byte ord
     */
    server_addr.s_addr = inet_addr(argv[1]);
    server_port = htons(atoi(argv[2]));

    /* Create socket for client */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket");
        exit(1);
    }

    /* Initialize socker for connection */
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr = server_addr;
    servaddr.sin_port = server_port;

    /* Establish connection with server */
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        perror("Connect");
        exit(1);
    }

    /* Read lines from stdin until EOF */
    char *buffer = NULL;
    size_t n_buffer = 0;
    int n_read;
    while((n_read = getline(&buffer, &n_buffer, stdin))) {
        if (n_read == -1) {
            break;
        }

        if (n_read == 0) {
            continue;
        }

        /* Convert buffer to arguement list */
        char *buffer_dup = strdup(buffer);
        CVector *args = to_args(buffer_dup);

        /* Prepare request payload */
        struct Payload request;
        init_pay(&request);

        if (args->used == 0) {
            fprintf(stderr, "Invalid Command\n");
            continue;
        }

        int c_size = sizeof(Commands) / sizeof(char *);
        int status = -1;
        for (int i = 0; i < c_size; i++) {
            if (!strcmp(Commands[i], args->vector[0])) {
                status = (*Commandfun[i])(&request, buffer, sockfd);
                break;
            }
        }

        if (status == -1) {
            fprintf(stderr, "Invalid Command\n");
        } else if (status == 1) {
            printf("Server Error\n");
            break;
        } else if (status == 2) {
            printf("Client Error\n");
            break;
        }
    }

    fprintf(stderr, "Connection Ended\n");
}
