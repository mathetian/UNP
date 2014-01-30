#include "unp.h"
#include "unp.cpp"

#define SERV_PORT 8088

void dg_echo(int sockfd, SA * pcliaddr, socklen_t clilen)
{
	int n;
	socklen_t len;
	char msg[MAXLINE];

	while(true)
	{
		len = clilen;
		n = recvfrom(sockfd, msg, MAXLINE, 0, pcliaddr, &len);
		if(n == -1) err_sys("Recvfrom error");
		
		n = sendto(sockfd, msg, n, 0, pcliaddr, len);
		if(n == -1) err_sys("Sendto error");
	}
}

int main(int argc, char * argv[])
{
	int sockfd, flag;
	struct sockaddr_in servaddr, cliaddr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1) err_sys("Socket error");
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	flag = bind(sockfd, (SA*)&servaddr, sizeof(servaddr));
	if(flag == -1) err_sys("Bind error");

	/**Difference**/
	dg_echo(sockfd, (SA*)&cliaddr, sizeof(cliaddr));
	exit(0);
}