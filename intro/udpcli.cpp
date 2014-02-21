#include "../unp.h"
#include "../unp.cpp"

#include "wrap.cpp"

#include "../inet.cpp"
#define SERV_PORT 8088

static void sig_alrm(int)
{
	return;
}

void dg_cli(int sockfd, const SA * pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE], recvline[MAXLINE];

	socklen_t len;
	struct sockaddr * preply_addr;

	preply_addr = (struct sockaddr *)malloc(servlen);
	if(preply_addr == NULL) 
		err_sys("malloc error");

	len = servlen;

	signal(SIGALRM, sig_alrm);

	while(fgets(sendline, MAXLINE, stdin) != NULL)
	{
		Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		
		len = servlen;
		alarm(5);

		if((n = Recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr, &len)) < 0)
		{ }
		else
		{
			if(len != servlen || memcmp(pservaddr, preply_addr, len) != 0)
			{
				printf("reply from ignored, please check it out\n");
				continue;
			}

			recvline[n] = 0;
			puts(recvline);
		}
	}
}

/**Don't work as expect																									**/
void dg_cli2(int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	char sendline[MAXLINE], recvline[MAXLINE];

	Connect(sockfd, (SA *)pservaddr, servlen);
	printf("connect successfully\n");
	while(fgets(sendline, MAXLINE, stdin) != NULL)
	{
		Write(sockfd, sendline, strlen(sendline));
		n = Read(sockfd, recvline, MAXLINE);

		puts(recvline);
	}
}

#define NDG 20000000
#define DGLEN 15000

void dg_cli3(int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int i;
	char sendline[DGLEN];
	for(i=0;i<DGLEN;i++)
		Sendto(sockfd, sendline, DGLEN, 0, pservaddr, servlen);

	servlen = sizeof(pservaddr);

	getsockname(sockfd, (SA *)pservaddr, &servlen); 
	printf("system allocate %s\n", sock_ntop((SA*)pservaddr, servlen));
}

int main(int argc, char ** argv)
{
	int sockfd;
	struct sockaddr_in servaddr;
	
	char ip[256]; int port;
	memset(ip, 0, sizeof(ip));

	if(argc != 2)  
		err_quit("Usage: udpcli <IPAddress:Port>");
	else
	{
		parseIPAndPort(argv[1], ip, port);
	}

	printf("%s %d\n", ip, port);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(port);

	Inet_pton(AF_INET, ip, &servaddr.sin_addr);

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
	dg_cli3(sockfd, (SA*)&servaddr, sizeof(servaddr));

	exit(0);
}