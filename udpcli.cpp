#include "unp.h"
#include "unp.cpp"

#define SERV_PORT 8088

static void sig_alrm(int)
{
	/**Just interrupt the read or write**/
	return;
}

/**V3, add TIMEDOUT feature**/
void dg_cli(int sockfd, const SA * pservaddr, socklen_t servlen)
{
	int n, flag;
	char sendline[MAXLINE], recvline[MAXLINE];

	/**Add relibility**/
	socklen_t len;
	struct sockaddr * preply_addr;

	preply_addr = (struct sockaddr *)malloc(servlen);
	if(preply_addr == NULL) err_sys("malloc error");

	len = servlen;

	signal(SIGALRM, sig_alrm);

	while(fgets(sendline, MAXLINE, stdin) != NULL)
	{
		flag = sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		if(flag == -1) err_sys("sendto error");
		
		len = servlen;
		alarm(5);

		if((n = recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr, &len)) < 0)
		{
			if(errno == EINTR) fprintf(stderr, "socket timeout\n");
			else err_sys("recvfrom error");
		}
		else
		{
			if(len != servlen || memcmp(pservaddr, preply_addr, len) != 0)
			{
				//printf("reply from %s (ignored)\n", sock_ntop(preply_addr, len ));
				printf("reply from ignored, please check it out\n");
				continue;
			}
			recvline[n] = 0;
			puts(recvline);
		}
	}
}

int main(int argc, char ** argv)
{
	int sockfd, flag;
	struct sockaddr_in servaddr;
	if(argc != 2) err_quit("usage: a.out <IPAddress>");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);

	flag = inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	if(flag != 1) err_sys("inet_pton error");

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	dg_cli(sockfd, (SA*)&servaddr, sizeof(servaddr));

	exit(0);
}