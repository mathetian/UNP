#include "unp.h"
#include "unp.cpp"

#include <time.h>

int main(int argc, char*argv[])
{
	int listenfd, connfd; int flag;
	struct sockaddr_in servaddr, cliaddr;
	char buff[MAXLINE];
	socklen_t len;

	time_t ticks;
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(listenfd == -1) err_sys("Socket create error: ");

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family=AF_INET;
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(8081);

	flag = bind(listenfd,(SA*)&servaddr,sizeof(servaddr));
	if(flag == -1) err_sys("Bind error");

	flag = listen(listenfd,LISTENQ);
	if(flag == -1) err_sys("Listen error");

	for(;;)
	{
		len = sizeof(cliaddr);
		connfd = accept(listenfd,(struct sockaddr*)&cliaddr,&len);
		if(connfd == -1) err_sys("Accept error");
		printf("connection from %s, port %d\n", inet_ntop(AF_INET, 
			&cliaddr.sin_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port));
		ticks=time(NULL);
		snprintf(buff,sizeof(buff),"%.24s\r\n",ctime(&ticks));
		write(connfd,buff,sizeof(buff));
		close(connfd);
		sleep(100000);
		printf("end of sleep\n");
	}
}