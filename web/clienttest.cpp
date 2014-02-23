#include "../include/unp.h"

#define MAXN 16484

int main(int argc, char *argv[])
{
	int i, j, fd, nchildren, nloops, nbytes;
	pid_t pid;
	size_t n;

	char request[MAXLINE], reply[MAXLINE];

	if(argc != 6)
		err_quit("usage: clienttest <IP> <Port> <#nchildren> \
				<#loops/child> <#bytes/request>");

	nchildren = atoi(argv[3]);
	nloops    = atoi(argv[4]);
	nbytes    = atoi(argv[5]);

	snprintf(request, sizeof(request), "%d\n", nbytes);

	for(i = 0;i < nchildren;i++)
	{
		if((pid = fork()) == 0)
		{
			for(j=0;j<nloops;j++)
			{
				fd = Tcp_connect(argv[1], argv[2]);
				write(fd, request, strlen(request));

				if((n = Readn(fd, reply, nbytes)) != nbytes)
					err_quit("server returned different bytes");
				close(fd);
			}
			printf("child %d done\n", i);
			exit(0);
		}
	}

	while(wait(NULL) > 0);

	if(errno != ECHILD)
		err_sys("wait error");

	return 0;
}