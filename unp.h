#ifndef _UNP_H
#define _UNP_H

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <assert.h>

#include <netdb.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>

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
int    Accpet(int sockfd, struct sockaddr *cliaddr, socklen_t * addrlen);

void parseIPAndPort(const char *str, char *ip, int &port);
inline int max(int a,int b)
{
	return a > b ? a : b;
}

#endif