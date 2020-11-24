#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "dir.h"
#include "usage.h"
#include "ftp.h"

#define NUM_CONNECTIONS 4

typedef struct sockaddr_in sin_t;

int main(int argc, char **argv) {

    int value;
    int socketd;
    sin_t *address;
    in_port_t port;
    
    // Check the command line arguments
    if (argc != 2) {
      usage(argv[0]);
      return -1;
    }

    // set port
    port = (in_port_t) strtoul(argv[1], NULL, 0);
    if (port < 1024 || port > 65535) {
        perror("Incorrect port number specified");
        exit(-1);
    }

    // create socket
    socketd = socket(PF_INET, SOCK_STREAM, 0);
    if (socketd < 0) {
        perror("Failed to create a socket");
        exit(-1);
    }

    // check for reuse
    value = 1;
    if (setsockopt(socketd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof (int)) != 0) {
        perror("Failed to set the socket option");
        exit(-1);
    }

    // bind the socket to port
    address = (sin_t *) calloc(0, sizeof (sin_t));
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    address->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(socketd, (const struct sockaddr *) address, sizeof(sin_t)) != 0) {
        perror("Failed to bind the socket");
        exit(-1);
    }
    
    // set socket to listen to connections
    if (listen(socketd, NUM_CONNECTIONS) != 0) {
        perror("Failed to listen to connections");
        exit(-1);
    }

    // ca length
    int clientd;
    sin_t *client_address = calloc(0, sizeof(sin_t));
    socklen_t *ca_length = (socklen_t *) sizeof(sin_t);

    pthread_t thread;

    while (1) {
        // accept connection
        printf("Waiting on port %d.\n", port);

        clientd = accept(socketd, (struct sockaddr *) client_address, ca_length);

        if (clientd < 0) {
            perror("Failed to accept the client connection");
            continue;
        }

        printf("Accepted the client connection from %s:%d.\n", 
            inet_ntoa(client_address->sin_addr), ntohs(client_address->sin_port));

        // generate thread
        if (pthread_create(&thread, NULL, interact, ca_length) != 0) {
            perror("Failed to create thread");
            continue;
        }

        // wait until thread is finished.
        pthread_join(thread, NULL);
        printf("Interaction thread has finished.\n");
    }

    return 0;
}
