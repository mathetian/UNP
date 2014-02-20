#include "../unp.h"
#include "../unp.cpp"
#include "../inet.cpp"
#include "../read.cpp"
#include "wrap.cpp"

#define SERVPORT 9001

void str_echo(int sockfd)
{
	ssize_t n;
	char buf[MAXLINE];

	while((n=read(sockfd, buf, MAXLINE)) != 0)
	{
		if(n < 0 && errno == EINTR)
			continue;
		else if(n < 0) 
			err_sys("str_echo: read_error");
	
		printf("%s",buf);
		writen(sockfd, buf, n);
	}
}

int main(int argc, char*argv[])
{
	int listenfd, connfd;
	struct sockaddr_in servaddr;
	struct sockaddr_in cliaddr;
	socklen_t servlen;
	socklen_t clilen;
	
	pid_t childpid;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERVPORT);

	Bind(listenfd,(SA*)&servaddr,sizeof(servaddr));

	Listen(listenfd, LISTENQ);
	
	/**can't figure out the problem**/
	// getsockname(listenfd, (SA *)&servaddr, &servlen); 
	// printf("system allocate %s\n", sock_ntop((SA*)&servaddr, servlen));

	while(true)
	{
		connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);

		if((childpid = fork()) < 0) 
			err_sys("Fork error");
		else if(childpid == 0)
		{
			close(listenfd);
			printf("Start connection with %d\n", connfd);
			str_echo(connfd);
			printf("End connection with %d\n", connfd);
			close(connfd);
			exit(0);
		}
		close(connfd);
	}
	return 0;
}