#include "unp.h"
#include "read.cpp"
#include "unp.cpp"

#include <limits.h>

#define SERV_PORT 8085

int main(int argc, char*argv[])
{
	int i, maxi, listenfd, connfd, sockfd;
	int nready, flag;
	struct pollfd client[OPEN_MAX];

	ssize_t n;
	fd_set rset, allset;

	char buf[MAXLINE];
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

	maxi = -1;

	client[0].fd = listenfd;
	client[0].events = POLLRDNORM;

	for(i = 1;i < OPEN_MAX;i++)
		client[i].fd = -1;

	while(true)
	{
		nready = poll(client, maxi + 1, INFTIM);

		if(client[0].revents & POLLRDNORM)
		{
			connfd = accept(listenfd, (SA*)&cliaddr, &clilen);
			if(connfd == -1 && errno == EINTR) continue;
			if(connfd == -1) err_sys("Accept error");
			
			for(i = 0;i < OPEN_MAX;i++)
			{
				if(client[i].fd < 0)
				{
					client[i].fd = connfd;
					break;
				}
			}

			if(i == OPEN_MAX)
				err_quit("too many clients");
			
			client[i].events = POLLRDNORM;

			maxi = max(maxi, i);

			if(--nready <= 0) continue;
		}
		for(i = 0;i <= maxi;i++)
		{
			if((sockfd = client[i].fd) < 0) continue;
			if(client[i].revents & (POLLRDNORM | POLLERR))
			{
				if((n=read(sockfd, buf, MAXLINE)) == 0)
				{
					if(errno = ECONNRESET)
					{
						close(sockfd);
						client[i].fd = -1;
					}
					else err_sys("read error");
				}
				else writen(sockfd, buf, n);

				if(--nready <= 0) break;
			}
		}
	}
	return 0;
}