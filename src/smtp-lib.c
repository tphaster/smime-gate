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

int smtp_send_command (int sockfd, size_t cmd, struct mail_object *mail)
{
    char hostname[HOSTNAME_MAXLEN], cmd_line[LINE_MAXLEN];
    size_t i;
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
            strncpy(cmd_line+pos, hostname, HOSTNAME_MAXLEN);
            pos += min(HOSTNAME_MAXLEN, strlen(hostname));

            /* terminating CRLF */
            strcpy(cmd_line+pos, "\r\n");

            Writen(sockfd, cmd_line, strlen(cmd_line));
            break;

        case MAIL:
            if (NULL == mail)
                return -1;  /* NULL pointer dereference*/

            /* command code */
            strcpy(cmd_line, "MAIL FROM:<");
            pos = 11;

            /* sender (Reverse-path) */
            strncpy(cmd_line+pos, mail->mail_from, ADDR_MAXLEN);
            pos += min(ADDR_MAXLEN, strlen(mail->mail_from));

            /* terminating CRLF */
            strcpy(cmd_line+pos, ">\r\n");

            Writen(sockfd, cmd_line, strlen(cmd_line));
            break;

        case RCPT:
            if (NULL == mail)
                return -1;  /* NULL pointer dereference*/

            for (i = 0; i < mail->no_rcpt; ++i) {
                /* command code */
                strcpy(cmd_line, "RCPT TO:<");
                pos = 9;

                /* sender (Reverse-path) */
                strncpy(cmd_line+pos, mail->rcpt_to[i], ADDR_MAXLEN);
                pos += min(ADDR_MAXLEN, strlen(mail->rcpt_to[i]));

                /* terminating CRLF */
                strcpy(cmd_line+pos, ">\r\n");

                Writen(sockfd, cmd_line, strlen(cmd_line));
            }
            break;

        case DATA:
            /* command code and terminating CRLF */
            strcpy(cmd_line, "DATA\r\n");

            Writen(sockfd, cmd_line, strlen(cmd_line));
            break;

        case RSET:
            /* command code and terminating CRLF */
            strcpy(cmd_line, "RSET\r\n");

            Writen(sockfd, cmd_line, strlen(cmd_line));
            break;

        case VRFY:
            return -1;  /* not implemented */
            break;

        case NOOP:
            /* command code and terminating CRLF */
            strcpy(cmd_line, "NOOP\r\n");

            Writen(sockfd, cmd_line, strlen(cmd_line));
            break;

        case QUIT:
            /* command code and terminating CRLF */
            strcpy(cmd_line, "QUIT\r\n");

            Writen(sockfd, cmd_line, strlen(cmd_line));
            break;

        default:
            return -1;  /* no such command */
            break;
    }

    return 0;
}

