#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "server.h"

/////////////////////////////////////////////////////////////////////////////////
int create_socket(int port) {
    int value;
    int socketd;
    struct sockaddr_in address;

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

    return socketd;
}