#include "unp.h"

void Inet_pton(int family, const char *strptr, void * addrptr)
{
	if(family == AF_INET)
	{
		struct in_addr in_val;
		if(inet_aton(strptr, &in_val))
		{
			memcpy(addrptr, &in_val, sizeof(struct in_addr));
		}
		else err_quit("parse error");
	}
	else
		err_quit("family error");
}

const char * Inet_ntop(int family, const void * addrptr, char * strptr, size_t len)
{
	const u_char *p = (const u_char*) addrptr;
	if(family == AF_INET)
	{
		char temp[INET_ADDRSTRLEN];
		snprintf(temp, sizeof(temp), "%d.%d.%d.%d",p[0],p[1],p[2],p[3]);
		if(strlen(temp) >= len)
		{
			errno = ENOSPC;
			return NULL;
		}	
		strcpy(strptr, temp);
		return strptr;
	}
	errno = EAFNOSUPPORT;
	return NULL;
}

int Connect_nonb(int sockfd, const SA *saptr, socklen_t salen, int nsec)
{
	int flags, n, error;
	socklen_t len;
	fd_set rset, wset;
	struct timeval tval;

	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

	error = 0;
	if((n = connect(sockfd, saptr, salen)) < 0)
	{
		if(errno != EINPROGRESS) return -1;
	}
	
	if(n != 0)
	{
		FD_ZERO(&rset);
		FD_SET(sockfd, &rset);

		wset = rset;
		tval.tv_sec  = nsec;
		tval.tv_usec = 0;

		if((n = select(sockfd + 1, &rset, &wset, NULL, nsec ? &tval : NULL)) == 0)
		{
			close(sockfd);
			errno = ETIMEDOUT;
			return -1;
		}

		if(FD_ISSET(sockfd, &rset) || FD_ISSET(sockfd, &wset))
		{
			len = sizeof(error);
			if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
				return -1;
			else
				err_quit("select error: sockfd not set");
		}			
	}

	fcntl(sockfd, F_SETFL, flags);
	if(error)
	{
		close(sockfd);
		errno = error;
		return -1;
	}

	return 0;
}


void ParseIPAndPort(const char *str, char *ip, int &port)
{
	int i = 0;
	while(i < strlen(str) && str[i] != ':') i++;

	assert(i != strlen(str));

	memcpy(ip, str, i);
	i++; port = 0;
	while(i < strlen(str))
	{
		assert(str[i] >= '0' && str[i] <= '9');
		port = port*10 + (str[i] - '0');
		i++;
	}
}

char * Sock_ntop(const struct sockaddr * sa, socklen_t addrlen)
{
	char portstr[8];
	static char str[128];

	switch(sa->sa_family)
	{
	case AF_INET:
	{
		struct sockaddr_in * sin = (struct sockaddr_in*)sa;
		if(inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return NULL;
		if(ntohs(sin->sin_port) != 0)
		{
			snprintf(portstr, sizeof(portstr), ":%d",ntohs(sin->sin_port));
			strcat(str,portstr);
		}
		return str;
	}
	default:
	{
		snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d", sa->sa_family, addrlen);
		return(str);
	}
	}
}

int Sockfd_to_family(int sockfd)
{
	struct sockaddr_storage ss;
	socklen_t len;

	len = sizeof(ss);
	if(getsockname(sockfd, (SA*)&ss, &len) < 0)
		return -1;
	return ss.ss_family;
}

static void connect_alarm(int signo)
{
	return;
}

typedef void (*Sigfunc)(int);

int Connect_timeo(int sockfd, const SA *saptr, socklen_t salen, int nsec)
{
	Sigfunc sigfunc;

	int n;
	sigfunc = signal(SIGALRM, connect_alarm);
	if(alarm(nsec) != 0)
		err_msg("connect_timeo: alarm has been set");
	
	if((n=connect(sockfd, saptr, salen)) < 0)
	{
		close(sockfd);
		if(errno == EINTR) errno = ETIMEDOUT;
	}

	alarm(0);
	signal(SIGALRM, sigfunc);

	return n;
}

int Readable_timeo(int fd, int sec)
{
	fd_set rset;
	struct timeval tv;

	FD_ZERO(&rset);
	FD_SET(fd, &rset);

	tv.tv_sec = sec;
	tv.tv_usec = 0;

	return select(fd + 1, &rset, NULL, NULL, &tv);
}