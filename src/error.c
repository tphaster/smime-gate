/**
 * File:         src/error.c
 * Description:  Error reporting functions.
 * Author:       Tomasz Pieczerak (tphaster) (based on W. Richard Stevens code)
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "include/system.h"

int daemon_proc;    /* set nonzero by daemonize() */

static void err_doit (int, int, const char *, va_list);


/* err_ret - nonfatal error related to a system call,
 *           print a message and return.
 */
void err_ret (const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, LOG_INFO, fmt, ap);
    va_end(ap);
    return;
}

/* err_sys - fatal error related to a system call,
 *           print a message and terminate.
 */
void err_sys (const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, LOG_ERR, fmt, ap);
    va_end(ap);
    exit(1);
}

/* err_dump - fatal error related to a system call,
 *            print a message, dump core, and terminate.
 */
void err_dump (const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, LOG_ERR, fmt, ap);
    va_end(ap);
    abort();    /* dump core and terminate */
    exit(1);    /* shouldn't get here */
}

/* err_msg - nonfatal error unrelated to a system call,
 *           print a message and return.
 */
void err_msg (const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(0, LOG_INFO, fmt, ap);
    va_end(ap);
    return;
}

/* err_quit - fatal error unrelated to a system call,
 *            print a message and terminate.
 */

void err_quit (const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(0, LOG_ERR, fmt, ap);
    va_end(ap);
    exit(1);
}

/* err_doit - print a message and return to caller,
 *            caller specifies "errnoflag" and "level".
 */

static void err_doit (int errnoflag, int level, const char *fmt, va_list ap)
{
    int errno_save, n;
    char buf[MAXLINE];

    errno_save = errno;     /* value caller might want printed */
#ifdef  HAVE_VSNPRINTF
    vsnprintf(buf, sizeof(buf), fmt, ap);   /* this is safe */
#else
    vsprintf(buf, fmt, ap);                 /* this is not safe */
#endif
    n = strlen(buf);

    if (errnoflag)
        snprintf(buf+n, sizeof(buf)-n, ": %s", strerror(errno_save));

    strcat(buf, "\n");

    if (daemon_proc) {
        syslog(level, buf);
    }
    else {
        fflush(stdout);     /* in case stdout and stderr are the same */
        fputs(buf, stderr);
        fflush(stderr);
    }
    return;
}

