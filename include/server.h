#ifndef _SERVERH__
#define _SERVERH__


#define NUM_CONNECTIONS 4

#define SIN_SIZE sizeof(struct sockaddr_in)

int create_socket(int port);

#endif