#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include "usage.h"
#include "server.h"
#include "ftp.h"

int main(int argc, char **argv) {
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

    // create socket on port
    int socketd = create_socket(port);

    printf("Waiting on port %d.\n", port);

    while (1) {
        // accept connection
        struct sockaddr_in client_address;
        socklen_t ca_length = SIN_SIZE;

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
