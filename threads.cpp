#include <pthread.h>

#include "unp.h"

void *copyto(void *);

static int sockfd; static FILE *fp;

void str_cli(FILE *fp_arg, int sockfd_arg)
{
	char recvline[MAXLINE];
	pthread_t tid;

	sockfd = sockfd_arg;
	fp = fp_arg;

	pthread_create(&tid, NULL, copyto, NULL);
	while(readline(sockfd, recvline, MAXLINE) > 0)
		puts(recvline);
}

void * copyto(void *arg)
{
	char sendline[MAXLINE];
	while(fgets(sendline, MAXLINE, fp) != NULL)
		write(sockfd, sendline, strlen(sendline));

	shutdown(sockfd, SHUT_WR);
	return NULL;
}


/***server**/

static void *doit(void *);

int main(int argc, char * argv[])
{
	int listenfd, connfd;
	pthread_t tid;
	socklen_t addrlen, len;
	struct sockaddr *cliaddr;

	if(argc == 2) listenfd = tcp_listen(NULL, argv[1], &addrlen);
	else if(argc = 3) listenfd = tcp_listen(argv[1], argv[2], &addrlen);
	else err_quit("usage");

	cliaddr = (struct sockaddr *)malloc(addrlen);
	while(true)
	{
		len = addrlen;
		connfd = accept(listenfd, cliaddr, &len);
		pthread_create(&tid, NULL, &doit, (void *)connfd);
	}

	exit(0);
}

static void * doit(void *arg)
{
	int connfd;
	connfd = *((int*)arg);
	pthread_detach(pthread_self());
	str_echo(connfd);
	close(connfd);
	return NULL;
}