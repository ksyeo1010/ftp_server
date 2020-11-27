#ifndef _SERVERH__
#define _SERVERH__

/* Max number of connections for a socket */
#define NUM_CONNECTIONS 4

/* The size of sockaddr_in */
#define SIN_SIZE sizeof(struct sockaddr_in)

/**
 * @brief Creates a socket. Does the following functions:
 *        socket(), bind(), listen().
 * 
 * @param {port} The port to initialize the socket.
 * @returns The socket file descriptor.
 */
int create_socket(int port);

#endif