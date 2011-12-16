/**
 * File:        include/smtp-lib.h
 * Description: Header file for basic SMTP communication.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include <sys/types.h>

/*** SMTP Commands ***/
#define EHLO    1
#define HELO    2
#define MAIL    3
#define RCPT    4
#define DATA    5
#define RSET    6
#define VRFY    7
#define NOOP    8
#define QUIT    9

/*** SMTP Replies ***/
#define R211     1  /* (after QUIT) closing connection */
#define R220     2  /* (on connection start) greeting */
#define R250     3  /* requested mail action okay, completed */
#define R250E    4  /* same as R250, but to be continued */
#define R251     5  /* (after RCPT) user not local; will forward to <forward-path> */
#define R354     6  /* (after DATA) start mail input; end with <CRLF>.<CRLF> */
#define R450     7  /* requested mail action not taken: mailbox unavailable */
#define R451     8  /* requested action aborted: local error in processing */
#define R452     9  /* requested action not taken: insufficient system storage */
#define R455    10  /* server unable to accommodate parameters */
#define R502    11  /* (after EHLO) command not implemented */
#define R503    12  /* bad sequence of commands */
#define R504    13  /* (after HELO/EHLO) command parameter not implemented */
#define R550    14  /* requested action not taken: mailbox unavailable */
#define R551    15  /* (after RCPT) user not local; please try <forward-path> */
#define R552    16  /* requested mail action aborted: exceeded storage allocation */
#define R553    17  /* (after RCPT) requested action not taken: mailbox name not allowed */
#define R554    18  /* transaction failed */
#define R555    19  /* MAIL FROM/RCPT TO parameters not recognized or not implemented */

/*** Constants ***/
#define DOMAIN_MAXLEN       128
#define LINE_MAXLEN         256
#define ADDR_MAXLEN         128

#define START       0
#define CR_READ     1

/*** Typedefs ***/

/* Mail object */
struct mail_object {
    char *mail_from;
    char **rcpt_to;
    size_t no_rcpt;
    char *data;
};

/* SMTP Command */
struct smtp_command {
    size_t code;
    union {
        char domain[DOMAIN_MAXLEN];
        char mail_from[ADDR_MAXLEN];
        char rcpt_to[ADDR_MAXLEN];
    } params;
};

/* SMTP Reply */
struct smtp_reply {
    size_t code;
    char msg[LINE_MAXLEN-4];
};

/*** Functions ***/
int smtp_send_command (int sockfd, size_t cmd, struct mail_object *mail);
int smtp_send_reply (int sockfd, size_t code, const char *msg, size_t msg_len);
ssize_t smtp_readline (int fd, void *vptr, size_t maxlen);

