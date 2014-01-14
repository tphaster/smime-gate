/**
 * File:        src/main.c
 * Description: S/MIME Gate main function.
 * Author:      Tomasz Pieczerak (tphaster)
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "config.h"
#include "system.h"
#include "smime-gate.h"

/** Global Variables **/
struct config conf;                         /* global configuration */
volatile sig_atomic_t sigchld_notify = 0;   /* SIGCHLD notifier */
int sproc_counter = 0;                      /* forked subprocess counter */
pthread_mutex_t sproc_mutex = PTHREAD_MUTEX_INITIALIZER;
                                            /* mutex for subprocess counter */

/* S/MIME Gate main function */
int main (int argc, char **argv)
{
    int listenfd, connfd;
    pid_t childpid, unsentpid;
    pthread_t guard_id;
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
    if (0 != conf.daemon) {
        daemonize(conf.prog_name, 0);
#ifdef DEBUG
        printf(DPREF "smime-gate becomes a daemon\n");
#endif
    }

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
#ifdef DEBUG
    printf(DPREF "listening on port %d, address %s",
            ntohs(conf.smtp_port), inet_ntoa(servaddr.sin_addr));
#endif

    /* set appropriate handler for SIGCHLD */
    Signal(SIGCHLD, sig_chld);

    /* start unsent service */
#ifdef DEBUG
        printf(DPREF "starting unsent service\n");
#endif
    if ( (unsentpid = Fork()) == 0) {
        unsent_service();
        exit(0);
    }

    /* start child process guard */
#ifdef DEBUG
        printf(DPREF "starting child process guard\n");
#endif
    if (0 != pthread_create(&guard_id, NULL, child_process_guard, NULL) )
        err_sys("pthread_create error");

    /* SMTP Server's main loop */
    for (;;) {
        clilen = sizeof(cliaddr);
        if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
#ifdef DEBUG
            printf(DPREF "accept() call returned non-zero status: %d\n", errno);
#endif
            if (errno == EINTR)
                continue;   /* back to for() */
            else
                err_sys("accept error");
        }

        /* check whether subprocesses limit is not exceeded  */
        if (sproc_counter < MAXSUBPROC) {
#ifdef DEBUG
            printf(DPREF "incomming connection, forking a child...\n");
#endif
            pthread_mutex_lock(&sproc_mutex);
            if ( (childpid = Fork()) == 0) {    /* child process */
                Close(listenfd);                /* close listening socket */
                smime_gate_service(connfd);     /* process the request */
                exit(0);
            }
            ++sproc_counter;
            pthread_mutex_unlock(&sproc_mutex);
        }
        else
            err_msg("subprocesses limit exceeded, connection refused");

        Close(connfd);  /* parent closes connected socket */
    }
}   /* end of main() */

