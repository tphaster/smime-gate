/**
 * File:          include/system.h
 * Description:   Header file for system functions and definitions.
 * Author:        Tomasz Pieczerak (tphaster)
 */

#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <poll.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#ifdef DEBUG
#include <stdio.h>
#include <arpa/inet.h>
#endif

/** Typedefs **/
typedef void    Sigfunc(int);   /* for signal handlers */

/** Macros **/
#define min(a,b)    ((a) < (b) ? (a) : (b))
#define max(a,b)    ((a) > (b) ? (a) : (b))

/* Define bzero() as a macro, if it's not in standard C library. */
#ifndef HAVE_BZERO
#define bzero(ptr,n)    memset(ptr, 0, n)
#endif

#define SA      struct sockaddr

/** Constants **/
#define MAXLINE         4096    /* maximum text line length */
#define MAXSOCKADDR      128    /* maximum socket address structure size */
#define BUFFSIZE        8192    /* buffer size for reads and writes */
#define LISTENQ         1024    /* default value of backlog in listen() */
#define MAXSUBPROC       200    /* maximum number of forked subprocesses */
#define FNMAXLEN          48    /* filename maximum length */
#define MAILBUF           10    /* mail buffer size */
#define CMDMAXLEN        512    /* command maximum length */
#define UNSENT_SLEEP      60    /* (sec) how often try to resend unsent mails */
#define SIGCHLD_SLEEP     50    /* SIGCHLD handling interval */


/** Externs **/
extern volatile sig_atomic_t sigchld_notify; /* SIGCHLD notifier */
extern int sproc_counter;                    /* forked subprocess counter */
extern pthread_mutex_t sproc_mutex;          /* mutex for subprocess counter */


/** Functions **/

/* Error reporting functions */
void err_ret (const char *fmt, ...);
void err_sys (const char *fmt, ...);
void err_msg (const char *fmt, ...);
void err_quit (const char *fmt, ...);

/* Standard Unix calls wrapper functions */
void *Calloc (size_t n, size_t size);
void Close (int fd);
pid_t Fork(void);
void *Malloc (size_t size);

/* Socket wrapper functions */
void Bind (int fd, const struct sockaddr *sa, socklen_t salen);
void Connect (int fd, const struct sockaddr *sa, socklen_t salen);
void Listen (int fd, int backlog);
int Socket (int family, int type, int protocol);

/* Read/write functions */
ssize_t readn (int fd, void *vptr, size_t n);
ssize_t writen (int fd, const void *vptr, size_t n);

/* System environment functions */
void daemonize (const char *pname, int facility);
Sigfunc *Signal (int signo, Sigfunc *func);

#endif  /* __SYSTEM_H */

