#include <arpa/inet.h>
#include <ifaddrs.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "command.h"
#include "dir.h"
#include "ftp.h"
#include "server.h"

#define USERNAME "cs317"    /* username to log in */
#define TIMEOUT  60         /* Timeout in seconds */

/* helper function declaration */

void close_pasv(cs_t *conn);
void *pasv_accept(void *args);
void *timeout(void *args);

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
    // actual freeing done by ftp.c
    conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC221);
    conn->state = CLOSING;
}

/////////////////////////////////////////////////////////////////////////////////
void cwd(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);

    // if we don't have params
    if (tok == NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC500, "Invalid CWD command.");
        return;
    }

    // if param contains ./
    if (strstr(tok, "./") != NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC550_S);
        return;
    }

    // check if the dir we trying to get exists 
    struct stat st;
    char dir[BUFFER_SIZE];
    bzero(dir, BUFFER_SIZE);

    // append to data
    strcpy(dir, conn->dir);
    strcat(dir, "/");
    strcat(dir, tok);

    // check if dir exists
    if (stat(dir, &st) < 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC500, "Invalid directory name.");
    } else {
        if (S_ISDIR(st.st_mode)) {
            // if exists
            conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC250);
            strcpy(conn->dir, dir);
        } else {
            conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC500, "Not a direcotry.");
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////
void cdup(cs_t *conn) {
    // check if we are root directory
    if (strcmp(conn->dir, root) == 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC550_S);
        return;
    }

    // get last occurrence with "/"
    char *str = strrchr(conn->dir, '/');

    // set everything to null
    bzero(str, (int) sizeof(str));
    conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
}

/////////////////////////////////////////////////////////////////////////////////
void type(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);

    // params should be a character
    int length = str_len(tok);
    if (length != 1) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
        conn->type = ASCII;
        return;
    }

    // if we have more params
    if (strtok(NULL, DELIM) != NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC500, "Unrecognized TYPE command.");
        return;
    }

    if (*tok == ASCII) {
        // ASCII case
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
        conn->type = ASCII;
    } else if (*tok == IMAGE) {
        // IMAGE case
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
        conn->type = IMAGE;
    } else {
        // NON implemented
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC500, "Unrecognized TYPE command.");
    }
    
}

/////////////////////////////////////////////////////////////////////////////////
void mode(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);

    // if we have a non character
    int length = str_len(tok);
    if (length != 1) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
        conn->mode = STREAM;
        return;
    }

    // if we have more params
    if (strtok(NULL, DELIM) != NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC500, "Unrecognized MODE command.");
        return;
    }

    if (*tok == STREAM) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
        conn->mode = STREAM;
    } else {
        // not supported
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC500, "Unrecognized MODE command.");
    }
}

/////////////////////////////////////////////////////////////////////////////////
void stru(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);

    // if we have a non character
    int length = str_len(tok);
    if (length != 1) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
        conn->mode = FILE_STRUC;
        return;
    }

    // if we have more params
    if (strtok(NULL, DELIM) != NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC500, "Unrecognized STRU command.");
        return;
    }

    if (*tok == FILE_STRUC) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC200);
        conn->mode = FILE_STRUC;
    } else {
        // not supported
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC500, "Unrecognized STRU command.");
    }
}

