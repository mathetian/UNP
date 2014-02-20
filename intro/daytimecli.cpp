#include "../unp.h"
#include "../unp.cpp"
#include "wrap.cpp"
#include "../inet.cpp"

int main(int argc,char*argv[])
{
	int sockfd, n;
	char recvline[MAXLINE+1];
	struct sockaddr_in servaddr;
	socklen_t server_len;

	char ip[256]; int port;
	memset(ip, 0, sizeof(ip));

	if(argc != 2)
		err_quit("Usage: a.out <IPAddress:Port>\n");
	else
	{
		parseIPAndPort(argv[1], ip, port);
	}

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&servaddr,sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(port);
	
	if(inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0)
		err_quit("inet_pton error for %s",ip);
	
	Connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
	
	/**
		Didn't find any way to find local port or ip.
	getsockname(sockfd, (SA *)&servaddr, &server_len);
	printf("system allocate %s\n", sock_ntop((SA*)&servaddr, server_len));
	getpeername(sockfd, (SA *)&servaddr, &server_len);
	printf("system allocate %s\n", sock_ntop((SA*)&servaddr, server_len));
	**/
	while((n = read(sockfd, recvline, MAXLINE)) > 0)
	{
		recvline[n] = 0;
		if(fputs(recvline, stdout) == EOF)
			err_sys("fputs error");
	}
	if(n<0) err_sys("read error");

	exit(0);
}
