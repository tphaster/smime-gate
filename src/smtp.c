/**
 * File:        src/smtp.c
 * Description: High-level SMTP server/client functions
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "smtp.h"
#include "system.h"

#define DSTART       0
#define DDOT_READ    1
#define DCR_READ     2

#define MAIL_START_LEN  512

ssize_t smtp_recv_mail_data (int sockfd, char **buf_ptr, size_t *buf_size);
int save_mail_to_disk (struct mail_object *mail, char *filename);
int load_mail_from_disk (char *filename, struct mail_object *mail);


int smtp_send_mail (int sockfd, struct mail_object *mail)
{
    size_t i;
    struct esmtp_ext ext;
    struct smtp_reply *rply = Malloc(sizeof(struct smtp_reply));

    smtp_recv_reply(sockfd, rply);
    if (R220 != rply->code) {
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
            free(rply);
            return RCVERROR;   /* something goes wrong... */
        }
    }

    /* MAIL */
    smtp_send_command(sockfd, MAIL, mail);
    smtp_recv_reply(sockfd, rply);
    if (R250 != rply->code) {
        free(rply);
        return RCVERROR;
    }

    /* RCPT */
    for (i = 0; i < mail->no_rcpt; ++i) {
        smtp_send_command(sockfd, RCPT_N(i), mail);
        smtp_recv_reply(sockfd, rply);
        if (R250 != rply->code) {
            free(rply);
            return RCVERROR;
        }
    }

    /* DATA */
    smtp_send_command(sockfd, DATA, NULL);
    smtp_recv_reply(sockfd, rply);
    if (R354 != rply->code) {
        free(rply);
        return RCVERROR;
    }

    /* Sending data */
    Writen(sockfd, mail->data, mail->data_size);
    Writen(sockfd, ".\r\n", 3);     /* end of message */
    smtp_recv_reply(sockfd, rply);
    if (R250 != rply->code) {
        free(rply);
        return RCVERROR;
    }

    /* QUIT */
    smtp_send_command(sockfd, QUIT, NULL);
    smtp_recv_reply(sockfd, rply);
    if (R221 != rply->code) {
        free(rply);
        return RCVERROR;
    }

    free(rply);
    return 0;
}

