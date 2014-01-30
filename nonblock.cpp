#include "unp.h"
#include "unp.cpp"

#include <time.h>
#include <sys/time.h>

char * gf_time()
{
	struct timeval tv;
	static char str[30];
	char *ptr;

	if(gettimeofday(&tv, NULL) < 0)
		err_sys("gettimeofday error");

	ptr = ctime(&tv.tv_sec);
	strcpy(str, &ptr[11]);
	snprintf(str + 8, sizeof(str) - 8, ".%061d", tv.tv_usec);

	return str;
}	

void str_cli(FILE *fp, int sockfd)
{
	int maxfdp1, val, stdineof;
	ssize_t n, nwritten;
	fd_set rset, wset;
	char to[MAXLINE], fr[MAXLINE];

	char *toiptr, *tooptr, *friptr, *froptr;

	val = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, val | O_NONBLOCK);

	val = fcntl(STDOUT_FILENO, F_GETFL, 0);
	fcntl(STDOUT_FILENO, F_SETFL, val | O_NONBLOCK);

	toiptr = tooptr = to;
	friptr = froptr = fr;
	stdineof = 0;

	maxfdp1 = max(max(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;

	while(true)
	{
		FD_ZERO(&rset);
		FD_ZERO(&wset);

		if(stdineof == 0 && toiptr < &to[MAXLINE]) FD_SET(STDIN_FILENO, &rset);
		
		if(friptr < &fr[MAXLINE]) FD_SET(sockfd, &rset);

		if(tooptr != toiptr) FD_SET(sockfd, &wset);
		if(froptr != friptr) FD_SET(STDOUT_FILENO, &wset);

		select(maxfdp1, &rset, &wset, NULL, NULL);

		if(FD_ISSET(STDIN_FILENO, &rset))
		{
			if((n = read(STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0)
			{
				if(errno != EWOULDBLOCK)
					err_sys("read error on stdin");
			}
			else if(n == 0)
			{
				fprintf(stderr,"%s: EOF on stdin\n", gf_time());
				stdineof = 1;
				if(tooptr == toiptr) shutdown(sockfd, SHUT_WR);
			}
			else
			{
				fprintf(stderr, "%s: read %d bytes from stdin\n", gf_time(), n);
				toiptr += n;
				FD_SET(sockfd, &wset);
			}
		}

		if(FD_ISSET(sockfd, &rset))
		{
			if((n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0)
			{
				if(errno != EWOULDBLOCK)
					err_sys("read error on socket");
			}
			else if(n == 0)
			{
				fprintf(stderr, "%s: EOF on socket\n", gf_time());
				if(stdineof) return;
				else err_quit("str_cli: server terminated prematurely");
			}
			else
			{
				fprintf(stderr, "%s: read %d bytes from socket\n", gf_time(), n);
				friptr += n;
				FD_SET(STDOUT_FILENO, &wset);
			}
		}		

		if(FD_ISSET(STDOUT_FILENO, &wset) && ((n = friptr - froptr) > 0))
		{
			if((nwritten = write(STDOUT_FILENO, froptr, n)) < 0)
			{
				if(errno != EWOULDBLOCK)
					err_sys("write error to stdout");
			}
			else
			{
				fprintf(stderr, "%s: wrote %d bytes to stdout\n", gf_time(), nwritten);
				froptr += nwritten;
				if(froptr == friptr) froptr = friptr = fr;
			}
		}

		if(FD_ISSET(sockfd, &wset) && ((n = toiptr - tooptr) > 0))
		{
			if((nwritten = write(sockfd, tooptr, n)) < 0)
			{
				if(errno != EWOULDBLOCK) err_sys("write error to socket");
			}
			else
			{
				fprintf(stderr, "%s: wrote %d bytes to socket\n", gf_time(), nwritten);
				tooptr += nwritten;
				if(tooptr == toiptr)
				{
					toiptr = tooptr = to;
					if(stdineof) shutdown(sockfd, SHUT_WR);
				}
			}
		}
	}
}

void str_cli2(FILE * fp, int sockfd)
{
	pid_t pid;
	char sendline[MAXLINE], recvline[MAXLINE];

	if((pid = fork()) < 0) err_sys("fork error");
	else if(pid == 0)
	{
		while(readline(sockfd, recvline, MAXLINE) > 0)
			puts(recvline);

		kill(getppid(), SIGTERM);
		exit(0);
	}
	while(fgets(sendline, MAXLINE, fp) != NULL)
		write(sockfd, sendline, strlen(sendline));

	shutdown(sockfd, SHUT_WR);
	pause();

	return;
}	