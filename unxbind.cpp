#include "unp.h"
#include "unp.cpp"

#include <sys/un.h>

int main(int argc, char*argv[])
{
	int sockfd, flag;
	socklen_t len;

	struct sockaddr_un addr1, addr2;

	if(argc != 2) err_quit("usage: unxbind <pathname>");

	sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if(sockfd == -1) err_sys("socket error");

	unlink(argv[1]);
	
	bzero(&addr1, sizeof(addr1));
	addr1.sun_family = AF_LOCAL;
	strncpy(addr1.sun_path, argv[1], sizeof(addr1.sun_path) - 1);

	flag = bind(sockfd, (SA*)&addr1, sizeof(&addr1));
	len = sizeof(addr2);

	getsockname(sockfd, (SA*)&addr2, &len);
	printf("bound name = %s, returned len = %d\n", addr2.sun_path, len);

	exit(0);
}