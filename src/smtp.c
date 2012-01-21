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
#include "smtp-lib.h"
#include "smtp.h"
#include "system.h"

#define MAIL_START_LEN  512     /* default mail block size */

static ssize_t smtp_recv_mail_data (int sockfd, char **buf_ptr,
                                    size_t *buf_size);

/* smtp_send_mail - send a mail object through connected socket */
int smtp_send_mail (int sockfd, struct mail_object *mail, int cli)
{
    int ret;
    unsigned int i;
    struct esmtp_ext ext;
    struct smtp_reply rply;

    if (NULL == mail)
        return NULLPTR;

    /* only for new SMTP sessions */
    if (cli & SMTP_CLI_NEW) {
        /* Receive first welcome reply */
        if (0 != smtp_recv_reply(sockfd, &rply) || R220 != rply.code) {
            close(sockfd);
            return ERECVERR;
        }

        /* EHLO */
        if (0 != smtp_send_command(sockfd, EHLO, NULL)) {
            close(sockfd);
            return ESENDERR;
        }

        /* Receive ESMTP Extensions */
        bzero(&ext, sizeof(ext));

        for (;;) {
            if (0 != smtp_recv_reply(sockfd, &rply)) {
                close(sockfd);
                return ERECVERR;
            }

            if (R250E == rply.code) {
                if (0 == strncmp("8BITMIME", rply.msg, 9))
                    ext.ext[_8BITMIME] = 1;
                else if (0 == strncmp("DSN", rply.msg, 4))
                    ext.ext[DSN] = 1;
                else if (0 == strncmp("ETRN", rply.msg, 5))
                    ext.ext[ETRN] = 1;
                else if (0 == strncmp("EXPN", rply.msg, 6))
                    ext.ext[EXPN] = 1;
                else if (0 == strncmp("HELP", rply.msg, 7))
                    ext.ext[HELP] = 1;
                else if (0 == strncmp("ONEX", rply.msg, 8))
                    ext.ext[ONEX] = 1;
                else if (0 == strncmp("PIPELINING", rply.msg, 11))
                    ext.ext[PIPELINING] = 1;
                else if (0 == strncmp("SIZE", rply.msg, 5))
                    ext.ext[SIZE] = 1;
                else if (0 == strncmp("VERB", rply.msg, 5))
                    ext.ext[VERB] = 1;
                else if (0 == strncmp("VRFY", rply.msg, 5))
                    ext.ext[VRFY] = 1;
                else {
                    /* unknown extension, ignore */;
                }
            }
            else if (R250 == rply.code)
                break;
            else {
                close(sockfd);
                return ERECVERR;  /* something went wrong... */
            }
        }
    }   /* end of section for new SMTP session */

    /* MAIL */
    if (0 != smtp_send_command(sockfd, MAIL, mail)) {
        close(sockfd);
        return ESENDERR;
    }
    if (0 != smtp_recv_reply(sockfd, &rply) || R250 != rply.code) {
        close(sockfd);
        return ERECVERR;
    }

    /* RCPT */
    for (i = 0; i < mail->no_rcpt; ++i) {
        if (0 != smtp_send_command(sockfd, RCPT_N(i), mail)) {
            close(sockfd);
            return ESENDERR;
        }
        if (0 != smtp_recv_reply(sockfd, &rply) || R250 != rply.code) {
            close(sockfd);
            return ERECVERR;
        }
    }

    /* DATA */
    if (0 != smtp_send_command(sockfd, DATA, NULL)) {
        close(sockfd);
        return ESENDERR;
    }
    if (0 != smtp_recv_reply(sockfd, &rply) || R354 != rply.code) {
        close(sockfd);
        return ERECVERR;
    }

    /* Sending data */
    if (((ssize_t) mail->data_size)
            != writen(sockfd, mail->data, mail->data_size))
    {
        close(sockfd);
        return ESENDERR;
    }
    if (3 != writen(sockfd, ".\r\n", 3)) {  /* ending sequence */
        close(sockfd);
        return ESENDERR;
    }

    if (0 != smtp_recv_reply(sockfd, &rply) || R250 != rply.code) {
        close(sockfd);
        return ERECVERR;
    }
    ret = 0;    /* mail object successfully sent */

    if (cli & SMTP_CLI_LST) {
        /* QUIT */
        if (0 != smtp_send_command(sockfd, QUIT, NULL))
            ret = WQUITNSEND;
        if (0 != smtp_recv_reply(sockfd, &rply) || R221 != rply.code)
            ret = WQUITRNRCV;

        close(sockfd);
    }

    return ret;
}