int smtp_recv_mail (int sockfd, struct mail_object *mail, char *filename)
{
    size_t i;
    ssize_t data_size;
    int state = SMTP_CLEAR;
    int mail_rcvd = 0;
    char **temp_rcpt;
    struct smtp_command *cmd = malloc(sizeof(struct smtp_command));

    smtp_send_reply(sockfd, R220, NULL, 0);

    while (state >= 0) {
        if (SMTP_DATA == state) {
            data_size = smtp_recv_mail_data(sockfd, &(mail->data), NULL);
            if (data_size <= 0) {
                close(sockfd);
                free(cmd);
                return RCVERROR;
            }

            mail->data_size = data_size;
            if (0 == save_mail_to_disk(mail, filename)) {
                smtp_send_reply(sockfd, R250, NULL, 0);
                mail_rcvd = 1;
            }
            else
                smtp_send_reply(sockfd, R552, NULL, 0);

            state = SMTP_EHLO;
            continue;
        }
        else
            smtp_recv_command(sockfd, cmd);

        switch(state) {
            case SMTP_CLEAR:
                if (EHLO == cmd->code || HELO == cmd->code) {
                    state = SMTP_EHLO;
                    smtp_send_reply(sockfd, R250, NULL, 0);
                }
                else if (MAIL == cmd->code || RCPT == cmd->code ||
                         DATA == cmd->code) {
                    smtp_send_reply(sockfd, R503, NULL, 0);
                }
                else if (QUIT == cmd->code) {
                    smtp_send_reply(sockfd, R221, NULL, 0);
                    state = SMTP_QUIT;
                }
                else if (RSET == cmd->code || NOOP == cmd->code)
                    smtp_send_reply(sockfd, R250, NULL, 0);
                else if (VRFY == cmd->code)
                    smtp_send_reply(sockfd, R252, NULL, 0);
                else
                    smtp_send_reply(sockfd, R500, NULL, 0);

                break;

            case SMTP_EHLO:
                if (EHLO == cmd->code || HELO == cmd->code ||
                        RCPT == cmd->code || DATA == cmd->code ) {
                    smtp_send_reply(sockfd, R503, NULL, 0);
                }
                else if (MAIL == cmd->code) {
                    if (0 == mail_rcvd) {
                        state = SMTP_MAIL;
                        mail->mail_from = malloc(strlen(cmd->data)+1);
                        strcpy(mail->mail_from, cmd->data);
                        smtp_send_reply(sockfd, R250, NULL, 0);
                    }
                    else
                        smtp_send_reply(sockfd, R552, NULL, 0);
                }
                else if (QUIT == cmd->code) {
                    smtp_send_reply(sockfd, R221, NULL, 0);
                    state = SMTP_QUIT;
                }
                else if (RSET == cmd->code || NOOP == cmd->code)
                    smtp_send_reply(sockfd, R250, NULL, 0);
                else if (VRFY == cmd->code)
                    smtp_send_reply(sockfd, R252, NULL, 0);
                else
                    smtp_send_reply(sockfd, R500, NULL, 0);
                break;

            case SMTP_MAIL:
                if (EHLO == cmd->code || HELO == cmd->code ||
                    MAIL == cmd->code || DATA == cmd->code ) {
                    smtp_send_reply(sockfd, R503, NULL, 0);
                }
                else if (RCPT == cmd->code) {
                    state = SMTP_RCPT;
                    mail->rcpt_to = malloc(sizeof(char *));
                    mail->rcpt_to[0] = malloc(strlen(cmd->data)+1);
                    strcpy(mail->rcpt_to[0], cmd->data);
                    mail->no_rcpt = 1;
                    smtp_send_reply(sockfd, R250, NULL, 0);
                }
                else if (QUIT == cmd->code) {
                    smtp_send_reply(sockfd, R221, NULL, 0);
                    free(mail->mail_from);
                    mail->mail_from = NULL;
                    state = SMTP_QUIT;
                }
                else if (RSET == cmd->code) {
                    free(mail->mail_from);
                    mail->mail_from = NULL;
                    smtp_send_reply(sockfd, R250, NULL, 0);
                }
                else if (NOOP == cmd->code)
                    smtp_send_reply(sockfd, R250, NULL, 0);
                else if (VRFY == cmd->code)
                    smtp_send_reply(sockfd, R252, NULL, 0);
                else
                    smtp_send_reply(sockfd, R500, NULL, 0);
                break;

            case SMTP_RCPT:
                if (EHLO == cmd->code || HELO == cmd->code ||
                    MAIL == cmd->code) {
                    smtp_send_reply(sockfd, R503, NULL, 0);
                }
                else if (RCPT == cmd->code) {
                    temp_rcpt = mail->rcpt_to;
                    mail->rcpt_to = malloc((mail->no_rcpt+1) * sizeof(char *));
                    for (i = 0; i < mail->no_rcpt; ++i)
                        mail->rcpt_to[i] = temp_rcpt[i];
                    free(temp_rcpt);
                    temp_rcpt = NULL;

                    mail->rcpt_to[mail->no_rcpt] = malloc(strlen(cmd->data)+1);
                    strcpy(mail->rcpt_to[mail->no_rcpt], cmd->data);
                    mail->no_rcpt += 1;
                    smtp_send_reply(sockfd, R250, NULL, 0);
                }
                else if (DATA == cmd->code) {
                    state = SMTP_DATA;
                    smtp_send_reply(sockfd, R354, NULL, 0);
                }
                else if (QUIT == cmd->code) {
                    smtp_send_reply(sockfd, R221, NULL, 0);

                    free(mail->mail_from);
                    mail->mail_from = NULL;

                    for (i = 0; i < mail->no_rcpt; ++i)
                        free(mail->rcpt_to[i]);
                    free(mail->rcpt_to);
                    mail->rcpt_to = NULL;

                    state = SMTP_QUIT;
                }
                else if (RSET == cmd->code) {
                    free(mail->mail_from);
                    mail->mail_from = NULL;

                    for (i = 0; i < mail->no_rcpt; ++i)
                        free(mail->rcpt_to[i]);
                    free(mail->rcpt_to);
                    mail->rcpt_to = NULL;

                    smtp_send_reply(sockfd, R250, NULL, 0);
                }
                else if (NOOP == cmd->code)
                    smtp_send_reply(sockfd, R250, NULL, 0);
                else if (VRFY == cmd->code)
                    smtp_send_reply(sockfd, R252, NULL, 0);
                else
                    smtp_send_reply(sockfd, R500, NULL, 0);

                break;

            default:
                /* this can't happen */;
        }
    }

    close(sockfd);
    free(cmd);

    if (mail_rcvd)
        return 0;
    else
        return RCVERROR;
}

