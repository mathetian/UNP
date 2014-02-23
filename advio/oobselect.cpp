#include "../include/unp.h"

int main(int argc, char *argv[])
{
	int connfd, listenfd;
	char buf[100];
	int n;

	fd_set rset, xset;

	if(argc == 2) 
		listenfd = Tcp_listen(NULL, argv[1], NULL);
	else if(argc == 3) 
		listenfd = Tcp_listen(argv[1], argv[2], NULL);
	else
		err_quit("usage: oobselect [<host>] <port#>");

	connfd = Accept(listenfd, NULL, NULL);
	FD_ZERO(&rset);
	FD_ZERO(&xset);

	while(true)
	{
		FD_SET(connfd, &rset);
		FD_SET(connfd, &xset);

		Select(connfd + 1, &rset, NULL, &xset, NULL);

		if(FD_ISSET(connfd, &xset))
		{
			n = Recv(connfd, buf, sizeof(buf - 1), MSG_OOB);
			buf[n] = 0;
			printf("read %d OOB bytes: %s\n", n, buf);
		}

		if(FD_ISSET(connfd, &rset))
		{
			if((n = Read(connfd, buf, sizeof(buf) - 1)) == 0)
			{
				printf("EOF\n");
				exit(0);
			}
			buf[n] = 0;
			printf("read %d bytes: %s\n", n , buf);
		}
	}
}