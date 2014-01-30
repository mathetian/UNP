#include "unp.h"
#include "unp.cpp"
#include "read.cpp"

void str_cli(FILE * fp, int sockfd)
{
	int maxfd1, stdineof, n;
	fd_set rset; char buf[MAXLINE];
	stdineof = 0;
	
	FD_ZERO(&rset);

	while(true)
	{
		if(stdineof == 0)
			FD_SET(fileno(fp), &rset);

		FD_SET(sockfd,&rset);

		maxfd1 = max(fileno(fp), sockfd) + 1;
		select(maxfd1, &rset, NULL, NULL, NULL);

		if(FD_ISSET(sockfd, &rset))
		{
			if((n = read(sockfd, buf, MAXLINE)) == 0)
			{
				if(stdineof == 1) return;
				else err_quit("str_cli: server terminated prematurely");
			}
			puts(buf);
		}
		
		if(FD_ISSET(fileno(fp), &rset))
		{
			if((n=read(fileno(fp), buf, MAXLINE))==0)
			{
				stdineof = 1;
				shutdown(sockfd, SHUT_WR);
				FD_CLR(fileno(fp), &rset);
				continue;
			}
			writen(sockfd, buf, n);
		}
	}
}