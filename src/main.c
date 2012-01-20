
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "config.h"
#include "system.h"
#include "smime-gate.h"

/*** Global Variables***/
struct config conf;     /* global configuration */

/* main - smtp-gate main function */
int main (int argc, char **argv)
{
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    void sig_chld(int);

    /* parse command line arguments and load config */
    parse_args(argc, argv);
    load_config();

#ifdef DEBUG
    print_config();
#endif

    /* become a daemon, if it is set so*/
    if (0 != conf.daemon)
        daemonize(conf.prog_name, 0);

    /* create listening socket for SMTP Server */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* bind local address to listening socket */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = conf.smtp_port;

    Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

    /* start listening for client's connections */
    Listen(listenfd, LISTENQ);

    /* set appropriate handler for SIGCHLD */
    Signal(SIGCHLD, sig_chld);

    /* SMTP Server's main loop */
    for (;;) {
        clilen = sizeof(cliaddr);
        if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
            if (errno == EINTR)
                continue;   /* back to for() */
            else
                err_sys("accept error");
        }

        if ( (childpid = Fork()) == 0) {    /* child process */
            Close(listenfd);            /* close listening socket */
            smime_gate_service(connfd);    /* process the request */
            exit(0);
        }
        Close(connfd);  /* parent closes connected socket */
    }
}   /* end of main() */

