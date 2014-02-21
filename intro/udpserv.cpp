#include "../unp.h"
#include "../unp.cpp"

#include "wrap.cpp"

#define SERV_PORT 8088

static int count;

void recvfrom_int(int signo)
{
	printf("\nreceived %d datagrams\n", count);
	exit(0);
}

void dg_echo(int sockfd, SA * pcliaddr, socklen_t clilen)
{
	int n;
	socklen_t len;
	char msg[MAXLINE];

	while(true)
	{
		len = clilen;
		n = Recvfrom(sockfd, msg, MAXLINE, 0, pcliaddr, &len);
		printf("received msg: %s\n", msg);
		Sendto(sockfd, msg, n, 0, pcliaddr, len);
	}
}

void dg_echo2(int sockfd, SA *pcliaddr, socklen_t clilen)
{
	socklen_t len;
	char mesg[MAXLINE];

	signal(SIGINT, recvfrom_int);
	while(true)
	{
		len = clilen;
		Recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
		count++;
	}
}

int main(int argc, char * argv[])
{
	int sockfd;
	struct sockaddr_in servaddr, cliaddr;

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	Bind(sockfd, (SA*)&servaddr, sizeof(servaddr));
	
	printf("Start at 127.0.0.1:%d\n", SERV_PORT);
	
	dg_echo2(sockfd, (SA*)&cliaddr, sizeof(cliaddr));
	exit(0);
}