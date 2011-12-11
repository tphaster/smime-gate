/**
 * File:        src/rwwrap.c
 * Description: Read/write from file descriptor wrapper functions.
 * Author:      Tomasz Pieczerak (tphaster)
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

ssize_t Readn (int fd, void *ptr, size_t nbytes)
{
    ssize_t n;

    if ( (n = readn(fd, ptr, nbytes)) < 0)
        err_sys("readn error");
    return n;
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
                return(-1);     /* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return n;
}

void Writen (int fd, void *ptr, size_t nbytes)
{
    if (writen(fd, ptr, nbytes) != (ssize_t) nbytes)
        err_sys("writen error");
}

static ssize_t my_read (int fd, char *ptr)
{
    static int read_cnt = 0;
    static char *read_ptr;
    static char read_buf[MAXLINE];

    if (read_cnt <= 0) {
again:
        if ( (read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
            if (errno == EINTR)
                goto again;
            return -1;
        }
        else if (read_cnt == 0)
            return 0;
        read_ptr = read_buf;
    }

    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}

ssize_t readline (int fd, void *vptr, size_t maxlen)
{
    int rc;
    size_t n;
    char c, *ptr;

    ptr = vptr;
    for (n = 1; n < maxlen; n++) {
        if ( (rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;      /* newline is stored, like fgets() */
        }
        else if (rc == 0) {
            if (n == 1)
                return 0;   /* EOF, no data read */
            else
                break;      /* EOF, some data was read */
        }
        else
            return -1;      /* error, errno set by read() */
    }

    *ptr = 0;   /* null terminate like fgets() */
    return n;
}

ssize_t Readline (int fd, void *ptr, size_t maxlen)
{
    ssize_t n;

    if ( (n = readline(fd, ptr, maxlen)) < 0)
        err_sys("readline error");
    return n;
}

