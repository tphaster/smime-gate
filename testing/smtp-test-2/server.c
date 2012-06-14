/**
 * smtp-test-3 (server) - test functions for sending replies to client and
 *                        receiving commands from him
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include "system.h"
#include "smtp-lib.h"

#define SMTP_PORT   5780

void service (int sockfd);
ssize_t smtp_recv_mail_data (int sockfd, char **buf_ptr, size_t *buf_size);


int main (void)
{
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    /* create listen socket */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SMTP_PORT);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);

    /* accept clients */
    for (;;) {
        clilen = sizeof(cliaddr);
        if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
            if (errno == EINTR)
                continue;
            else
                err_sys("accept error");
        }

        /* run server service */
        service(connfd);
        Close(connfd);

        exit(0);
    }
}

void service (int sockfd)
{
    int i;
    struct smtp_command cmd;
    char msg[] = "Message";

    /* send various replies */
    smtp_send_reply(sockfd, R220, NULL, 0);
    smtp_send_reply(sockfd, R221, NULL, 0);
    smtp_send_reply(sockfd, R250, NULL, 0);
    smtp_send_reply(sockfd, R250E, "8BITMIME", 8);
    smtp_send_reply(sockfd, R251, NULL, 0);
    smtp_send_reply(sockfd, R252, NULL, 0);
    smtp_send_reply(sockfd, R354, NULL, 0);
    smtp_send_reply(sockfd, R553, NULL, 0);
    smtp_send_reply(sockfd, R554, msg, 7);
    smtp_send_reply(sockfd, R555, msg, 0);

    /* receive commands from client */
    for (i = 0; i < 9; ++i) {
        if (0 == smtp_recv_command(sockfd, &cmd))
            printf("C: %d |%s|\n", cmd.code, cmd.data);
        else
            printf("R: ERROR\n");
    }
}

