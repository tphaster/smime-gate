/**
 * smtp-test-4 (server) - automated SMTP server (using high-level smtp_recv_mail())
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/in.h>
#include "system.h"
#include "smtp-lib.h"
#include "smtp.h"

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

        exit(1);
    }
}

void service (int sockfd)
{
    unsigned int i;
    int ret;
    struct mail_object mail;
    int state = SMTP_SRV_NEW;

    /* receive mail from  client */
    while (0 ==
            (ret = smtp_recv_mail(sockfd, &mail, "mail001-t", state)))
    {
        printf("Mail received!\n");

        /* print mail object */
        printf("FROM: %s\n", mail.mail_from);
        for (i = 0; i < mail.no_rcpt; ++i)
            printf("TO:   %s\n", mail.rcpt_to[i]);
        mail.data[mail.data_size] = '\0';
        printf("DATA:\n%sEND DATA\n", mail.data);

        free_mail_object(&mail);
        state = SMTP_SRV_NXT;
    }

    printf("Client quits or ERROR (%d)\n", ret);
}