/* smtp_recv_mail - receive mail object from connected socket *
 *                  (SMTP server)                             */
int smtp_recv_mail (int sockfd, struct mail_object *mail, char *filename,
                    int srv)
{
    int ret, cmd_ret;
    unsigned int i;
    char **temp_rcpt;
    struct smtp_command cmd;
    ssize_t data_size;          /* size of data received */
    int state = SMTP_CLEAR;     /* SMTP server states */

    bzero(mail, sizeof(struct mail_object));

    /* sent welcome reply, if it is a new session */
    if (SMTP_SRV_NEW == srv) {
        if (0 != smtp_send_reply(sockfd, R220, NULL, 0)) {
            close(sockfd);
            return ESENDERR;
        }
    }
    else if (SMTP_SRV_ERR == srv)
        state = SMTP_ERR;
    else
        state = SMTP_EHLO;

    for (;;) {
        /* receiving mail object data */
        if (SMTP_DATA == state) {
            data_size = smtp_recv_mail_data(sockfd, &(mail->data), NULL);
            if (data_size <= 0) {
                close(sockfd);
                return ERECVERR;
            }
            mail->data_size = data_size;

            /* save mail to disk */
            if (0 == save_mail_to_file(mail, filename)) {
                smtp_send_reply(sockfd, R250, NULL, 0);     /* mail accepted */
                return 0;
            }
            else {
                free(mail->data);
                mail->data_size = 0;
                /* insufficient system storage */
                smtp_send_reply(sockfd, R452, NULL, 0);
                state = SMTP_RCPT;
            }
        }

        /* receiving command */
        if (0 > (cmd_ret = smtp_recv_command(sockfd, &cmd))) {
            close(sockfd);
            return ERECVERR;
        }

        switch (state) {
            case SMTP_CLEAR:    /* new SMTP session */
                if (EHLO == cmd.code || HELO == cmd.code)
                {
                    if (0 != cmd_ret) {
                        /* unable to accommodate parameters */
                        ret = smtp_send_reply(sockfd, R455, NULL, 0);
                        break;
                    }

                    state = SMTP_EHLO;              /* EHLO/HELO received */
                    ret = smtp_send_reply(sockfd, R250, NULL, 0);   /* OK */
                }
                else if (MAIL == cmd.code || RCPT == cmd.code ||
                         DATA == cmd.code) {
                    /* bad sequence of commands*/
                    ret = smtp_send_reply(sockfd, R503, NULL, 0);
                }
                else if (QUIT == cmd.code) {
                    smtp_send_reply(sockfd, R221, NULL, 0);
                    close(sockfd);

                    return EQUITRECV;   /* mail not received, client quits */
                }
                else if (RSET == cmd.code || NOOP == cmd.code)
                    ret = smtp_send_reply(sockfd, R250, NULL, 0);   /* OK */
                else if (VRFY == cmd.code)
                    /* cannot verify user*/
                    ret = smtp_send_reply(sockfd, R252, NULL, 0);
                else
                    /* syntax error, command unrecognized */
                    ret = smtp_send_reply(sockfd, R500, NULL, 0);

                break;  /* end of SMTP_CLEAR */

            case SMTP_EHLO:     /* EHLO/HELO received */
                if (EHLO == cmd.code || HELO == cmd.code ||
                    RCPT == cmd.code || DATA == cmd.code)
                {
                    /* bad sequence of commands*/
                    ret = smtp_send_reply(sockfd, R503, NULL, 0);
                }
                else if (MAIL == cmd.code) {
                    if (0 != cmd_ret) {
                        /* unable to accommodate parameters */
                        ret = smtp_send_reply(sockfd, R455, NULL, 0);
                        break;
                    }

                    if (NULL == (mail->mail_from = malloc(strlen(cmd.data)+1)))
                    {
                        /* insufficient system storage */
                        ret = smtp_send_reply(sockfd, R452, NULL, 0);
                        break;
                    }
                    state = SMTP_MAIL;                  /* MAIL received */
                    strcpy(mail->mail_from, cmd.data);
                    ret = smtp_send_reply(sockfd, R250, NULL, 0);  /* OK */
                }
                else if (QUIT == cmd.code) {
                    smtp_send_reply(sockfd, R221, NULL, 0);
                    close(sockfd);

                    return EQUITRECV;   /* mail not received, client quits */
                }
                else if (RSET == cmd.code || NOOP == cmd.code)
                    ret = smtp_send_reply(sockfd, R250, NULL, 0);   /* OK */
                else if (VRFY == cmd.code)
                    /* cannot verify user*/
                    ret = smtp_send_reply(sockfd, R252, NULL, 0);
                else
                    /* syntax error, command unrecognized */
                    ret = smtp_send_reply(sockfd, R500, NULL, 0);

                break;  /* end of SMTP_EHLO */

            case SMTP_MAIL:     /* MAIL received*/
                if (EHLO == cmd.code || HELO == cmd.code ||
                    MAIL == cmd.code || DATA == cmd.code)
                {
                    /* bad sequence of commands*/
                    ret = smtp_send_reply(sockfd, R503, NULL, 0);
                }
                else if (RCPT == cmd.code) {
                    if (0 != cmd_ret) {
                        /* unable to accommodate parameters */
                        ret = smtp_send_reply(sockfd, R455, NULL, 0);
                        break;
                    }

                    if (NULL == (mail->rcpt_to = malloc(sizeof(char *)))) {
                        /* insufficient system storage */
                        ret = smtp_send_reply(sockfd, R452, NULL, 0);
                        break;
                    }
                    if (NULL ==
                        (mail->rcpt_to[0] = malloc(strlen(cmd.data)+1)))
                    {
                        free(mail->rcpt_to);
                        mail->rcpt_to = NULL;
                        /* insufficient system storage */
                        ret = smtp_send_reply(sockfd, R452, NULL, 0);
                        break;
                    }
                    state = SMTP_RCPT;                  /* RCPT received */
                    strcpy(mail->rcpt_to[0], cmd.data);
                    mail->no_rcpt = 1;
                    ret = smtp_send_reply(sockfd, R250, NULL, 0);  /* OK */
                }
                else if (QUIT == cmd.code) {
                    smtp_send_reply(sockfd, R221, NULL, 0);
                    close(sockfd);
                    free_mail_object(mail);

                    return EQUITRECV;   /* mail not received, client quits */
                }
                else if (RSET == cmd.code) {
                    free_mail_object(mail);
                    state = SMTP_EHLO;
                    ret = smtp_send_reply(sockfd, R250, NULL, 0); /* OK */
                }
                else if (NOOP == cmd.code)
                    ret = smtp_send_reply(sockfd, R250, NULL, 0);   /* OK */
                else if (VRFY == cmd.code)
                    /* cannot verify user*/
                    ret = smtp_send_reply(sockfd, R252, NULL, 0);
                else
                    /* syntax error, command unrecognized */
                    ret = smtp_send_reply(sockfd, R500, NULL, 0);

                break;  /* end of SMTP_MAIL */

            case SMTP_RCPT:     /* RCPT received */
                if (EHLO == cmd.code || HELO == cmd.code || MAIL == cmd.code) {
                    /* bad sequence of commands*/
                    ret = smtp_send_reply(sockfd, R503, NULL, 0);
                }
                else if (RCPT == cmd.code) {
                    if (0 != cmd_ret) {
                        /* unable to accommodate parameters */
                        ret = smtp_send_reply(sockfd, R455, NULL, 0);
                        break;
                    }
                    /* next recipient */
                    temp_rcpt = mail->rcpt_to;
                    mail->rcpt_to = malloc((mail->no_rcpt+1) * sizeof(char *));
                    if (NULL == mail->rcpt_to) {
                        mail->rcpt_to = temp_rcpt;
                        /* insufficient system storage */
                        ret = smtp_send_reply(sockfd, R452, NULL, 0);
                        break;
                    }

                    for (i = 0; i < mail->no_rcpt; ++i)
                        mail->rcpt_to[i] = temp_rcpt[i];
                    free(temp_rcpt);
                    temp_rcpt = NULL;

                    mail->rcpt_to[mail->no_rcpt] = malloc(strlen(cmd.data)+1);
                    if (NULL == mail->rcpt_to[mail->no_rcpt]) {
                        /* insufficient system storage */
                        ret = smtp_send_reply(sockfd, R452, NULL, 0);
                        break;
                    }
                    strcpy(mail->rcpt_to[mail->no_rcpt], cmd.data);
                    mail->no_rcpt += 1;

                    ret = smtp_send_reply(sockfd, R250, NULL, 0);   /* OK */
                }
                else if (DATA == cmd.code) {
                    state = SMTP_DATA;  /* DATA received */
                    /* start mail input */
                    ret = smtp_send_reply(sockfd, R354, NULL, 0);
                }
                else if (QUIT == cmd.code) {
                    smtp_send_reply(sockfd, R221, NULL, 0);
                    close(sockfd);
                    free_mail_object(mail);

                    return EQUITRECV;   /* mail not received, client quits */
                }
                else if (RSET == cmd.code) {
                    free_mail_object(mail);
                    state = SMTP_EHLO;
                    ret = smtp_send_reply(sockfd, R250, NULL, 0);   /* OK */
                }
                else if (NOOP == cmd.code)
                    ret = smtp_send_reply(sockfd, R250, NULL, 0);   /* OK */
                else if (VRFY == cmd.code)
                    /* cannot verify user*/
                    ret = smtp_send_reply(sockfd, R252, NULL, 0);
                else
                    /* syntax error, command unrecognized */
                    ret = smtp_send_reply(sockfd, R500, NULL, 0);

                break;  /* end of SMTP_RCPT */

            case SMTP_ERR:      /* server is dysfunctional */
                if (EHLO == cmd.code || HELO == cmd.code ||
                    RCPT == cmd.code || DATA == cmd.code)
                {
                    /* bad sequence of commands*/
                    ret = smtp_send_reply(sockfd, R503, NULL, 0);
                }
                else if (MAIL == cmd.code)
                    /* exceeded storage allocation */
                    ret = smtp_send_reply(sockfd, R552, NULL, 0);
                else if (QUIT == cmd.code) {
                    smtp_send_reply(sockfd, R221, NULL, 0);
                    close(sockfd);

                    return EQUITRECV;   /* mail not received, client quits */
                }
                else if (RSET == cmd.code || NOOP == cmd.code)
                    ret = smtp_send_reply(sockfd, R250, NULL, 0);   /* OK */
                else if (VRFY == cmd.code)
                    /* cannot verify user*/
                    ret = smtp_send_reply(sockfd, R252, NULL, 0);
                else
                    /* syntax error, command unrecognized */
                    ret = smtp_send_reply(sockfd, R500, NULL, 0);

                break;  /* end of SMTP_ERR */

            default:
                /* this can't happen */;
        }

        if (0 != ret) {
            free_mail_object(mail);
            close(sockfd);
            return ESENDERR;
        }
    }
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

/* smtp_recv_mail_data - accepts mail data from client, it will continue     *
 *                       receiving until it gets .CRLF or system runs out of *
 *                       memory.                                             */

/* data receipt states */
#define D_START     0       /* clear, lookin for CR */
#define D1_LF       1       /* CR received, looking for LF */
#define D2_DOT      2       /* LF received, looking for dot */
#define D3_CR       3       /* dot received, looking for CR */
#define D4_LF       4       /* second CR received, looking for LF */

static ssize_t smtp_recv_mail_data (int sockfd, char **buf_ptr,
                                    size_t *buf_size)
{
    int rc, state;
    size_t n, buflen;
    char c, *ptr, *buf, *temp_buf;

    if (NULL == (buf = malloc(MAIL_START_LEN * sizeof(char))))
        return -1;

    state = D_START;
    ptr = buf;
    buflen = MAIL_START_LEN;
    n = 0;

    for (;;) {
        if ( (rc = buf_read(sockfd, &c)) == 1) {
            *ptr++ = c;

            if (D4_LF == state) {
                if (c == '\n') {    /* end of mail */
                    n -= 2;     /* ".CRLF" isn't a part of mail */
                    *(ptr-3) = '\0';
                    break;
                }
                else if (c == '\r')
                    state = D1_LF;  /* CR read; check for LF in next turn */
                else
                    state = D_START;
            }
            else if (D3_CR == state) {
                if (c == '\r')
                    state = D4_LF;  /* second CR read; check for *
                                     * second LF in next turn    */
                else if (c == '\r')
                    state = D1_LF;  /* CR read; check for LF in next turn */
                else
                    state = D_START;
            }
            else if (D2_DOT == state) {
                if (c == '.')
                    state = D3_CR;  /* dot read; check for second *
                                     * CR in next turn            */
                else if (c == '\r')
                    state = D1_LF;  /* CR read; check for LF in next turn */
                else
                    state = D_START;
            }
            else if (D1_LF == state) {
                if (c == '\n')
                    state = D2_DOT; /* LF read; next turn check for dot */
                else if (c == '\r')
                    state = D1_LF;  /* CR read; check for LF in next turn */
                else
                    state = D_START;
            }
            else if (c == '\r')
                state = D1_LF;  /* CR read; check for LF in next turn */
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
            if (NULL == (buf = malloc(buflen * sizeof(char)))) {
                free(temp_buf);
                return -1;
            }
            ptr = buf+n;
            memcpy(buf, temp_buf, n);

            free(temp_buf);
            temp_buf = NULL;
        }
    }

    *buf_ptr = buf;
    if (NULL != buf_size)
        *buf_size = buflen;

    return n;
}

/* save_mail_to_disc - saves given mail object to file */
int save_mail_to_file (struct mail_object *mail, const char *filename)
{
    FILE *fp;
    size_t i;

    if ((fp = fopen(filename, "w")) == NULL)
        return EFOPEN;  /* can't open file */

    /* save sender address */
    fprintf(fp, "%s\n", mail->mail_from);

    /* save recipients addresses */
    fprintf(fp, "%d\n", mail->no_rcpt);
    for (i = 0; i < mail->no_rcpt; ++i)
        fprintf(fp, "%s\n", mail->rcpt_to[i]);

    /* save mail data */
    for (i = 0; i < mail->data_size; ++i)
        putc(mail->data[i], fp);

    fclose(fp);

    return 0;
}

/* load_mail_from_file - loads mail object from file */
int load_mail_from_file (const char *filename, struct mail_object *mail)
{
    FILE *fp;
    char buf[ADDR_MAXLEN];
    size_t len, i;
    long pos;

    if ((fp = fopen(filename, "r")) == NULL)
        return EFOPEN;  /* can't open file */

    /* get MAIL FROM: */
    if (fgets(buf, LINE_MAXLEN, fp) == NULL) {
        fclose(fp);
        return EUEXEOF; /* error or EOF */
    }

    len = strlen(buf)-1;
    buf[len] = '\0';
    if (NULL == (mail->mail_from = malloc(len))) {
        fclose(fp);
        return ENOMEM;
    }
    strncpy(mail->mail_from, buf, len);

    /* get RCPT TO: */
    if (fgets(buf, LINE_MAXLEN, fp) == NULL) {
        fclose(fp);
        free_mail_object(mail);
        return EUEXEOF; /* error or EOF */
    }

    mail->no_rcpt = atoi(buf);
    if (NULL == (mail->rcpt_to = calloc(mail->no_rcpt, sizeof(char *)))) {
        fclose(fp);
        free_mail_object(mail);
        return ENOMEM;
    }

    for (i = 0; i < mail->no_rcpt; ++i) {
        if (fgets(buf, LINE_MAXLEN, fp) == NULL) {
            fclose(fp);
            free_mail_object(mail);
            return EUEXEOF; /* error or EOF */
        }

        len = strlen(buf)-1;
        buf[len] = '\0';
        if (NULL == (mail->rcpt_to[i] = malloc(len))) {
            fclose(fp);
            free_mail_object(mail);
            return ENOMEM;
        }
        strncpy(mail->rcpt_to[i], buf, len);
    }

    /* get DATA */
    pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    mail->data_size = ftell(fp)-pos;
    fseek(fp, pos, SEEK_SET);

    if (NULL == (mail->data = malloc(mail->data_size))) {
        fclose(fp);
        free_mail_object(mail);
        return ENOMEM;
    }
    fread(mail->data, sizeof(char), mail->data_size, fp);

    return 0;
}

