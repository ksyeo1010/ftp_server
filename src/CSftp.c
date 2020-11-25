#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "dir.h"
#include "usage.h"
#include "ftp.h"

#define SIN_SIZE sizeof(struct sockaddr_in)

int main(int argc, char **argv) {

    int value;
    int socketd;
    struct sockaddr_in address;
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
    if (setsockopt(socketd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int)) != 0) {
        perror("Failed to set the socket option");
        exit(-1);
    }

    // bind the socket to port
    bzero(&address, SIN_SIZE);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(socketd, (const struct sockaddr *) &address, SIN_SIZE) != 0) {
        perror("Failed to bind the socket");
        exit(-1);
    }
    
    // set socket to listen to connections
    if (listen(socketd, NUM_CONNECTIONS) != 0) {
        perror("Failed to listen to connections");
        exit(-1);
    }

    while (1) {
        // accept connection
        struct sockaddr_in client_address;
        socklen_t ca_length = SIN_SIZE;

        printf("Waiting on port %d.\n", port);

        int clientd = accept(socketd, (struct sockaddr *) &client_address, &ca_length);

        if (clientd < 0) {
            perror("Failed to accept the client connection");
            continue;
        }

        printf("Accepted the client connection from %s:%d.\n", 
            inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        
        // create threads
        pthread_t thread;    

        // generate thread
        if (pthread_create(&thread, NULL, interact, &clientd) != 0) {
            perror("Failed to create thread");
            continue;
        }

        // wait until thread is finished.
        pthread_join(thread, NULL);
        printf("Interaction thread has finished.\n");
    }

    return 0;
}
