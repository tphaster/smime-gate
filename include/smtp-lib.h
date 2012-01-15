/**
 * File:        include/smtp-lib.h
 * Description: Header file for basic SMTP communication.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#ifndef __SMTP_LIB_H
#define __SMTP_LIB_H

#include <stdint.h>
#include <sys/types.h>

/*** SMTP Commands ***/
#define EHLO    1       /* Extended HELLO */
#define HELO    2       /* HELLO */
#define MAIL    3       /* MAIL */
#define RCPT    4       /* RECIPIENT */
#define DATA    5       /* DATA */
#define RSET    6       /* RESET */
#define VRFY    7       /* VERIFY */
#define NOOP    8       /* NOOP */
#define QUIT    9       /* QUIT */

/** Retrieving recipient number from RCPT command **/
#define RCPT_N(x)   (4+((x)<<4))        /* make RCPT command for 'x' rcpt */
#define GET_CMD(x)  ((x) & 0xF)         /* cut rcpt number, pure command */
#define GET_RNO(x)  (((x) & ~0xF)>>4)   /* get rcpt number from command */

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
#define SMTP_CLEAR  0       /* clear */
#define SMTP_EHLO   1       /* after EHLO/HELO receipt */
#define SMTP_MAIL   2       /* after MAIL receipt */
#define SMTP_RCPT   3       /* after (at last one) RCPT receipt */
#define SMTP_DATA   4       /* receiving mail data */


/*** Constants ***/
#define LINE_MAXLEN         256     /* maximum SMTP line length  */
#define DOMAIN_MAXLEN       128     /* maximum domain name length */
#define ADDR_MAXLEN         128     /* maximum mail address length */
#define CMD_MAXLEN          128     /* maximum command data length */

/** Errors -- function fails to complete action **/
#define NULLPTR     -1      /* NULL pointer dereference */
#define RCVERROR    -2      /* receipt error */
#define BADARG      -3      /* bad function arguments */
#define SENDERROR   -4      /* sending error */

/** Warnings -- function ended well, but receipt was invalid for SMTP **/
#define RCV_BADPARAM    1   /* received command with bad parameter */
#define RCV_NKNOWNCMD   2   /* received unknown command */
#define RCV_NKNOWNRPLY  3   /* received unknown reply */


/*** Typedefs ***/
#include "smtp-types.h"

/** SMTP Command **/
struct smtp_command {
    uint16_t code;          /* command code (see SMTP Commands) */
    char data[CMD_MAXLEN];  /* additional data */
};

/** SMTP Reply **/
struct smtp_reply {
    uint16_t code;              /* reply code (see SMTP Replies) */
    char msg[LINE_MAXLEN-4];    /* message sent in reply */
};

/** ESMTP Extensions **/
struct esmtp_ext {
    uint8_t ext[NO_EXT];    /* table for extensions, ex. esmtp_ext.ext[EXPN]
                             * (see ESMTP Extensions)*/
};

/*** Functions ***/
int smtp_send_command (int sockfd, size_t cmd, struct mail_object *mail);
int smtp_send_reply (int sockfd, size_t code, const char *msg, size_t msg_len);
ssize_t smtp_readline (int fd, void *vptr, size_t maxlen);
int smtp_recv_command (int sockfd, struct smtp_command *cmd);
int smtp_recv_reply (int sockfd, struct smtp_reply *rply);

#endif  /* __SMTP_LIB_H */

