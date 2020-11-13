#include <stdlib.h>
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

int get_files(struct Payload* payload, char *buffer, int fd) {
    return 1;
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
        exit(1); }

    /* Read lines from stdin until EOF */
    char *buffer = NULL;
    size_t n_buffer = 0;
    int n_read;
    while((n_read = getline(&buffer, &n_buffer, stdin))) {
        if (n_read == -1) {
            break;
        }

        /* Convert buffer to arguement list */
        char *buffer_dup = strdup(buffer);
        CVector *args = to_args(buffer_dup);

        /* Prepare request payload */
        struct Payload request;
        init_pay(&request);

        if (args->used == 0) {
            fprintf(stderr, "Invalid Command\n");
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
            printf("Invalid Command\n");
        } else if (status == 1) {
            printf("Server Error\n");
            break;
        }
    }

    close(sockfd);
    fprintf(stderr, "Connection Ended\n");
}
