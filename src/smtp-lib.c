/**
 * File:         src/smtp.c
 * Description:  SMTP protocol helper function.
 * Author:       Tomasz Pieczerak (tphaster)
 */

/* TODO
 *  smtp_send_reply()
 */

#include <stdio.h>

#include <string.h>
#include <unistd.h>

#include "smtp-lib.h"
#include "system.h"

int smtp_send_command (int sockfd, size_t cmd)
{
    char hostname[HOSTNAME_MAXLEN], cmd_line[LINE_MAXLEN];
    size_t pos = 0;

    switch (cmd) {
        case EHLO:
        case HELO:
            /* command code */
            if (EHLO == cmd)
                strcpy(cmd_line, "EHLO ");
            else
                strcpy(cmd_line, "HELO ");
            pos = 5;

            /* hostname */
            bzero(hostname, sizeof hostname);
            gethostname(hostname, sizeof hostname);
            strncpy(cmd_line+pos, hostname, LINE_MAXLEN-pos);
            pos += strlen(hostname);

            /* terminating CRLF */
            strcpy(cmd_line+pos, "\r\n");

            Writen(sockfd, cmd_line, strlen(cmd_line));
            break;

        case MAIL:
            break;
        case RCPT:
            break;
        case DATA:
            break;
        case RSET:
            break;
        case VRFY:
            break;
        case NOOP:
            break;
        case QUIT:
            break;
        default:
            break;
    }
    return 0;
}

