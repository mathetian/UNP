#include "../unp.h"
#include "../unp.cpp"
#include "../intro/wrap.cpp"

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

void dg_cli(int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE], recvline[MAXLINE];

	while(fgets(sendline, MAXLINE, stdin) != NULL)
	{
		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

		if(readable_timeo(sockfd, 5) == 0)
			fprintf(stderr, "socket timeout\n");
		else
		{
			n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
			recvline[n] = 0;
			puts(recvline);
		}
	}
}

void dg_cli2(int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE], recvline[MAXLINE + 1];

	struct timeval tv;
	tv.tv_sec  = 0;
	tv.tv_usec = 0;

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

	while(fgets(sendline, MAXLINE, stdin) != NULL)
	{
		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
		if(n < 0)
		{
			if(errno == EWOULDBLOCK)
			{
				fprintf(stderr, "socket timeout\n");
				continue;
			}
			else err_sys("Recvfrom error");
		}
	}
	recvline[n] = 0;
	puts(recvline);
}

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