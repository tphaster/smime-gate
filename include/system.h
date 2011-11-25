/**
 * File:          include/system.h
 * Description:   Header file for system functions.
 * Author:        Tomasz Pieczerak (tphaster)
 */

#include <poll.h>
#include <signal.h>
#include <sys/socket.h>

/* Typedefs */
typedef void    Sigfunc(int);   /* for signal handlers */

/* Useful macros */
#define min(a,b)    ((a) < (b) ? (a) : (b))
#define max(a,b)    ((a) > (b) ? (a) : (b))

/* Miscellaneous constants */
#define MAXLINE         4096    /*  max text line length */
#define MAXSOCKADDR      128    /*  max socket address structure size */
#define BUFFSIZE        8192    /*  buffer size for reads and writes */

/* Error reporting functions */
void err_ret (const char *fmt, ...);
void err_sys (const char *fmt, ...);
void err_dump (const char *fmt, ...);
void err_msg (const char *fmt, ...);
void err_quit (const char *fmt, ...);

/* Standard Unix calls wrapper functions */
void *Calloc (size_t n, size_t size);
void Close (int fd);
void Dup2 (int fd1, int fd2);
int Fcntl (int fd, int cmd, int arg);
void Gettimeofday (struct timeval *tv, void *foo);
int Ioctl (int fd, int request, void *arg);
pid_t Fork(void);
void *Malloc (size_t size);
void Mktemp (char *template);
void *Mmap (void *addr, size_t len, int prot, int flags, int fd, off_t offset);
int Open (const char *pathname, int oflag, mode_t mode);
void Pipe (int *fds);
ssize_t Read (int fd, void *ptr, size_t nbytes);
void Sigaddset (sigset_t *set, int signo);
void Sigdelset (sigset_t *set, int signo);
void Sigemptyset (sigset_t *set);
void Sigfillset (sigset_t *set);
int Sigismember (const sigset_t *set, int signo);
void Sigpending (sigset_t *set);
void Sigprocmask (int how, const sigset_t *set, sigset_t *oset);
char *Strdup (const char *str);
long Sysconf (int name);
#ifdef  HAVE_SYS_SYSCTL_H
void Sysctl (int *name, u_int namelen, void *oldp, size_t *oldlenp, void *newp,
             size_t newlen);
#endif
void Unlink (const char *pathname);
pid_t Wait (int *iptr);
pid_t Waitpid (pid_t pid, int *iptr, int options);
void Write (int fd, void *ptr, size_t nbytes);

/* Signal wrapper functions */
Sigfunc *signal (int signo, Sigfunc *func);
Sigfunc *Signal (int signo, Sigfunc *func);

/* Socket wrapper functions */
int Accept (int fd, struct sockaddr *sa, socklen_t *salenptr);
void Bind (int fd, const struct sockaddr *sa, socklen_t salen);
void Connect (int fd, const struct sockaddr *sa, socklen_t salen);
void Getpeername (int fd, struct sockaddr *sa, socklen_t *salenptr);
void Getsockname (int fd, struct sockaddr *sa, socklen_t *salenptr);
void Getsockopt (int fd, int level, int optname, void *optval,
                 socklen_t *optlenptr);
int Isfdtype (int fd, int fdtype);
void Listen (int fd, int backlog);
#ifdef  HAVE_POLL
int Poll (struct pollfd *fdarray, unsigned long nfds, int timeout);
#endif
ssize_t Recv (int fd, void *ptr, size_t nbytes, int flags);
ssize_t Recvfrom (int fd, void *ptr, size_t nbytes, int flags,
                  struct sockaddr *sa, socklen_t *salenptr);
ssize_t Recvmsg (int fd, struct msghdr *msg, int flags);
int Select (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
            struct timeval *timeout);
void Send (int fd, const void *ptr, size_t nbytes, int flags);
void Sendto (int fd, const void *ptr, size_t nbytes, int flags,
             const struct sockaddr *sa, socklen_t salen);
void Sendmsg (int fd, const struct msghdr *msg, int flags);
void Setsockopt (int fd, int level, int optname, const void *optval,
                 socklen_t optlen);
void Shutdown (int fd, int how);
int Sockatmark (int fd);
int Socket (int family, int type, int protocol);
void Socketpair (int family, int type, int protocol, int *fd);

