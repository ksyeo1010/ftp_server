#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ftp.h"
#include "command.h"

/* Helper function declarations */

void handleMessage(char *buffer, cs_t *conn);
void stringUpr(char *s);

/////////////////////////////////////////////////////////////////////////////////
void *interact(void *args) {
    // set clientd
    int clientd = *(int *) args;

    /* Set some initial states of our ftp server */

    // the receive buffer
    char r_buffer[BUFFER_SIZE];

    // the connection state
    cs_t conn = {
        NOT_AUTHENTICATED,      // auth
        SUCCESS,                // state
        {ASCII, NON_PRINT},     // type
        STREAM,                 // mode
        clientd,                // clientd
        -1,                     // pasv_socketd
        -1,                     // pasv_clientd
        0,                      // s_buf length
        '\0',                   // send buffer
        '\0',                   // set current directory to root
        0                       // pthread
    };

    // set root dir
    strcpy(conn.dir, root);

    // initial message
    conn.s_length = snprintf(conn.s_buffer, BUFFER_SIZE, RC220);
    if (send(clientd, conn.s_buffer, conn.s_length, 0) != conn.s_length) {
        perror("Failed to send to the socket");
    } else {
        while (1) {
            // reset buffer
            bzero(r_buffer, BUFFER_SIZE);

            // receive message
            ssize_t length = recv(clientd, r_buffer, BUFFER_SIZE, 0);

            if (length < 0) {
                perror("Failed to read from socket");
                break;
            }

            if (length == 0) {
                printf("EOF\n");
                break;
            }

            // handle message
            handleMessage(r_buffer, &conn);

            // send back
            if (send(clientd, conn.s_buffer, conn.s_length, 0) != conn.s_length) {
                perror("Failed to send to the socket");
                break;
            }

            if (conn.state == CLOSING) break;
        }
    }

    // close connection
    close(clientd);

    return NULL;
}

/**
 * @brief Helper to detect which command to run.
 * 
 * @param {buffer} The message buffer.
 * @param {conn} The connection state pointer.
 */
void handleMessage(char *buffer, cs_t *conn) {
    char dup[BUFFER_SIZE];
    strcpy(dup, buffer);

    char *tok = strtok(dup, DELIM);
    stringUpr(tok); 

    if (strcmp(tok, USER) == 0) {
        user(conn);
    } else if (strcmp(tok, QUIT) == 0) {
        quit(conn);
    } else {
        // if not authenticated
        if (!conn->auth) {
            conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC530);
        } else {
            if (strcmp(tok,  CWD) == 0) {
                cwd(conn);
            } else if (strcmp(tok, CDUP) == 0) {
                cdup(conn);                
            } else if (strcmp(tok, TYPE) == 0) {
                type(conn);  
            } else if (strcmp(tok, MODE) == 0) {
                mode(conn);  
            } else if (strcmp(tok, STRU) == 0) {
                stru(conn);  
            } else if (strcmp(tok, RETR) == 0) {
                retr(conn);  
            } else if (strcmp(tok, PASV) == 0) {
                pasv(conn);  
            } else if (strcmp(tok, NLST) == 0) {
                nlst(conn);  
            } else {
                // command not defined
                conn->s_length = snprintf(conn->s_buffer, BUFFER_SIZE, RC500_S);
            }
        }
    }
}

/**
 * @brief Helper to change a string to upper case.
 * 
 * @param {s} Pointer to the start of the string.
 */
void stringUpr(char *s) {
    int i = 0;
    while (s[i] != '\0') {
        s[i] = toupper((unsigned char) s[i]);
        i++;
    }
}