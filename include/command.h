#ifndef _COMMANDH__
#define _COMMANDH__

/* Connection states */
#define ERROR               (-1)
#define FAILED              0
#define SUCCESS             1
#define CLOSING             2

/* Connection auth values */
#define AUTHENTICATED       1
#define NOT_AUTHENTICATED   0

/* Representation states */
#define NONE                'N'
#define ASCII               'A'
#define IMAGE               'I'
#define NON_PRINT           'N'
#define TELNET              'T'
#define ASA                 'C'

/* Modes */
#define STREAM              'S'

/* Structures */
#define FILE                'F'

/* Buffer size */
#define BUFFER_SIZE         1024

/* Delimeter to split message */
#define DELIM               " \t\r\n"

/* COMMAND TYPES */
#define USER                "USER"
#define QUIT                "QUIT"
#define CWD                 "CWD"
#define CDUP                "CDUP"
#define TYPE                "TYPE"
#define MODE                "MODE"
#define STRU                "STRU"
#define RETR                "RETR"
#define PASV                "PASV"
#define NLST                "NLST"

/* REPLY CODES */
#define RC150               "150 File status okay; about to open data connection.\r\n"
#define RC200               "200 Command okay.\r\n"
#define RC220               "220 Welcome.\r\n"
#define RC221               "221 Service closing control connection.\r\n"
#define RC226               "226 Closing data connection. Requested file action successful.\r\n"
#define RC227               "227 Entering Passive Mode (%i,%i,%i,%i,%i,%i).\r\n"
#define RC230               "230 User logged in, proceed.\r\n"
#define RC250               "250 Requested file action okay, completed.\r\n"
#define RC332               "332 Need account for login.\r\n"
#define RC421               "421 Service not available, closing control connection.\r\n"
#define RC425               "425 Can't open data connection.\r\n"
#define RC426               "426 Connection closed; transfer aborted.\r\n"
#define RC451               "451 Requested action aborted: local error in processing.\r\n"
#define RC500               "500 Syntax error, command unrecognized.\r\n"
#define RC501               "501 Syntax error in parameters or arguments.\r\n"
#define RC504               "504 Command not implemented for that parameter.\r\n"
#define RC530               "530 Not logged in.\r\n"
#define RC550               "550 Requested action not taken.\r\n"

/* strlen macro */
#define str_len(s) (s==NULL?0:strlen(s))

struct rep_type {
    char type;
    char code;
};

typedef struct connection_state {
    int auth;
    int state;
    struct rep_type rep;
    char mode;
    int clientd;
    int pasv_socketd;
    int pasv_clientd;
    int s_length;
    char s_buffer[1024];
    void *pthread;
} cs_t;

typedef struct pasv_arg {
    int socketd;
    cs_t *conn;
} pasv_arg_t;

extern char root[];

extern void user(cs_t *conn);

extern void quit(cs_t *conn);

extern void cwd(cs_t *conn);

extern void cdup(cs_t *conn);

extern void type(cs_t *conn);

extern void mode(cs_t *conn);

extern void stru(cs_t *conn);

extern void retr(cs_t *conn);

extern void pasv(cs_t *conn);

extern void nlst(cs_t *conn);

#endif
