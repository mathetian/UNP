#include "../unp.h"
#include "../unp.cpp"

#include "wrap.cpp"

#include <time.h>

int main(int argc, char*argv[])
{
	int listenfd, connfd; int flag;
	struct sockaddr_in servaddr, cliaddr;
	char buff[MAXLINE];
	socklen_t len;

	time_t ticks;
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(8082);

	Bind(listenfd, (SA*)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	for(;;)
	{
		len = sizeof(cliaddr);
		connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &len);
		connfd = accept(listenfd,(struct sockaddr*)&cliaddr,&len);
		
		printf("connection from %s, port %d\n", inet_ntop(AF_INET,
				&cliaddr.sin_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port));
		
		ticks=time(NULL);
		snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
		write(connfd, buff, sizeof(buff));
		
		close(connfd);
		sleep(10);
		printf("end of sleep\n");
	}
}