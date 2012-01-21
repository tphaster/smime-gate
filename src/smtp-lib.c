/**
 * File:         src/smtp.c
 * Description:  SMTP protocol helper function.
 * Author:       Tomasz Pieczerak (tphaster)
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "smtp-lib.h"
#include "system.h"

/* smtp_send_command - send SMTP Command on given socket */
int smtp_send_command (int sockfd, size_t cmd, struct mail_object *mail)
{
    int ret;
    size_t slen;
    char domain[DOMAIN_MAXLEN], cmd_line[LINE_MAXLEN];
    size_t pos = 0;
    int rcpt_no = GET_RNO(cmd);
    cmd = GET_CMD(cmd);

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
            bzero(domain, sizeof domain);
            if (-1 == gethostname(domain, sizeof domain))
                return BADDOMAIN;
            strncpy(cmd_line+pos, domain, CMD_MAXLEN);
            pos += min(DOMAIN_MAXLEN, strlen(domain));

            /* terminating CRLF */
            strcpy(cmd_line+pos, "\r\n");

            ret = writen(sockfd, cmd_line, slen = strlen(cmd_line));

            break;  /* end of HELO/EHLO */

        case MAIL:
            if (NULL == mail)
                return NULLPTR;  /* NULL pointer dereference */

            /* command code */
            strcpy(cmd_line, "MAIL FROM:<");
            pos = 11;

            /* sender (Reverse-path) */
            if (strlen(mail->mail_from) > ADDR_MAXLEN)
                return BADADDR;
            strncpy(cmd_line+pos, mail->mail_from, ADDR_MAXLEN);
            pos += min(ADDR_MAXLEN, strlen(mail->mail_from));

            /* terminating CRLF */
            strcpy(cmd_line+pos, ">\r\n");

            ret = writen(sockfd, cmd_line, slen = strlen(cmd_line));

            break;  /* end of MAIL */

        case RCPT:
            if (NULL == mail)
                return NULLPTR;     /* NULL pointer dereference*/

            if (rcpt_no < 0 || ((size_t) rcpt_no) > mail->no_rcpt)
                return BADARG;      /* bad parameter */

            /* command code */
            strcpy(cmd_line, "RCPT TO:<");
            pos = 9;

            /* sender (Reverse-path) */
            if (strlen(mail->rcpt_to[rcpt_no]) > ADDR_MAXLEN)
                return BADADDR;
            strncpy(cmd_line+pos, mail->rcpt_to[rcpt_no], CMD_MAXLEN);
            pos += min(ADDR_MAXLEN, strlen(mail->rcpt_to[rcpt_no]));

            /* terminating CRLF */
            strcpy(cmd_line+pos, ">\r\n");

            ret = writen(sockfd, cmd_line, slen = strlen(cmd_line));

            break;  /* end of RCPT */

        case DATA:
            /* command code and terminating CRLF */
            strcpy(cmd_line, "DATA\r\n");

            ret = writen(sockfd, cmd_line, slen = strlen(cmd_line));

            break;  /* end of DATA */

        case RSET:
            /* command code and terminating CRLF */
            strcpy(cmd_line, "RSET\r\n");

            ret = writen(sockfd, cmd_line, slen = strlen(cmd_line));

            break;  /* end of RSET */

        case VRFY:
            return BADARG;  /* not implemented */
            break;  /* end of VRFY */

        case NOOP:
            /* command code and terminating CRLF */
            strcpy(cmd_line, "NOOP\r\n");

            ret = writen(sockfd, cmd_line, slen = strlen(cmd_line));

            break;  /* end of NOOP */

        case QUIT:
            /* command code and terminating CRLF */
            strcpy(cmd_line, "QUIT\r\n");

            ret = writen(sockfd, cmd_line, slen = strlen(cmd_line));

            break;  /* end of QUIT */

        default:
            return BADARG;  /* no such command */
    }

    if (ret < 0 || (size_t) ret != slen)
        return SENDERROR;
    else
        return 0;
}

