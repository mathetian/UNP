#include	"../unp.h"


int Socket(int family, int type, int protocol)
{
	int		n;

	if ( (n = socket(family, type, protocol)) < 0)
		err_sys("socket error");
	return n;
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int n;

	while((n = accept(fd, sa, salenptr)) < 0) {
		if (errno == ECONNABORTED) continue;
		else err_sys("accept error");
	}
	return n;
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (bind(fd, sa, salen) < 0)
		err_sys("bind error");
}

void Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (connect(fd, sa, salen) < 0)
		err_sys("connect error");
}

void Listen(int fd, int backlog)
{
	char	*ptr;

	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (listen(fd, backlog) < 0)
		err_sys("listen error");
}

size_t Recv(int fd, void *ptr, size_t nbytes, int flags)
{
	size_t		n;

	if ( (n = recv(fd, ptr, nbytes, flags)) < 0)
		err_sys("recv error");
	return n;
}

size_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags,
		 struct sockaddr *sa, socklen_t *salenptr)
{
	size_t		n;

	if ( (n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0)
	{
		if(errno == EINTR) fprintf(stderr, "socket timeout\n");
		else err_sys("recvfrom error");
	}

	return n;
}

size_t Recvmsg(int fd, struct msghdr *msg, int flags)
{
	size_t		n;

	if ( (n = recvmsg(fd, msg, flags)) < 0)
		err_sys("recvmsg error");
	return n;
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	int		n;

	if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
		err_sys("select error");
	return n;
}

void Send(int fd, const void *ptr, size_t nbytes, int flags)
{
	if (send(fd, ptr, nbytes, flags) != (size_t)nbytes)
		err_sys("send error");
}

void Sendto(int fd, const void *ptr, size_t nbytes, int flags, const struct sockaddr *sa, socklen_t salen)
{
	if (sendto(fd, ptr, nbytes, flags, sa, salen) != (size_t)nbytes)
		err_sys("sendto error");
}


void Shutdown(int fd, int how)
{
	if (shutdown(fd, how) < 0)
		err_sys("shutdown error");
}

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

void Write(int fd, void *ptr, size_t nbytes)
{
	if (write(fd, ptr, nbytes) != nbytes)
		err_sys("write error");
}

size_t Read(int fd, void *ptr, size_t nbytes)
{
	size_t		n;

	if ( (n = read(fd, ptr, nbytes)) == -1)
		err_sys("read error");
	return(n);
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