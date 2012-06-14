/**
 * smime-gate-test (server) - full automated SMTP server, saves received
 *                            mail objects to files
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include "system.h"
#include "smtp.h"

#define SMTP_PORT   5780

void service (int sockfd);
ssize_t smtp_recv_mail_data (int sockfd, char **buf_ptr, size_t *buf_size);


int main (void)
{
    int listenfd, connfd;
    socklen_t clilen;
    pid_t childpid;
    struct sockaddr_in cliaddr, servaddr;
    void sig_chld(int);

    /* create listen socket */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(SMTP_PORT);

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
    Listen(listenfd, LISTENQ);
    Signal(SIGCHLD, sig_chld);

    /* accept clients */
    for (;;) {
        clilen = sizeof(cliaddr);
        if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
            if (errno == EINTR)
                continue;
            else
                err_sys("accept error");
        }

        /* fork and run server service */
        if ( (childpid = Fork()) == 0) {
            Close(listenfd);
            service(connfd);
            exit(0);
        }
        Close(connfd);
    }
}

void service (int sockfd)
{
    struct mail_object mail;
    int i = 0;
    int srv = SMTP_SRV_NEW;
    int p = getpid();
    char file[30];

    snprintf(file, 30, "rcvd_mail%d-%d", p, i);

    /* receive mail object(s) from client */
    while (0 == smtp_recv_mail(sockfd, &mail, file, srv)) {
        printf("Received mail, saved in %s\n", file);
        ++i;
        snprintf(file, 30, "rcvd_mail%d-%d", p, i);
        free_mail_object(&mail);

        srv = SMTP_SRV_NXT;
    }
}