/* WARNING! Function buf_read() is not safe for threads */
static ssize_t buf_read (int fd, char *ptr)
{
    static int read_cnt = 0;
    static char *read_ptr;
    static char read_buf[LINE_MAXLEN];

    /* if buffer is empty */
    if (read_cnt <= 0) {
again:
        /* put available data into buffer */
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                goto again;
            return -1;  /* reading error, check errno for more information */
        }
        else if (read_cnt == 0)
            return 0;   /* there is nothing more to read */

        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;

    return 1;   /* one character read */
}

ssize_t smtp_recv_mail_data (int sockfd, char **buf_ptr, size_t *buf_size)
{
    int rc, state;
    size_t n, buflen;
    char c, *ptr, *buf, *temp_buf;

    state = DSTART;
    buf = malloc(MAIL_START_LEN * sizeof(char));
    ptr = buf;
    buflen = MAIL_START_LEN;
    n = 0;

    for (;;) {
        if ( (rc = buf_read(sockfd, &c)) == 1) {
            *ptr++ = c;

            if (DCR_READ == state) {
                if (c == '\n') {    /* end of mail */
                    n -= 3; /* ".CRLF" isn't a part of mail */
                    break;
                }
                else
                    state = DSTART;
            }
            else if (DDOT_READ == state) {
                if (c == '\r') {
                    state = CR_READ;    /* CR read; next turn check for LF */
                    continue;
                }
                else
                    state = DSTART;
            }
            else if (c == '.')
                state = DDOT_READ;   /* "." read; check for CR in next turn */
        }
        else {
            *buf_ptr = NULL;
            *buf_size = 0;
            free(buf);

            if (rc == 1)
                return 0;   /* EOF, not enough data */
            else
                return -1;  /* error, errno set by read() */
        }

        ++n;

        if (n == buflen) {
            buflen += MAIL_START_LEN;
            temp_buf = buf;
            buf = malloc(buflen * sizeof(char));
            ptr = buf+n;
            memcpy(buf, temp_buf, n);

            free(temp_buf);
            temp_buf = NULL;
        }
    }

    *buf_ptr = buf;
    *buf_size = buflen;

    return n;
}

int save_mail_to_disk (struct mail_object *mail, char *filename)
{
    FILE *fp;
    size_t i;

    if ((fp = fopen(filename, "w")) == NULL)
        return -1;  /* can't open file */

    fprintf(fp, "%s\n", mail->mail_from);
    fprintf(fp, "%d\n", mail->no_rcpt);

    for (i = 0; i < mail->no_rcpt; ++i)
        fprintf(fp, "%s\n", mail->rcpt_to[i]);

    for (i = 0; i < mail->data_size; ++i)
        putc(mail->data[i], fp);

    return 0;
}

int load_mail_from_disk (char *filename, struct mail_object *mail)
{
    return 0;
}

