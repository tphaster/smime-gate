/**
 * File:         src/signal.c
 * Description:  Signal wrapper functions.
 * Author:       Tomasz Pieczerak (tphaster)
 *               (based on W. Richard Stevens "UNIX Network Programming")
 */

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "system.h"

Sigfunc *signal (int signo, Sigfunc *func)
{
    struct sigaction    act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (signo == SIGALRM) {
#ifdef  SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;   /* SunOS 4.x */
#endif
    }
    else {
#ifdef  SA_RESTART
        act.sa_flags |= SA_RESTART;     /* SVR4, 44BSD */
#endif
    }
    if (sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;

    return oact.sa_handler;
}

Sigfunc *Signal (int signo, Sigfunc *func)
{
    Sigfunc *sigfunc;

    if ( (sigfunc = signal(signo, func)) == SIG_ERR)
        err_sys("signal error");
    return sigfunc;
}

void sig_chld (int signo __attribute__((__unused__)))
{
    pid_t pid;
    int stat;

    while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        --sproc_counter;
#ifdef DEBUG
        printf(DPREF "child %d terminated", pid);
#endif
    }
    return;
}