/* smtp_send_reply - send SMTP Reply on given socket */
int smtp_send_reply (int sockfd, size_t code, const char *msg, size_t msg_len)
{
    int ret;
    size_t slen;
    char domain[DOMAIN_MAXLEN], rply_line[LINE_MAXLEN];
    size_t pos = 0;

    switch (code) {
        case R220:  /* (on connection start) greeting */
            if (NULL == msg) {
                /* reply code */
                strcpy(rply_line, "220 ");
                pos = 4;

                /* hostname */
                bzero(domain, sizeof domain);
                if (-1 == gethostname(domain, sizeof domain))
                    return BADDOMAIN;
                strncpy(rply_line+pos, domain, RPLY_MAXLEN);
                pos += min(DOMAIN_MAXLEN, strlen(domain));

                /* text string and terminating CRLF */
                strcpy(rply_line+pos, " Service ready\r\n");

                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "220 ");

            break;  /* end of R220 */

        case R221:  /* (after QUIT) closing connection */
            if (NULL == msg) {
                /* reply code, text string and terminating <CRLF> */
                strcpy(rply_line, "221 closing connection, bye\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "221 ");

            break;  /* end of R221 */

        case R250:  /* requested mail action okay, completed */
            if (NULL == msg) {
                strcpy(rply_line, "250 OK\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "250 ");

            break;  /* end of R250 */

        case R250E:  /* same as R250, but to be continued */
            if (NULL == msg)
                return BADARG;  /* message should be user defined */
            else
                strcpy(rply_line, "250-");

            break;  /* end of R250E */

        case R251:  /* (after RCPT) user not local; will forward to... */
            if (NULL == msg) {
                strcpy(rply_line, "251 User not local; "
                                  "will forward to next hop\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "251 ");

            break;  /* end of R251 */

        case R252:  /* cannot VRFY user, but will accept message
                       and attempt delivery */
            if (NULL == msg) {
                strcpy(rply_line, "252 cannot VRFY user, but will "
                                  "accept message and attempt delivery\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "252 ");

            break;  /* end of R252 */

        case R354:  /* (after DATA) start mail input; end with <CRLF>.<CRLF> */
            if (NULL == msg) {
                strcpy(rply_line, "354 Start mail input; "
                                  "end with <CRLF>.<CRLF>\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "354 ");

            break;  /* end of R354 */

        case R450:  /* requested mail action not taken: mailbox unavailable */
            if (NULL == msg) {
                strcpy(rply_line, "450 Requested mail action not taken: "
                                  "mailbox unavailable\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "450 ");

            break;  /* end of R450 */

        case R451:  /* requested action aborted: local error in processing */
            if (NULL == msg) {
                strcpy(rply_line, "451 Requested action aborted: "
                                  "local error in processing\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "451 ");

            break;  /* end of R451 */

        case R452:  /* requested action not taken:
                       insufficient system storage */
            if (NULL == msg) {
                strcpy(rply_line, "452 requested action not taken: "
                                  "insufficient system storage\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "452 ");

            break;  /* end of R452 */

        case R455:  /* server unable to accommodate parameters */
            if (NULL == msg) {
                strcpy(rply_line, "455 Server unable to accommodate "
                                  "parameters\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "455  ");

            break;  /* end of R455 */

        case R500:  /* syntax error, command unrecognized */
            if (NULL == msg) {
                strcpy(rply_line, "500 Syntax error, command unrecognized\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "500 ");

            break;  /* end of R500 */

        case R502:  /* (after EHLO) command not implemented */
            if (NULL == msg) {
                strcpy(rply_line, "502 Command not implemented\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "502 ");

            break;  /* end of R502 */

        case R503:  /* bad sequence of commands */
            if (NULL == msg) {
                strcpy(rply_line, "503 Bad sequence of commands\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "503 ");

            break;  /* end of R503 */

        case R504:  /* (after HELO/EHLO) command parameter not implemented */
            if (NULL == msg) {
                strcpy(rply_line, "504 Command parameter not implemented\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "504 ");

            break;  /* end of R504 */

        case R550:  /* requested action not taken: mailbox unavailable */
            if (NULL == msg) {
                strcpy(rply_line, "550 Requested action not taken: "
                                  "mailbox unavailable\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "550 ");

            break;  /* end of R550 */

        case R551:  /* (after RCPT) user not local; please try <forward-path> */
            if (NULL == msg) {
                strcpy(rply_line, "551 User not local; "
                                  "please try next hop\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "551 ");

            break;  /* end of R551 */

        case R552:  /* requested mail action aborted:
                       exceeded storage allocation */
            if (NULL == msg) {
                strcpy(rply_line, "552 Requested mail action aborted: "
                                  "exceeded storage allocation\r\n");
                ret = writen(sockfd, rply_line, slen =strlen(rply_line));
            }
            else
                strcpy(rply_line, "552 ");

            break;  /* end of R552 */

        case R553:  /* (after RCPT) requested action not taken:
                       mailbox name not allowed */
            if (NULL == msg) {
                strcpy(rply_line, "553 Requested action not taken: "
                                  "mailbox name not allowed\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
                return 0;
            }
            else
                strcpy(rply_line, "553 ");

            break;  /* end of R553 */

        case R554:  /* transaction failed */
            if (NULL == msg) {
                strcpy(rply_line, "554 Transaction failed\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "554 ");

            break;  /* end of R554 */

        case R555:  /* MAIL FROM/RCPT TO parameters not recognized or
                       not implemented */
            if (NULL == msg) {
                strcpy(rply_line, "555 MAIL FROM/RCPT TO parameters not "
                                  "recognized or not implemented\r\n");
                ret = writen(sockfd, rply_line, slen = strlen(rply_line));
            }
            else
                strcpy(rply_line, "555 ");

            break;  /* end of R555 */

        default:
            return BADARG;  /* no such command */
    }

    /* user defined message */
    if (NULL != msg) {
        pos = 4;
        if (msg_len > RPLY_MAXLEN)
            return BADARG;
        strncpy(rply_line+pos, msg, min(msg_len, RPLY_MAXLEN));
        pos += min(RPLY_MAXLEN, msg_len);
        strcpy(rply_line+pos, "\r\n");
        ret = writen(sockfd, rply_line, slen = strlen(rply_line));
    }

    if (ret < 0 || (size_t) ret != slen)
        return SENDERROR;
    else
        return 0;
}

/* buf_read - buffered read from socket, used by functions          *
 *            receiving SMTP Commands and Replies.                  *
 *            WARNING! Function buf_read() is not safe for threads, *
 *            it uses static variables inside functions.            */
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

/* smtp_readline - read one line (ended with CRLF) from socket, on which  *
 *                 active SMTP session is running                         */

/* readline states */
#define RL_START        0
#define RL_CR_READ      1

ssize_t smtp_readline (int fd, void *vptr, size_t maxlen)
{
    int rc, state;
    unsigned int n;
    char c, *ptr;

    ptr = vptr;
    state = RL_START;

    for (n = 1; n < maxlen; n++) {
        if ( (rc = buf_read(fd, &c)) == 1) {
            *ptr++ = c;

            if (RL_CR_READ == state) {
                if (c == '\n') {
                    ptr -= 2;   /* CRLF is not stored */
                    break;
                }
                else
                    state = RL_START;
            }

            if (c == '\r')
                state = RL_CR_READ; /* CR read; in next turn check for LF */
        }
        else if (rc == 0) {
            if (n == 1)
                return 0;   /* EOF, no data read */
            else
                break;      /* EOF, some data was read */
        }
        else
            return -1;      /* error, errno set by read() */
    }

    *ptr = 0;   /* null terminate like fgets() */

    return n;
}

/* smtp_recv_command - receive SMTP Command from active SMTP session, *
 *                     functions used by SMTP servers.                */
int smtp_recv_command (int sockfd, struct smtp_command *cmd)
{
    size_t len;
    char line[LINE_MAXLEN];

    if (NULL == cmd)
        return NULLPTR; /* NULL pointer dereference */

    if (smtp_readline(sockfd, line, LINE_MAXLEN) <= 0)
        return RCVERROR;   /* receiving error: no data to read or error */

    bzero(cmd->data, sizeof(cmd->data));

    if ((len = strlen(line)) < 4) {
        cmd->code = 0;
        return RCV_NKNOWNCMD;   /* unknown command received */
    }

    if (0 == strncasecmp("HELO ", line, 5)) {
        cmd->code = HELO;
        if (len < 6)
            return RCV_BADPARAM;    /* bad parameter */
        strncpy(cmd->data, line+5, DOMAIN_MAXLEN);
    }
    else if (0 == strncasecmp("EHLO ", line, 5)) {
        cmd->code = EHLO;
        if (len < 6)
            return RCV_BADPARAM;    /* bad parameter */
        strncpy(cmd->data, line+5, DOMAIN_MAXLEN);
    }
    else if (0 == strncasecmp("MAIL FROM:", line, 10)) {
        cmd->code = MAIL;
        if (len < 15 || '<' != line[10] || '>' != line[len-1])
            return RCV_BADPARAM;    /* bad parameter */
        line[len-1] = '\0';
        strncpy(cmd->data, line+11, ADDR_MAXLEN);
    }
    else if (0 == strncasecmp("RCPT TO:", line, 8)) {
        cmd->code = RCPT;
        if (len < 13 || '<' != line[8] || '>' != line[len-1])
            return RCV_BADPARAM;    /* bad parameter */
        line[len-1] = '\0';
        strncpy(cmd->data, line+9, ADDR_MAXLEN);
    }
    else if (4 == len && 0 == strncasecmp("DATA", line, 4))
        cmd->code = DATA;
    else if (4 == len && 0 == strncasecmp("RSET", line, 4))
        cmd->code = RSET;
    else if (0 == strncasecmp("VRFY ", line, 5))
        cmd->code = VRFY;
    else if ( (4 == len && 0 == strncasecmp("NOOP", line, 4))
            || 0 == strncasecmp("NOOP ", line, 5) )
        cmd->code = NOOP;
    else if (4 == len && 0 == strncasecmp("QUIT", line, 4))
        cmd->code = QUIT;
    else {
        cmd->code = 0;
        strncpy(cmd->data, line, LINE_MAXLEN);

        return RCV_NKNOWNCMD;   /* unknown command received */
    }

    return 0;
}

/* smtp_recv_reply - receive SMTP Reply from active SMTP session, *
 *                   functions used by SMTP client.               */
int smtp_recv_reply (int sockfd, struct smtp_reply *rply)
{
    char line[LINE_MAXLEN];

    if (NULL == rply)
        return NULLPTR; /* NULL pointer dereference */

    if (smtp_readline(sockfd, line, LINE_MAXLEN) <= 0)
        return RCVERROR;   /* receiving error: no data to read or error */

    bzero(rply, sizeof(*rply));

    if (0 == strncmp("220", line, 3))
        rply->code = R220;
    else if (0 == strncmp("221", line, 3))
        rply->code = R221;
    else if (0 == strncmp("250-", line, 4))
        rply->code = R250E;
    else if (0 == strncmp("250", line, 3))
        rply->code = R250;
    else if (0 == strncmp("251", line, 3))
        rply->code = R251;
    else if (0 == strncmp("252", line, 3))
        rply->code = R252;
    else if (0 == strncmp("354", line, 3))
        rply->code = R354;
    else if (0 == strncmp("450", line, 3))
        rply->code = R450;
    else if (0 == strncmp("451", line, 3))
        rply->code = R451;
    else if (0 == strncmp("452", line, 3))
        rply->code = R452;
    else if (0 == strncmp("455", line, 3))
        rply->code = R455;
    else if (0 == strncmp("500", line, 3))
        rply->code = R500;
    else if (0 == strncmp("502", line, 3))
        rply->code = R502;
    else if (0 == strncmp("503", line, 3))
        rply->code = R503;
    else if (0 == strncmp("504", line, 3))
        rply->code = R504;
    else if (0 == strncmp("550", line, 3))
        rply->code = R550;
    else if (0 == strncmp("551", line, 3))
        rply->code = R551;
    else if (0 == strncmp("552", line, 3))
        rply->code = R552;
    else if (0 == strncmp("553", line, 3))
        rply->code = R553;
    else if (0 == strncmp("554", line, 3))
        rply->code = R554;
    else if (0 == strncmp("555", line, 3))
        rply->code = R555;
    else {
        rply->code = 0;
        strncpy(rply->msg, line, RPLY_MAXLEN);

        return RCV_NKNOWNRPLY;  /* unknown command received */
    }

    /* copy message if there is one */
    if (strlen(line) > 4)
        strncpy(rply->msg, line+4, RPLY_MAXLEN);

    return 0;
}

