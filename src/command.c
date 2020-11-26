#include <arpa/inet.h>
#include <ifaddrs.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "command.h"
#include "dir.h"
#include "ftp.h"
#include "server.h"

#define USERNAME "cs317"

void *pasv_accept(void *args);

/////////////////////////////////////////////////////////////////////////////////
void user(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);

    // check account empty
    if (tok == NULL) {
        conn->auth = NOT_AUTHENTICATED;
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC332);
        return;
    }
    
    // check correct account
    if (strcmp(tok, USERNAME) == 0) {
        conn->auth = AUTHENTICATED;
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC230);
    } else {
        conn->auth = NOT_AUTHENTICATED;
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC530);
    }
}

/////////////////////////////////////////////////////////////////////////////////
void quit(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);

    conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC221);
    conn->state = CLOSING;
}

/////////////////////////////////////////////////////////////////////////////////
void cwd(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);

    // if param contains ../
    if (strstr(tok, "../") != NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC501);
        return;
    }

    // if starts with ./
    if (strncmp(tok, "./", 2) == 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC501);
        return;
    }

    if (chdir(tok) < 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC550);
    } else {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC250);
    }
}

/////////////////////////////////////////////////////////////////////////////////
void cdup(cs_t *conn) {
    char dir[BUFFER_SIZE];

    // get cwd
    if (getcwd(dir, BUFFER_SIZE) == NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC550);
        return;
    }

    // check if we are root directory
    if (strcmp(dir, root) == 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC550);
        return;
    }

    if (chdir("..") < 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC550);
    } else {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
    }
}

/////////////////////////////////////////////////////////////////////////////////
void type(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);

    // params should be a character
    int length = str_len(tok);
    if (length != 1) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC501);
        return;
    }

    if (*tok == ASCII) {
        // ASCII case
        tok = strtok(NULL, DELIM);
        length = str_len(tok);
        // if length is not 1
        if (length != 1) {
            conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC501);
            return;
        }
        // if not a type we expect
        if ((*tok != NON_PRINT) &&
            (*tok != TELNET   ) &&
            (*tok != ASA      )) {
            conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC501);
            return;
        }
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
        conn->rep.type = ASCII;
        conn->rep.code = *tok;
    } else if (*tok == IMAGE) {
        // IMAGE case
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
        conn->rep.type = IMAGE;
        conn->rep.code = NONE;
    } else {
        // NON implemented
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC504);
    }
    
}

/////////////////////////////////////////////////////////////////////////////////
void mode(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);

    // if we have a non character
    int length = str_len(tok);
    if (length != 1) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC501);
        return;
    }

    if (*tok == STREAM) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
        conn->mode = STREAM;
    } else {
        // not supported
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC504);
    }
}

/////////////////////////////////////////////////////////////////////////////////
void stru(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);

    // if we have a non character
    int length = str_len(tok);
    if (length != 1) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC501);
        return;
    }

    if (*tok == FILE) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
        conn->mode = FILE;
    } else {
        // not supported
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC504);
    }
}

/////////////////////////////////////////////////////////////////////////////////
void retr(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);
}

/////////////////////////////////////////////////////////////////////////////////
void pasv(cs_t *conn) {
    int socketd;
    struct sockaddr_in sin;
    socklen_t sin_len = SIN_SIZE;
    unsigned short port;

    struct ifaddrs *ifas, *ifa;
    struct sockaddr_in *sa;
    struct in_addr ip_addr;
    in_addr_t addr;

    // if our ip addr are null
    if (getifaddrs(&ifas) == -1) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC421);
        conn->state = CLOSING;
        return;
    }

    // filter loopback addresses and those not ipv4
    for (ifa = ifas; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr != NULL) {
            if (ifa->ifa_addr->sa_family == AF_INET) {
                sa = (struct sockaddr_in *) ifa->ifa_addr;
                if (strcmp(inet_ntoa(sa->sin_addr), "127.0.0.1") != 0) {
                    ip_addr = sa->sin_addr;
                    addr = ip_addr.s_addr;
                }
            }
        }
    }

    // free ifas
    freeifaddrs(ifas);

    // close previous socket we are accepting at
    close(conn->pasv_socketd);
    free(conn->pthread);

    // create socket
    socketd = create_socket(0);
    conn->pasv_socketd = socketd;

    // get port
    if (getsockname(socketd, (struct sockaddr *) &sin, &sin_len) < 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC421);
        return;
    }
    port = ntohs(sin.sin_port);

    // generate thread
    pthread_t* thread = malloc(sizeof (pthread_t));
    conn->pthread = (void *) thread;
    if (pthread_create(thread, NULL, pasv_accept, conn) != 0) {
        perror("Failed to create thread");
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC421);
        return;
    }

    printf("Opening pasv connection from %s:%i\n", inet_ntoa(ip_addr), port);
    conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC227, 
        (int) addr         & 0xFF,
        (int) (addr >>  8) & 0xFF,
        (int) (addr >> 16) & 0xFF,
        (int) (addr >> 24) & 0xFF,
        (int) (port >> 8)  & 0xFF,
        (int) port         & 0xFF
    );
}

/////////////////////////////////////////////////////////////////////////////////
void nlst(cs_t *conn) {
    int length;
    char buffer[BUFFER_SIZE];
    char dir[BUFFER_SIZE];

    // check if there was something open
    if (conn->pthread == NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC425);
        return;
    }

    // check if theres something to wait on
    if (pthread_join(*(pthread_t *) conn->pthread, NULL) != 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC425);
        return;
    }

    // get cwd
    if (getcwd(dir, BUFFER_SIZE) == NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC451);
        return;
    }

    // send message to actual client, not pasv
    length = snprintf(buffer, BUFFER_SIZE, RC150);
    if (send(conn->clientd, buffer, length, 0) != length) {
        perror("Failed to send to the socket");
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC451);
        return;
    }

    // print files
    if (listFiles(conn->pasv_clientd, dir) < 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC426);
        return;
    }

    conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC226);

    // close pasv connection
    close(conn->pasv_clientd);

    // accept again on same socket
    if (pthread_create((pthread_t *) conn->pthread, NULL, pasv_accept, conn) != 0) {
        perror("Failed to create thread");
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC451);
    }
}


void *pasv_accept(void *args) {
    cs_t *conn = (cs_t *) args;

    // accept connection
    struct sockaddr_in client_address;
    socklen_t ca_length = SIN_SIZE;

    int clientd = accept(conn->pasv_socketd, (struct sockaddr *) &client_address, &ca_length);
    conn->pasv_clientd = clientd;

    if (clientd < 0) {
        perror("Failed to accept the client connection");
        NULL;
    }

    printf("Accepted the client connection from %s:%d.\n", 
        inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

    return NULL;
}