#include "unp.h"

int inet_pton_free(int family, const char *strptr, void * addrptr)
{
	if(family == AF_INET)
	{
		struct in_addr in_val;
		if(inet_aton(strptr, &in_val))
		{
			memcpy(addrptr, &in_val, sizeof(struct in_addr));
			return 1;
		}
		return 0;
	}
	errno = EAFNOSUPPORT;
	return -1;
}

const char * inet_ntop_free(int family, const void * addrptr, char * strptr, size_t len)
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

char * sock_ntop(const struct sockaddr * sa, socklen_t addrlen)
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

int sockfd_to_family(int sockfd)
{
	struct sockaddr_storage ss;
	socklen_t len;

	len = sizeof(ss);
	if(getsockname(sockfd, (SA*)&ss, &len) < 0)
		return -1;
	return ss.ss_family;
}