#ifdef _DEV_POLL_H

#include <sys/devpoll.h>

void str_cli(FILE *fp, int sockfd)
{
	int stdineof;
	char buf[MAXLINE];
	int n, wfd;

	struct pollfd pollfd[2];
	struct dvpoll dopoll;

	int i, result;

	wfd = open("/dev/poll",O_RDWR,0);
	
	pollfd[0].fd = fileno(fp);
	pollfd[0].events = POLLIN;
	pollfd[0].revents = 0;

	pollfd[1].fd = sockfd;
	pollfd[1].events = POLLIN;
	pollfd[1].revents = 0;

	write(wfd, pollfd, sizeof(struct pollfd) * 2);

	stdineof = 0;
	while(true)
	{
		/**Block until /dev/poll is ready**/
		dopoll.dp_timeout = -1;
		dopoll.dp_nfds = 2;
		dopoll.dp_fds = pollfd;

		result = ioctl(wfd, DP_POLL, &dopoll);

		for(i = 0;i < result;i++)
		{
			if(dopoll.dp_fds[i].fd == sockfd)
			{
				/**socket is readable**/
				if((n = read(sockfd, buf, MAXLINE)) == 0)
				{
					if(stdineof == 1) return;
					else err_quit("str_cli: server terminated prematurely");				
				}
				write(fileno(stdout), buf, n);
			}
			else
			{
				/**input is readable**/
				if((n = read(fileno(fp), buf, MAXLINE)) == 0)
				{
					stdineof = 1;
					shutdown(sockfd, SHUT_WR);
					continue;
				}
				write(sockfd, buf, n);
			}
		}
	}
}

#endif

#ifdef	HAVE_SYS_EVENT_H
void str_cli(FILE *fp, int sockfd)
{
	int kq, i, n, nev, stdineof = 0, isfile;
	char buf[MAXLINE];

	struct kevent kev[2];
	struct timespec ts;
	struct stat   st;

	isfile = ((fstat(fileno(fp), &st) == 0) && (st.st_mode & S_IFMT) == S_IFREG);

	EV_SET(&kev[0], fileno(fp), EVFILT_READ, EV_ADD, 0, 0, NULL);
	EV_SET(&kev[1], sockfd, EVFILT_READ, EV_ADD, 0, 0, NULL);

	kq = Kqueue();
	ts.tv_sec = tv.tv_nsec = 0;

	Kevent(kq, kev, 2, NULL, 0, &ts);

	while(true)
	{
		nev = Kevent(kq, NULL, 0, kev, 2, NULL);
		for(i = 0;i < nev;i++)
		{
			if(kev[i].ident == sockfd)
			{
				if((n = read(sockfd, buf, MAXLINE)) == 0)
				{
					if(stdineof == 1) return;
					else err_quit("str_cli: server terminated prematurely");
				}
				write(fileno(stdout), buf, n);
			}

			if(kev[i].ident == fileno(fp))
			{
				n = read(fileno(fp), buf, MAXLINE);
				if(n > 0) write(sockfd, buf, n);

				if(n == 0 || (isfile && n == kev[i].data))
				{
					stdineof = 1;
					shutdown(sockfd, SHUT_WR);
					kev[i].flags = EV_DELETE;
					Kevent(kq, &kev[i], 1, NULL, 0, &ts);
					continue;
				}
			}




		}

	}


}
#endif
