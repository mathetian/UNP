#include "unp.h"

static void recvfrom_alarm(int)
{
	return;
}

void dg_cli(int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	const int on = 1;
	char sendline[MAXLINE], recvline[MAXLINE];

	socklen_t len; struct sockaddr *preply_addr;

	preply_addr = (struct sockaddr *)malloc(servlen);

	if(preply_addr == NULL) err_sys("malloc error");
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
	
	signal(SIGALRM, recvfrom_alarm);
	while(fgets(sendline, MAXLINE, stdin) != NULL)
	{
		sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		alarm(5);
		while(true)
		{
			len = servlen;
			n = recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr, &len);
			if(n < 0)
			{
				if(errno == EINTR) break;
				else err_sys("recvfrom error");
			}
			else
			{
				recvline[n] = 0;
				printf("from %s: %s", sock_ntop_host(preply_addr, len), recvline);
			}
		}
	}
	free(preply_addr);
}	

/**Incorrect version**/
void dg_cli2(int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	const int on = 1;
	char sendline[MAXLINE], recvline[MAXLINE];

	socklen_t len; struct sockaddr *preply_addr;

	sigset_t sigset_alrm;

	preply_addr = (struct sockaddr *)malloc(servlen);

	if(preply_addr == NULL) err_sys("malloc error");
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
	
	sigemptyset(&sigset_alrm);
	sigaddset(&sigset_alrm, SIGALRM);
	signal(SIGALRM, recvfrom_alarm);

	while(fgets(sendline, MAXLINE, stdin) != NULL)
	{
		sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		alarm(5);
		while(true)
		{
			len = servlen;
			sigprocmask(SIG_UNBLOCK, &sigset_alrm, NULL);

			n = recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr, &len);
			
			sigprocmask(SIG_BLOCK, &sigset_alrm, NULL);

			if(n < 0)
			{
				if(errno == EINTR) break;
				else err_sys("recvfrom error");
			}
			else
			{
				recvline[n] = 0;
				printf("from %s: %s", sock_ntop_host(preply_addr, len), recvline);
			}
		}
	}
	free(preply_addr);
}	

/**pselect version**/
void dg_cli3(int sockfd, const SA *pservaddr, socklen_t servlen)
{
	int n;
	const int on = 1;
	char sendline[MAXLINE], recvline[MAXLINE];

	socklen_t len; struct sockaddr *preply_addr;

	sigset_t sigset_alrm, sigset_empty; fd_set rset;

	preply_addr = (struct sockaddr *)malloc(servlen);
	if(preply_addr == NULL) err_sys("malloc error");
	
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
	
	FD_ZERO(&rset);

	sigemptyset(&sigset_alrm);
	sigemptyset(&sigset_empty);
	sigaddset(&sigset_alrm, SIGALRM);

	signal(SIGALRM, recvfrom_alarm);

	while(fgets(sendline, MAXLINE, stdin) != NULL)
	{
		sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
		sigprocmask(SIG_BLOCK, &sigset_alrm, NULL);

		alarm(5);
		while(true)
		{
			FD_SET(sockfd, &rset);
			n = pselect(sockfd + 1, &rset, NULL, NULL, NULL, &sigset_empty);
			
			if(n < 0)
			{
				if(errno == EINTR) break;
				else err_sys("pselect error");
			}
			else if(n != 1)
				err_sys("pselect error: returned %d", n);
			
			len = servlen;
			n = recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr, &len);

			recvline[n] = 0;
			printf("from %s: %s", sock_ntop_host(preply_addr, len), recvline);
		}
	}
	free(preply_addr);
}	

int pselect(int nfds, fd_set *rset, fd_set *wset, fd_set *xset, const struct timespec *ts, const sigset_t *sigmask)
{
	int n;
	struct timeval tv;
	sigset_t savemask;

	if(ts != NULL)
	{
		tv.tv_sec = ts->tv_sec;
		tv.tv_usec = ts->tv_nsec/1000;
	}

	sigprocmask(SIG_SETMASK, sigmask, &savemask);
	n = select(nfds, rset, wset, xset, (ts == NULL) ? NULL : &tv);
	sigprocmask(SIG_SETMASK, &savemask, NULL);
	return n;
}