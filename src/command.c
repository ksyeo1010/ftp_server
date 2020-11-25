#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "command.h"

#define USERNAME "cs317"

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
}

/////////////////////////////////////////////////////////////////////////////////
void stru(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);
}

/////////////////////////////////////////////////////////////////////////////////
void retr(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);
}

/////////////////////////////////////////////////////////////////////////////////
void pasv(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);
}

/////////////////////////////////////////////////////////////////////////////////
void nlst(cs_t *conn) {
    char *tok = strtok(NULL, DELIM);
}