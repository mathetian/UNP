#include "unp.h"
#include "unp.cpp"
#include "read.cpp"

#define SERV_PORT 8085

void str_cli(FILE * fp, int sockfd)
{
	char sendline[MAXLINE], recvline[MAXLINE];

	while(fgets(sendline, MAXLINE, fp) != NULL)
	{
		writen(sockfd, sendline, strlen(sendline));

		if(readline(sockfd, recvline, MAXLINE) == 0)
			err_quit("str_cli: server terminated prematurely");

		fputs(recvline, stdout);
	}
}

int main(int argc, char*argv[])
{
	int sockfd;
	struct sockaddr_in servaddr;

	if(argc != 2)  err_quit("Usage: tcpicli <IPAddress>");

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) err_sys("Socket error");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);

	if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		err_quit("inet_pton error for %s",argv[1]);

	if(connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) == -1)
		err_sys("Connect error");

	printf("Connected successfully\n");

	str_cli(stdin, sockfd);

	printf("End Connected\n");
	exit(0);
}