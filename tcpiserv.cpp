#include "unp.h"
#include "read.cpp"
#include "unp.cpp"

#define SERV_PORT 8085

void str_echo(int sockfd)
{
	ssize_t n;
	char buf[MAXLINE];

again:
	while((n=read(sockfd, buf, MAXLINE)) > 0)
	{
		printf("%s",buf);
		writen(sockfd, buf, n);
	}
		
	if(n < 0 && errno == EINTR) goto again;
	else if(n < 0) err_sys("str_echo: read_error");
}

int main(int argc, char*argv[])
{
	int listenfd, connfd; int flag;
	pid_t childpid;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd == -1) err_sys("Socket error");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	flag = bind(listenfd, (SA*)&servaddr, sizeof(servaddr));
	if(flag == -1) err_sys("Bind error");

	flag = listen(listenfd, LISTENQ);
	if(flag == -1) err_sys("Listen error");

	clilen = sizeof(cliaddr);

	printf("Begin Listen in the port %d\n", SERV_PORT);

	while(true)
	{
		connfd = accept(listenfd, (SA*)&cliaddr, &clilen);
		if(connfd == -1) err_sys("Accept error");

		if((childpid = fork()) < 0)
			err_sys("Fork error");
		else if(childpid == 0)
		{
			close(listenfd);
			printf("Start connection with %d\n",connfd);
			str_echo(connfd);
			printf("End connection with %d\n", connfd);
			close(connfd);
			exit(0);
		}
		close(connfd);
	}
	return 0;
}