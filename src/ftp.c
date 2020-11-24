#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "ftp.h"


void *interact(void *args) {
    int clientd;
    char buffer[BUFFER_SIZE];
    ssize_t length;

    // set clientd
    clientd = *(int *) args;

    while (1) {
        // reset buffer
        bzero(buffer, BUFFER_SIZE);

        // receive message
        length = recv(clientd, buffer, BUFFER_SIZE, 0);
        if (length < 0) {
            perror("Failed to read from socket.");
            break;
        }

        if (length == 0) {
            printf("EOF\n");
            break;
        }

        printf("Received message: %s.\n", buffer);

        // send something back
        length = snprintf(buffer, BUFFER_SIZE, "Server: %s\r\n", "Hello");

        if (send(clientd, buffer, length, 0) != length) {
            perror("Failed to send to the socket");
            break;
        }
    }

    // close connection
    close(clientd);

    return NULL;
}