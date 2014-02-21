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