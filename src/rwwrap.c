/**
 * File:        src/rwwrap.c
 * Description: Read/write from file descriptor wrapper functions.
 * Author:      Tomasz Pieczerak (tphaster)
 *              (based on W. Richard Stevens "UNIX Network Programming")
 */

#include <errno.h>
#include <unistd.h>
#include "system.h"

/* readn - read "n" bytes from a descriptor. */
ssize_t readn (int fd, void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;  /* and call read() again */
            else
                return(-1);
        }
        else if (nread == 0)
            break;          /* EOF */

        nleft -= nread;
        ptr   += nread;
    }
    return (n - nleft);   /* return >= 0 */
}

/* writen - write "n" bytes to a descriptor. */
ssize_t writen (int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (errno == EINTR)
                nwritten = 0;   /* and call write() again */
            else
                return -1;      /* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return n;
}

