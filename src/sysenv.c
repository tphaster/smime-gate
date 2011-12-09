/**
 * File:         src/sysenv.c
 * Description:  Various system-dependent functions needed for
 *               proper execution in the Unix-like environment.
 * Author:       Tomasz Pieczerak (tphaster) (based on Richard W. Stevens code)
 */

#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "system.h"

#define MAXFD   64

extern int deamon_proc;

void deamonize (const char *pname, int facility)
{
    int i;
    pid_t pid;

    if ( (pid = Fork()) != 0)
        exit(0);

    setsid();

    Signal(SIGHUP, SIG_IGN);
    if ( (pid = Fork()) != 0)
        exit(0);

    deamon_proc = 1;
    chdir("/");
    umask(0);

    for (i = 0; i < MAXFD; ++i)
        close(i);

    openlog(pname, LOG_PID, facility);
}

