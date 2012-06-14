/**
 * smtp-test-2 (server) - interactive SMTP server, replies written by hand
 *                        (not checked), no server logic
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
    }
}

void service (int sockfd)
{
    ssize_t n;
    size_t len, data_size;
    char line[100];
    char *data;

    /* send 220 reply (service ready) */
    smtp_send_reply(sockfd, R220, NULL, 0);

    for (;;) {
        /* receive command from client */
        if ( (n = smtp_readline(sockfd, line, 100)) == 0)
            return;
        /* print it */
        printf("C: |%s|\n", line);

        /* get a reply from stdin */
        fgets(line, 100, stdin);
        len = strlen(line);
        line[len-1] = '\0';
        len--;

        printf("R: |%s|\n", line);

        line[len++] = '\r';
        line[len++] = '\n';
        line[len++] = '\0';

        /* send reply to client */
        writen(sockfd, line, len-1);

        /* if reply was 354, receive mai data until .CRLF */
        if (0 == strncmp(line, "354", 3)) {
            printf("Mail data recv:\n");
            n = smtp_recv_mail_data(sockfd, &data, &data_size);
            data[n+1] = '\0';
            printf("%s", data);
            printf("END OF MAIL\n");
            smtp_send_reply(sockfd, R250, NULL, 0);
            printf("R: |250 OK|\n");
            free(data);
        }
    }
}

