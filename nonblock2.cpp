#include "unp.h"
#include "unp.cpp"

int connect_nonb(int sockfd, const SA *saptr, socklen_t salen, int nsec)
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
	else if(n ==0) goto done;

	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);

	wset = rset;
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	if((n = select(sockfd + 1, &rset, &wset, NULL, nsec ? &tval : NULL)) == 0)
	{
		close(sockfd);
		errno = ETIMEDOUT;
		return -1;
	}
	else err_quit("select error: sockfd not set");

done:
	fcntl(sockfd, F_SETFL, flags);
	if(error)
	{
		close(sockfd);
		errno = error;
		return -1;
	}

	return 0;
}