/////////////////////////////////////////////////////////////////////////////////
void retr(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);
    FILE *pFile;
    int length;
    char buffer[BUFFER_SIZE];
    char dir[BUFFER_SIZE];

    // if nothing to retrieve
    if (tok == NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC500, "Invalid RETR command.");
        return;
    }

    // set up file
    strcpy(dir, conn->dir);
    strcat(dir, "/");
    strcat(dir, tok);

    // if nothing to open
    pFile = fopen(dir, "rb");
    if (pFile == NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC550, "File unavailable.");
        return;
    }

    // check if there is pasv open
    if (conn->pthread == NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC425);
        fclose(pFile);
        return;
    }

    // check if theres something to wait on, we wait
    if (pthread_join(*(pthread_t *) conn->pthread, NULL) != 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC425);
        fclose(pFile);
        return;
    }

    // send message to actual client, not pasv
    length = snprintf(buffer, BUFFER_SIZE, RC150);
    if (send(conn->clientd, buffer, length, 0) != length) {
        perror("Failed to send to the socket");
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC451);
        fclose(pFile);
        return;
    }

    // check if we are active
    if (send(conn->pasv_clientd, "", 0, 0) < 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC426);
        fclose(pFile);
        close_pasv(conn);
        return;
    }

    // read file and send data
    char buf[BUFFER_SIZE];
    size_t bytes;
    while (feof(pFile) == 0) {
        bytes = fread(buf, sizeof(*buf), sizeof(buf), pFile);
        if (send(conn->pasv_clientd, buf, bytes, 0) != bytes) {
            conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC426);
            close_pasv(conn);
            fclose(pFile);
            return;
        }
    }
    
    // close file
    fclose(pFile);

    // if everything was done
    conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC226);

    // close pasv connection
    close_pasv(conn);
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

    struct sockaddr_in client_addr;

    // get client addr
    if (getsockname(conn->clientd, (struct sockaddr *) &client_addr, &sin_len) < 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC421);
        return;
    }

    // if our ip addr are null
    if (getifaddrs(&ifas) == -1) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC421);
        conn->state = CLOSING;
        freeifaddrs(ifas);
        return;
    }

    // check if we are on localhost
    if (strcmp(inet_ntoa(client_addr.sin_addr), "127.0.0.1") == 0) {
        ip_addr = client_addr.sin_addr;
        addr = client_addr.sin_addr.s_addr;
    } else {
        // filter loopback addresses and those not ipv4
        for (ifa = ifas; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr != NULL) {
                if (ifa->ifa_addr->sa_family == AF_INET) {
                    sa = (struct sockaddr_in *) ifa->ifa_addr;
                    if (strcmp(inet_ntoa(sa->sin_addr), "127.0.0.1") != 0) {
                        ip_addr = sa->sin_addr;
                        addr = ip_addr.s_addr;
                        break;
                    }
                }
            }
        }
    }

    // free ifas
    freeifaddrs(ifas);

    // close previous socket we are accepting at
    close_pasv(conn);

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
        conn->state = CLOSING;
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
    // case with parameters
    char *tok = strtok(NULL, DELIM);
    if (tok != NULL) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC501);
        return;
    }

    int length;
    char buffer[BUFFER_SIZE];

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

    // send message to actual client, not pasv
    length = snprintf(buffer, BUFFER_SIZE, RC150);
    if (send(conn->clientd, buffer, length, 0) != length) {
        perror("Failed to send to the socket");
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC451);
        close_pasv(conn);
        return;
    }

    // check if we are active
    if (send(conn->pasv_clientd, "", 0, 0) < 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC426);
        close_pasv(conn);
        return;
    }

    // print files
    chdir(conn->dir); /*  listFiles does not print bytes correctly if not in the dir*/
    if (listFiles(conn->pasv_clientd, conn->dir) < 0) {
        conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC426);
        close_pasv(conn);
        return;
    }

    conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC226);

    // close pasv connection
    close_pasv(conn);
}

/////////////////////////////////////////////////////////////////////////////////
void close_pasv(cs_t *conn) {
    if (conn->pasv_clientd != -1) close(conn->pasv_clientd);
    if (conn->pasv_socketd != -1) close(conn->pasv_socketd);
    if (conn->pthread != NULL) {
        free(conn->pthread);
        conn->pthread = NULL;
    }
}

/**
 * @brief Function to initialize a pasv thread. Accepts a pasv client.
 * 
 * @param {args} The address of connection state.
 * @returns NULL
 */
void *pasv_accept(void *args) {
    cs_t *conn = (cs_t *) args;
    pthread_t thread;

    // accept connection
    struct sockaddr_in client_address;
    socklen_t ca_length = SIN_SIZE;

    // create timeout thread
    if (pthread_create(&thread, NULL, timeout, conn) != 0) {
        perror("Failed to create thread");
        return NULL;
    }

    int clientd = accept(conn->pasv_socketd, (struct sockaddr *) &client_address, &ca_length);
    conn->pasv_clientd = clientd;

    // cancel timeout thread if we have accepted
    if (pthread_cancel(thread) < 0) {
        perror("Failed to cancel timeout thread");
        return NULL;
    }

    if (clientd < 0) {
        perror("Failed to accept the pasv client connection");
        return NULL;
    }

    printf("Accepted the pasv client connection from %s:%d.\n", 
        inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

    return NULL;
}

/**
 * @brief Sleeps on a thread for a given number of seconds described 
 *        by TIMEOUT. Make sure thread creating this function deletes
 *        if it doesn't want to wait.
 * 
 * @param {args} The address of the connection state.
 * @returns NULL
 */
void *timeout(void *args) {
    cs_t *conn = (cs_t *) args;

    sleep(TIMEOUT);

    printf("Timed out, closing passive connection.\n");
    close_pasv(conn);
    return NULL;
}