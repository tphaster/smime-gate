/**
 * smtp-test-3 (client) - test functions for sending commands to server and
 *                        receiving replies from him
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "system.h"
#include "smtp.h"
#include "smtp-lib.h"
#include "smtp-types.h"

#define SMTP_PORT   5780

void str_cli (int sockfd);


int main (int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;

    if (argc != 2)
        err_quit("usage: client <IPaddress>");

    /* create socket */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SMTP_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    /* connect to server */
    Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

    /* run client */
    str_cli(sockfd);

    exit(0);
}

void str_cli (int sockfd)
{
    int i;
    struct mail_object mail;
    struct smtp_reply rply;
    char sndr[] = "sender@example.org";
    char rcpt1[] = "rcpt1@example.org";
    char rcpt2[] = "rcpt2@example.org";
    char *rcpts[2];

    /* assemble mail object */
    mail.mail_from = sndr;
    mail.rcpt_to = rcpts;
    mail.rcpt_to[0] = rcpt1;
    mail.rcpt_to[1] = rcpt2;
    mail.no_rcpt = 2;

    /* receive replies from server */
    for (i = 0; i < 10; ++i) {
        if (0 == smtp_recv_reply(sockfd, &rply))
            printf("R: %d |%s|\n", rply.code, rply.msg);
        else
            printf("R: ERROR\n");
    }

    /* send various commands */
    smtp_send_command(sockfd, EHLO, NULL);
    smtp_send_command(sockfd, HELO, NULL);
    smtp_send_command(sockfd, MAIL, &mail);
    smtp_send_command(sockfd, RCPT_N(0), &mail);
    smtp_send_command(sockfd, RCPT_N(1), &mail);
    smtp_send_command(sockfd, DATA, NULL);
    smtp_send_command(sockfd, RSET, NULL);
    smtp_send_command(sockfd, NOOP, NULL);
    smtp_send_command(sockfd, QUIT, NULL);
}

