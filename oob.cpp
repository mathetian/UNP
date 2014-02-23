#include "unp.h"
#include "unp.cpp"

#include "gethostlib.cpp"

int cli(int argc, char *argv[])
{
	int sockfd;
	if(argc != 3) 
		err_quit("usage tcpsend01 <host> <port #>");

	sockfd = tcp_connect(argv[1], argv[2]);
	write(sockfd, "123", 3);

	printf("wrote 3 bytes of normal data\n");
	sleep(1);

	send(sockfd, "4", 1, MSG_OOB);
	printf("wrote 1 byte of OOB data\n");
	sleep(1);

	write(sockfd, "56", 2);
	printf("wrote 2 bytes of normal data\n");
	sleep(1);

	send(sockfd, "7", 1, MSG_OOB);
	printf("wrote 1 byte of OOB data\n");
	sleep(1);

	write(sockfd, "89", 2);
	printf("wrote 2 bytes of normal data\n");
	sleep(1);

	exit(0);
}

void sig_urg(int signo)
{
	int n;
	char buff[100];

	printf("SIGURG received\n");
	n = recv(connfd, buff, sizeof(buff) - 1, MSG_OOB);
	buff[n] = 0;
	printf("read %d OOB byte: %s\n", n, buff);
}

int serv(int argc, char *argv[])
{
	int n;
	char buff[100];
	if(argc == 2) listenfd = tcp_listen(NULL, argv[1], NULL);
	else if(argc == 3) listenfd = tcp_listen(argv[1], argv[2], NULL);
	else err_quit("usage: tcprecv01 [<host>][<port#>]");

	connfd = accept(listenfd, NULL, NULL);
	signal(SIGURG, sig_urg);
	fcntl(connfd, F_SETOWN, getpid());

	while(true)
	{
		if((n = read(connfd, buff, sizeof(buff) -1)) == 0)
		{
			printf("received EOF\n");
			exit(0);
		}
		buff[n] = 0;
		printf("read %d bytes: %s\n", n, buff);
	}
}


