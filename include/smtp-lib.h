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

#define RCPT_N(x)   (4+((x)<<4))
#define GET_CMD(x)  ((x) & 0xF)
#define GET_RNO(x)  (((x) & ~0xF)>>4)

/*** SMTP Replies ***/
#define R220     1  /* (on connection start) greeting */
#define R221     2  /* (after QUIT) closing connection */
#define R250     3  /* requested mail action okay, completed */
#define R250E    4  /* same as R250, but to be continued */
#define R251     5  /* (after RCPT) user not local; will forward to <forward-path> */
#define R252     6  /* cannot VRFY user, but will accept message and attempt delivery */
#define R354     7  /* (after DATA) start mail input; end with <CRLF>.<CRLF> */
#define R450     8  /* requested mail action not taken: mailbox unavailable */
#define R451     9  /* requested action aborted: local error in processing */
#define R452    10  /* requested action not taken: insufficient system storage */
#define R455    11  /* server unable to accommodate parameters */
#define R500    12  /* syntax error, command unrecognized */
#define R502    13  /* (after EHLO) command not implemented */
#define R503    14  /* bad sequence of commands */
#define R504    15  /* (after HELO/EHLO) command parameter not implemented */
#define R550    16  /* requested action not taken: mailbox unavailable */
#define R551    17  /* (after RCPT) user not local; please try <forward-path> */
#define R552    18  /* requested mail action aborted: exceeded storage allocation */
#define R553    19  /* (after RCPT) requested action not taken: mailbox name not allowed */
#define R554    20  /* transaction failed */
#define R555    21  /* MAIL FROM/RCPT TO parameters not recognized or not implemented */

/*** ESMTP Extensions ***/
#define NO_EXT  10  /* number of extensions */

#define _8BITMIME    0
#define DSN          1
#define ETRN         2
#define EXPN         3
#define HELP         4
#define ONEX         5
#define PIPELINING   6
#define VRFY         7
#define VERB         8
#define SIZE         9

/*** SMTP Server States ***/
#define SMTP_CLEAR  0
#define SMTP_EHLO   1
#define SMTP_MAIL   2
#define SMTP_RCPT   3
#define SMTP_DATA   4
#define SMTP_QUIT   -1


/*** Constants ***/
#define DOMAIN_MAXLEN       128
#define LINE_MAXLEN         256
#define ADDR_MAXLEN         128

#define START       0
#define CR_READ     1

/* Errors */
#define NULLPTR     -1
#define RCVERROR    -2
#define BADARG      -3

/* Warnings */
#define RCV_BADPARAM    1
#define RCV_NKNOWNCMD   2
#define RCV_NKNOWNRPLY  3


/*** Macros ***/
#define min(a,b)    ((a) < (b) ? (a) : (b))
#define max(a,b)    ((a) > (b) ? (a) : (b))

/* Define bzero() as a macro, if it's not in standard C library. */
#ifndef HAVE_BZERO
#define bzero(ptr,n)    memset(ptr, 0, n)
#endif

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
    char data [max(DOMAIN_MAXLEN, ADDR_MAXLEN)];
};

/* SMTP Reply */
struct smtp_reply {
    size_t code;
    char msg[LINE_MAXLEN-4];
};

/* ESMTP Extensions */
struct esmtp_ext {
    size_t ext[NO_EXT];
    size_t no_ext;
};

/*** Functions ***/
int smtp_send_command (int sockfd, size_t cmd, struct mail_object *mail);
int smtp_send_reply (int sockfd, size_t code, const char *msg, size_t msg_len);
ssize_t smtp_readline (int fd, void *vptr, size_t maxlen);
int smtp_recv_command (int sockfd, struct smtp_command *cmd);
int smtp_recv_reply (int sockfd, struct smtp_reply *rply);

