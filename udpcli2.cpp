#include "unp.h"
#include "unp.cpp"

#define SERV_PORT 8088

/**V3, add TIMEDOUT feature**/
void dg_cli(int sockfd, const SA * pservaddr, socklen_t servlen)
{
	int n, flag;
	char sendline[MAXLINE], recvline[MAXLINE];

	flag = connect(sockfd, pservaddr, servlen);
	if(flag == -1) err_sys("connect error");

	while(fgets(sendline, MAXLINE, stdin) != NULL)
	{
		flag = write(sockfd, sendline, strlen(sendline));
		if(flag != strlen(sendline)) err_sys("write error\n");

		n = read(sockfd, recvline, MAXLINE);
		if(n == -1) err_sys("read error");

		recvline[n] = 0;

		puts(recvline);
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