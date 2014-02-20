#include "../unp.h"
#include "../unp.cpp"
#include "wrap.cpp"
#include "../inet.cpp"
#include <time.h>

int main(int argc, char*argv[])
{
	int listenfd, connfd;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t servlen, clilen;
	int pid; char buff[MAXLINE]; 
	
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	Listen(listenfd, LISTENQ);
	/**Another way for server allocate port, just for test**/
	getsockname(listenfd, (SA *)&servaddr, &servlen); 
	printf("system allocate %s\n", sock_ntop((SA*)&servaddr, servlen));
	
	for(;;)
	{
		connfd = Accept(listenfd,(SA*)&cliaddr, &clilen);

		if((pid = fork()) < 0) err_sys("Fork error");
		else if(pid == 0)
		{
			close(listenfd);
			
			printf("connection from %s\n", sock_ntop((SA *)&cliaddr, clilen));
		
			time_t ticks=time(NULL);
			
			snprintf(buff,sizeof(buff),"%.24s\r\n", ctime(&ticks));
			write(connfd,buff,sizeof(buff));

			close(connfd);
			exit(0);
		}
	
		close(connfd);
	}
}