#include "../unp.h"
#include "../unp.cpp"
#include "../read.cpp"
#include "wrap.cpp"

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

	char ip[256]; int port;
	memset(ip, 0, sizeof(ip));

	if(argc != 2)  
		err_quit("Usage: tcpicli <IPAddress:Port>");
	else
	{
		parseIPAndPort(argv[1], ip, port);
	}
	printf("%s %d\n", ip, port);
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

	if(inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0)
		err_quit("inet_pton error for %s", ip);

	Connect(sockfd, (SA*)&servaddr, sizeof(servaddr));

	printf("Connected successfully\n");

	str_cli(stdin, sockfd);

	printf("End Connected\n");
	
	exit(0);
}