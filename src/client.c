#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

const int RESPONSE_SIZE = 8192;

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
    char response[RESPONSE_SIZE];
    size_t n_buffer = 0;
    int n_read;
    while(n_read = getline(&buffer, &n_buffer, stdin)) {
        if (n_read == -1) {
            break;
        }

        write(sockfd, buffer, n_read);

        while(n_read = read(sockfd, response, RESPONSE_SIZE)) {
            if (n_read == -1) {
                perror("Response");
                exit(1);
            }

            if (strcmp(response, "END")) {
                break;
            }

            write(STDOUT_FILENO, response, n_read);
        }
    }

    close(sockfd);
    fprintf(stderr, "Connection Ended\n");
}
