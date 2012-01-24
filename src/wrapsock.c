/**
 * File:         src/wrapsock.c
 * Description:  Socket wrapper functions.
 * Author:       Tomasz Pieczerak (tphaster) (based on W. Richard Stevens code)
 */

#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include "system.h"

void Bind (int fd, const struct sockaddr *sa, socklen_t salen)
{
    if (bind(fd, sa, salen) < 0)
        err_sys("bind error");
}

void Connect (int fd, const struct sockaddr *sa, socklen_t salen)
{
    if (connect(fd, sa, salen) < 0)
        err_sys("connect error");
}

void Listen (int fd, int backlog)
{
    char *ptr;

    /* you can override 2nd argument with environment variable */
    if ( (ptr = getenv("LISTENQ")) != NULL)
        backlog = atoi(ptr);

    if (listen(fd, backlog) < 0)
        err_sys("listen error");
}

int Socket (int family, int type, int protocol)
{
    int n;

    if ( (n = socket(family, type, protocol)) < 0)
        err_sys("socket error");
    return n;
}

