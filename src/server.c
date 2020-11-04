#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

const int REQUEST_SIZE = 8192;
const int MAX_CONN = 8;

int main(int argc, char *argv[]) {
    struct in_addr server_addr;
    in_port_t server_port;

    /*
     * inet_addr converts from IPv4 numbers-and-dots
     * notation into binary data in network byte ord
     */
    if (argc == 2) {
        server_addr.s_addr = htonl(INADDR_ANY);
        server_port = htons(atoi(argv[1]));
    } else if (argc == 3) {
        server_addr.s_addr = inet_addr(argv[1]);
        server_port = htons(atoi(argv[2]));
    } else {
        fprintf(stderr, "Usage: ./server [IPv4-ADDRESS] PORT\n");
        exit(1);
    }

    /* Create socket for client */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Socket");
        exit(1);
    }

    /* Initialize socket for connection */
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr = server_addr;
    servaddr.sin_port = server_port;

    /* Establist connection with server */
    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        perror("Bind");
        exit(1);
    }

    /* Listen on this port */
    listen(sockfd, MAX_CONN);

    /* Structs to store client information */
    struct sockaddr_in cliaddr;
    socklen_t cli_addr_len;

    /* Infinite loop, keep accepting requests */
    for (;;) {
        /* Accept incoming request */
        int clifd = accept(sockfd, (struct sockaddr *) &cliaddr, &cli_addr_len);

        char buffer[REQUEST_SIZE];
        int n_read;

        /* Read command while client is outputing */
        while(n_read = read(clifd, buffer, REQUEST_SIZE)) {
            if (n_read == -1) {
                break;
            }

            write(STDOUT_FILENO, buffer, n_read);
            buffer[n_read] = '\0';
            
            execute();

            const char *message_end = "END";
            write(clifd, message_end, strlen(message_end));
        }

        fprintf(stderr, "Connection Ended\n");
    }
}
