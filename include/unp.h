#ifndef _UNP_H
#define _UNP_H

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <assert.h>
#include <limits.h>
#include <netdb.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <netinet/tcp.h>

#include <pthread.h>

#ifdef	HAVE_POLL_H
# include	<poll.h>		/* for convenience */
#endif

#ifdef	HAVE_SYS_EVENT_H
# include	<sys/event.h>	/* for kqueue */
#endif


#define MAXLINE 100
#define	LISTENQ 1024
#define OPEN_MAX 50

typedef struct sockaddr SA;

void   err_quit(const char *fmt, ...);
void   err_ret(const char *fmt, ...);
void   err_sys(const char *fmt, ...);
void   err_exit(int error, const char *fmt, ...);
void   err_dump(const char *fmt, ...);
void   err_msg(const char *fmt, ...);
void   err_quit(const char *fmt, ...);

int    Socket(int domain, int type, int protocol);
void   Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
void   Listen(int sockfd, int backlog);
int    Accept(int sockfd, struct sockaddr *cliaddr, socklen_t * addrlen);
void   Connect(int fd, const struct sockaddr *sa, socklen_t salen);
size_t Recv(int fd, void *ptr, size_t nbytes, int flags);
size_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags, struct sockaddr *sa, socklen_t *salenptr);
size_t Recvmsg(int fd, struct msghdr *msg, int flags);
int    Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
void   Send(int fd, const void *ptr, size_t nbytes, int flags);
void   Shutdown(int fd, int how);
void   Write(int fd, void *ptr, size_t nbytes);
size_t Read(int fd, void *ptr, size_t nbytes);
void   Sendto(int fd, const void *ptr, size_t nbytes, int flags, const struct sockaddr *sa, socklen_t salen);

struct addrinfo *Host_serv(const char *host, const char *serv, int family, int socktype);
int     Tcp_connect(const char *hostname, const char *service);
int 	Tcp_listen(const char *hostname, const char * service, socklen_t *addrlenp);
int 	Udp_client(const char *hostname, const char *service, SA **saptr, socklen_t *lenp);
int 	Udp_connect(const char *hostname, const char *service);
int 	Udp_server(const char *hostname, const char *service, socklen_t *addrlenp);


void   Inet_pton(int family, const char *strptr, void * addrptr);
const char * Inet_ntop(int family, const void * addrptr, char * strptr, size_t len);
int    Connect_nonb(int sockfd, const SA *saptr, socklen_t salen, int nsec);
void   ParseIPAndPort(const char *str, char *ip, int &port);
char * Sock_ntop(const struct sockaddr * sa, socklen_t addrlen);
int    Sockfd_to_family(int sockfd);
int    Connect_timeo(int sockfd, const SA *saptr, socklen_t salen, int nsec);
int    Readable_timeo(int fd, int sec);

size_t  Readn(int fd, void * vptr, size_t n);
size_t  Writen(int fd, const void * vptr, size_t n);
ssize_t Readline(int fd,void*vptr,size_t maxlen);

size_t Read_fd(int fd, void *ptr, size_t nbytes, int *recvfd);
size_t Write_fd(int fd, void *ptr, size_t nbytes, int sendfd);

inline int max(int a, int b)
{
    return a > b ? a : b;
}

inline int min(int a, int b)
{
    return a > b ? b : a;
}
#endif