/**
 * File:        src/smtp.c
 * Description: High-level SMTP server/client functions
 * Author:      Tomasz Pieczerak (tphaster)
 */

/* TODO
 *  smtp_recv_mail()
 */

#include <stdlib.h>
#include <string.h>

#include "smtp.h"
#include "system.h"

int smtp_send_mail (int sockfd, struct mail_object *mail)
{
    size_t i;
    struct esmtp_ext ext;
    struct smtp_command *cmd = Malloc(sizeof(struct smtp_command));
    struct smtp_reply *rply = Malloc(sizeof(struct smtp_reply));

    smtp_recv_reply(sockfd, rply);
    if (R220 != rply->code) {
        free(cmd);
        free(rply);
        return RCVERROR;
    }

    /* EHLO */
    smtp_send_command(sockfd, EHLO, NULL);

    bzero(&ext, sizeof(ext));

    /* ESMTP Extensions */
    for (;;) {
        smtp_recv_reply(sockfd, rply);

        if (R250E == rply->code) {
            if (0 == strncmp("8BITMIME", rply->msg, 9))
                ext.ext[_8BITMIME] = 1;
            else if (0 == strncmp("DSN", rply->msg, 4))
                ext.ext[DSN] = 1;
            else if (0 == strncmp("ETRN", rply->msg, 5))
                ext.ext[ETRN] = 1;
            else if (0 == strncmp("EXPN", rply->msg, 6))
                ext.ext[EXPN] = 1;
            else if (0 == strncmp("HELP", rply->msg, 7))
                ext.ext[HELP] = 1;
            else if (0 == strncmp("ONEX", rply->msg, 8))
                ext.ext[ONEX] = 1;
            else if (0 == strncmp("PIPELINING", rply->msg, 11))
                ext.ext[PIPELINING] = 1;
            else if (0 == strncmp("SIZE", rply->msg, 5))
                ext.ext[SIZE] = 1;
            else if (0 == strncmp("VERB", rply->msg, 5))
                ext.ext[VERB] = 1;
            else if (0 == strncmp("VRFY", rply->msg, 5))
                ext.ext[VRFY] = 1;
            else {
                /* unknown extension, ignore */;
            }
        }
        else if (R250 == rply->code)
            break;
        else {
            free(cmd);
            free(rply);
            return RCVERROR;   /* something goes wrong... */
        }
    }

    /* MAIL */
    smtp_send_command(sockfd, MAIL, mail);
    smtp_recv_reply(sockfd, rply);
    if (R250 != rply->code) {
        free(cmd);
        free(rply);
        return RCVERROR;
    }

    /* RCPT */
    for (i = 0; i < mail->no_rcpt; ++i) {
        smtp_send_command(sockfd, RCPT_N(i), mail);
        smtp_recv_reply(sockfd, rply);
        if (R250 != rply->code) {
            free(cmd);
            free(rply);
            return RCVERROR;
        }
    }

    /* DATA */
    smtp_send_command(sockfd, DATA, NULL);
    smtp_recv_reply(sockfd, rply);
    if (R354 != rply->code) {
        free(cmd);
        free(rply);
        return RCVERROR;
    }

    /* Sending data */
    /*Writen(sockfd, mail->data, strlen(mail->data));*/
    Writen(sockfd, ".\r\n", 3);     /* end of message */
    smtp_recv_reply(sockfd, rply);
    if (R250 != rply->code) {
        free(cmd);
        free(rply);
        return RCVERROR;
    }

    /* QUIT */
    smtp_send_command(sockfd, QUIT, NULL);
    smtp_recv_reply(sockfd, rply);
    if (R221 != rply->code) {
        free(cmd);
        free(rply);
        return RCVERROR;
    }

    free(cmd);
    free(rply);
    return 0;
}

