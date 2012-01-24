/**
 * File:         src/wrapunix.c
 * Description:  Standard Unix calls wrapper functions.
 * Author:       Tomasz Pieczerak (tphaster) (based on W. Richard Stevens code)
 */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <linux/sysctl.h>
#include "system.h"

void *Calloc (size_t n, size_t size)
{
    void *ptr;

    if ( (ptr = calloc(n, size)) == NULL)
        err_sys("calloc error");
    return ptr;
}

void Close (int fd)
{
    if (close(fd) == -1)
        err_sys("close error");
}

pid_t Fork(void)
{
    pid_t pid;

    if ( (pid = fork()) == -1)
        err_sys("fork error");
    return pid;
}

void *Malloc (size_t size)
{
    void *ptr;

    if ( (ptr = malloc(size)) == NULL)
        err_sys("malloc error");
    return ptr;
}

