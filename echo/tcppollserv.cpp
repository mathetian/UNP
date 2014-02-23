#include "../include/unp.h"

#define SERV_PORT 8085

int main(int argc, char*argv[])
{
	int listenfd, connfd, sockfd;
	struct sockaddr_in cliaddr, servaddr;
	socklen_t clilen;

	int nready, i, maxi;
	size_t n;

	struct pollfd client[OPEN_MAX];
	fd_set rset, allset;

	char buf[MAXLINE];

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	Bind(listenfd, (SA*)&servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	printf("Begin Listen in the port %d\n", SERV_PORT);

	maxi = -1;

	client[0].fd     = listenfd;
	client[0].events = POLLRDNORM;

	for(i = 1;i < OPEN_MAX;i++) 
		client[i].fd = -1;

	while(true)
	{
		nready = poll(client, maxi + 1, -1);

		if(client[0].revents & POLLRDNORM)
		{
			connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);
			
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
				else Writen(sockfd, buf, n);

				if(--nready <= 0) break;
			}
		}
	}
	return 0;
}