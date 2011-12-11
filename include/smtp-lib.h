/**
 * File:        include/smtp-lib.h
 * Description: Header file for basic SMTP communication.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include <sys/types.h>

/* SMTP Commands */
#define EHLO    1
#define HELO    2
#define MAIL    3
#define RCPT    4
#define DATA    5
#define RSET    6
#define VRFY    7
#define NOOP    8
#define QUIT    9

#define HOSTNAME_MAXLEN     128
#define LINE_MAXLEN         256

int smtp_send_command (int sockfd, size_t cmd);
int smtp_send_reply (int sockfd);

