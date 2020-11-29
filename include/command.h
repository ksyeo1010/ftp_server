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
#define ASCII               'A'
#define IMAGE               'I'

/* Modes */
#define STREAM              'S'

/* Structures */
#define FILE_STRUC          'F'

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
#define RC500_S             "500 Syntax error, command unrecognized.\r\n"
#define RC501               "501 Syntax error in parameters or arguments.\r\n"
#define RC500               "500 %s\r\n"
#define RC530               "530 Not logged in.\r\n"
#define RC550_S             "550 Requested action not taken.\r\n"
#define RC550               "550 %s\r\n"

/* strlen macro */
#define str_len(s) (s==NULL?0:strlen(s))

/* Representation type */
struct rep_type {
    char type;
    char code;
};

/* Connection state type */
typedef struct connection_state {
    int auth;
    int state;
    char type;
    char mode;
    int clientd;
    int pasv_socketd;
    int pasv_clientd;
    int s_length;
    char s_buffer[BUFFER_SIZE];
    char dir[BUFFER_SIZE];
    void *pthread;
} cs_t;

/**
 * @brief Handles the user case command.
 */
extern void user(cs_t *conn);

/**
 * @brief Handles the quit case command.
 */
extern void quit(cs_t *conn);

/**
 * @brief Handles the cwd case command.
 *        Does not accept './' and '../' in the string.
 */
extern void cwd(cs_t *conn);

/**
 * @brief Handles the cdup case command.
 *        Current clientd goes one dir up.
 */
extern void cdup(cs_t *conn);

/**
 * @brief Handles the type case command.
 *        Type is either A or I.
 */
extern void type(cs_t *conn);

/**
 * @brief Handles the mode case command.
 *        Mode is always S.
 */
extern void mode(cs_t *conn);

/**
 * @brief Handles the stru case command.
 *        Sructure is always F.
 */
extern void stru(cs_t *conn);

/**
 * @brief Handles the retr case command.
 *        Checks for pasv, the file. If everything is ok
 *        sends the file to the pasv client.
 */
extern void retr(cs_t *conn);

/**
 * @brief Handles the pasv case command.
 *        Instantiates a new passive socket. RETR and NLST
 *        need it to get data.
 */
extern void pasv(cs_t *conn);

/**
 * @brief Handles the nlst case command.
 *        Checks for pasv and then sends the current items in
 *        directory.
 */
extern void nlst(cs_t *conn);

#endif
