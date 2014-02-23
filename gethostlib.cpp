#include "unp.h"

struct addrinfo *host_serv(const char *host, const char *serv, int family, int socktype)
{
	int n;
	struct addrinfo hints, *res;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = family;
	hints.ai_socktype = socktype;

	if((n = getaddrinfo(host, serv, &hints, &res)) != 0)
		return NULL;
	return res;
}

int tcp_connect(const char *hostname, const char *service)
{
	int sockfd, n;
	struct addrinfo hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((n = getaddrinfo(hostname, service, &hints, &res)) != 0)
		err_quit("tcp_connect error for %s, %s: %s", hostname, service, gai_strerror(n));
	ressave = res;

	do{
		sockfd = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);
		if(sockfd < 0) continue;

		if(connect(sockfd, res -> ai_addr, res -> ai_addrlen) == 0)
			break;
		close(sockfd);
	}while((res = res->ai_next) != NULL);

	if(res == NULL)
		err_sys("tcp_connect error for %s, %s", hostname, service);
}	

int tcp_listen(const char *hostname, const char * service, socklen_t *addrlenp)
{
	int listenfd, n;
	const int on = 1;

	struct addrinfo hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((n = getaddrinfo(hostname, service, &hints, &res)) != 0)
		err_quit("tcp_listen error for %s, %s: %s", hostname, service, gai_strerror(n));

	ressave = res;

	do{
		listenfd = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
		if(listenfd < 0) continue;
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		if(bind(listenfd, res->ai_addr,res->ai_addrlen) == 0)
			break;
		close(listenfd);
	}while((res = res->ai_next) != NULL);

	if(res == NULL) err_sys("tcp_listen error for %s, %s", hostname, service);

	int flag = listen(listenfd, LISTENQ);
	if(flag == -1) err_sys("listen error");

	if(addrlenp) *addrlenp = res->ai_addrlen;
	freeaddrinfo(ressave);	

	return listenfd;
}

int udp_client(const char *hostname, const char *service, SA **saptr, socklen_t *lenp)
{
	int sockfd, n;
	struct addrinfo hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if((n = getaddrinfo(hostname, service, &hints, &res)) != 0)
		err_quit("udp_client error");
	ressave = res;

	do{
		sockfd = socket(res -> ai_family, res -> ai_socktype, res -> ai_protocol);
		if(sockfd >= 0) break;
	}while((res = res -> ai_next) != NULL);

	*saptr = (SA*)malloc(res -> ai_addrlen);
	if(*saptr == NULL) err_sys("udp_client error");

	memcpy(*saptr, res -> ai_addr, res->ai_addrlen);
	*lenp = res -> ai_addrlen;
	freeaddrinfo(ressave);

	return sockfd;
}

int udp_connect(const char *hostname, const char *service)
{
	int sockfd, n;
	struct addrinfo hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if((n = getaddrinfo(hostname, service, &hints, &res)) != 0)
		err_quit("udp_connect error");
	ressave = res;
	do{
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		if(sockfd < 0) continue;

		if(connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;
		close(sockfd);
	}while((res = res->ai_next) != NULL);

	if(res == NULL) err_sys("udp_connect error for %s, %s", hostname, service);
	freeaddrinfo(ressave);

	return sockfd;
}

int udp_server(const char *hostname, const char *service, socklen_t *addrlenp)
{
	int sockfd, n;
	struct addrinfo hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if((n = getaddrinfo(hostname,service, &hints, &res)) != 0)
		err_quit("udp_server error");
	ressave = res;

	do{
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(sockfd < 0) continue;
		if(bind(sockfd, res->ai_addr, res->ai_addrlen) == 0) break;
		close(sockfd);	
	}while((res = res->ai_next) != NULL);

	if(res == NULL) err_sys("udp_server error");

	if(addrlenp) *addrlenp = res->ai_addrlen;
	freeaddrinfo(ressave);
	return sockfd;
}



