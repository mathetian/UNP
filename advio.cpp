#include "unp.h"
#include "unp.cpp"

static void connect_alarm(int);

typedef void (*Sigfunc)(int);

int connect_timeo(int sockfd, const SA *saptr, socklen_t salen, int nsec)
{
	Sigfunc sigfunc;

	int n;
	sigfunc = signal(SIGALRM, connect_alarm);
	if(alarm(nsec) != 0)
		err_msg("connect_timeo: alarm has been set");
	
	if((n=connect(sockfd, saptr, salen)) < 0)
	{
		close(sockfd);
		if(errno == EINTR) errno = ETIMEDOUT;
	}

	alarm(0);
	signal(SIGALRM, sigfunc);

	return n;
}

static void connect_alarm(int signo)
{
	/**Just interrupt the connect**/
	return;
}

int readable_timeo(int fd, int sec)
{
	fd_set rset;
	struct timeval tv;

	FD_ZERO(&rset);
	FD_SET(fd, &rset);

	tv.tv_sec = sec;
	tv.tv_usec = 0;

	return select(fd + 1, &rset, NULL, NULL, &tv);
}

/**A simple demo, as I don't want to place into other files, so here**/

void dg_cli(int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE], recvline[MAXLINE];

	while(fgets(sendline, MAXLINE, stdin) != NULL)
	{
		int flag = sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		if(flag != strlen(sendline)) err_sys("sendto error");

		if(readable_timeo(sockfd, 5) == 0)
			fprintf(stderr, "socket timeout\n");
		else
		{
			n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
			recvline[n] = 0;
			puts(recvline);
		}
	}
}

/**fileno to FILE* **/
void str_echo(int sockfd)
{
	char line[MAXLINE];
	FILE *fpin, *fpout;

	fpin = fdopen(sockfd, "r");
	if(fpin == NULL) err_sys("fdopen error");

	fpout = fdopen(sockfd, "w");
	if(fpout == NULL) err_sys("fdopen error");

	while(fgets(line,MAXLINE, fpin) != NULL)
		fputs(line, fpout);
}

#ifdef _DEV_POLL_H

#include <sys/devpoll.h>

void str_cli(FILE *fp, int sockfd)
{
	int stdineof;
	char buf[MAXLINE];
	int n, wfd;

	struct pollfd pollfd[2];
	struct dvpoll dopoll;

	int i, result;

	wfd = open("/dev/poll",O_RDWR,0);
	
	pollfd[0].fd = fileno(fp);
	pollfd[0].events = POLLIN;
	pollfd[0].revents = 0;

	pollfd[1].fd = sockfd;
	pollfd[1].events = POLLIN;
	pollfd[1].revents = 0;

	write(wfd, pollfd, sizeof(struct pollfd) * 2);

	stdineof = 0;
	while(true)
	{
		/**Block until /dev/poll is ready**/
		dopoll.dp_timeout = -1;
		dopoll.dp_nfds = 2;
		dopoll.dp_fds = pollfd;

		result = ioctl(wfd, DP_POLL, &dopoll);

		for(i = 0;i < result;i++)
		{
			if(dopoll.dp_fds[i].fd == sockfd)
			{
				/**socket is readable**/
				if((n = read(sockfd, buf, MAXLINE)) == 0)
				{
					if(stdineof == 1) return;
					else err_quit("str_cli: server terminated prematurely");				
				}
				write(fileno(stdout), buf, n);
			}
			else
			{
				/**input is readable**/
				if((n = read(fileno(fp), buf, MAXLINE)) == 0)
				{
					stdineof = 1;
					shutdown(sockfd, SHUT_WR);
					continue;
				}
				write(sockfd, buf, n);
			}
		}
	}
}

#endif

#ifdef	HAVE_SYS_EVENT_H
void str_cli(FILE *fp, int sockfd)
{
	int kq, i, n, nev, stdineof = 0, isfile;
	char buf[MAXLINE];

	struct kevent kev[2];
	struct timespec ts;
	struct stat   st;

	isfile = ((fstat(fileno(fp), &st) == 0) && (st.st_mode & S_IFMT) == S_IFREG);

	EV_SET(&kev[0], fileno(fp), EVFILT_READ, EV_ADD, 0, 0, NULL);
	EV_SET(&kev[1], sockfd, EVFILT_READ, EV_ADD, 0, 0, NULL);

	kq = Kqueue();
	ts.tv_sec = tv.tv_nsec = 0;

	Kevent(kq, kev, 2, NULL, 0, &ts);

	while(true)
	{
		nev = Kevent(kq, NULL, 0, kev, 2, NULL);
		for(i = 0;i < nev;i++)
		{
			if(kev[i].ident == sockfd)
			{
				if((n = read(sockfd, buf, MAXLINE)) == 0)
				{
					if(stdineof == 1) return;
					else err_quit("str_cli: server terminated prematurely");
				}
				write(fileno(stdout), buf, n);
			}

			if(kev[i].ident == fileno(fp))
			{
				n = read(fileno(fp), buf, MAXLINE);
				if(n > 0) write(sockfd, buf, n);

				if(n == 0 || (isfile && n == kev[i].data))
				{
					stdineof = 1;
					shutdown(sockfd, SHUT_WR);
					kev[i].flags = EV_DELETE;
					Kevent(kq, &kev[i], 1, NULL, 0, &ts);
					continue;
				}
			}




		}

	}


}
#endif
