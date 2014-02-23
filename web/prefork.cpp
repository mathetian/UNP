#include "unp.h"
#include "unp.cpp"

#include "advio/readline.cpp"

static int nchildren;
static pid_t *pids;

void pr_cpu_time();
void sig_int(int signo);
pid_t child_make(int i, int listenfd, int addrlen);
void  child_main(int i, int listenfd, int addrlen);
void  web_child(int sockfd);
long * meter(int nchildren);
void my_lock_init(char *pathname);
void my_lock_wait();
void my_lock_release();
void my_lock_init2(char *pathname);
void my_lock_wait2();
void my_lock_release2();

void child_main3(int i, int listenfd, int addrlen);

void pr_cpu_time()
{
	double user, sys;

	struct rusage myusage, childusage;

	if(getrusage(RUSAGE_SELF, &myusage) < 0) err_sys("getrusage error");
	if(getrusage(RUSAGE_CHILDREN, &childusage) < 0) err_sys("getrusage error");

	user = (double)myusage.ru_utime.tv_sec + myusage.ru_utime.tv_usec/1000000.0;
	user += (double)childusage.ru_utime.tv_sec + childusage.ru_utime.tv_usec/1000000.0;
	sys = (double)myusage.ru_stime.tv_sec + myusage.ru_stime.tv_usec/1000000.0;
	sys += (double)childusage.ru_stime.tv_sec + childusage.ru_stime.tv_usec/1000000.0;

	printf("\nuser time = %g, sys time = %g\n", user, sys);	
}

void sig_int(int signo)
{
	int i;

	for(i=0;i<nchildren;i++)
		kill(pids[i], SIGTERM);
	while(wait(NULL) > 0);

	if(errno != ECHILD) err_sys("wait error");

	pr_cpu_time();
	exit(0);
}

pid_t child_make(int i, int listenfd, int addrlen)
{
	pid_t pid;

	if((pid = fork()) < 0) err_sys("fork error");
	else if(pid > 0) return pid;

	child_main(i, listenfd, addrlen);
}

void child_main(int i, int listenfd, int addrlen)
{
	int connfd;
	void web_child(int);
	socklen_t clilen;
	struct sockaddr *cliaddr;
	fd_set rset;

	cliaddr = (struct sockaddr *)malloc(addrlen);

	printf("child %ld starting\n", (long)getpid());

	FD_ZERO(&rset);

	while(true)
	{
		FD_SET(listenfd, &rset);
		select(listenfd + 1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(listenfd, &rset) == 0) err_quit("listenfd readable");
		clilen = addrlen;
		my_lock_wait();
		connfd = accept(listenfd, cliaddr, &clilen);
		my_lock_release();
		web_child(connfd);
		close(connfd);
	}
}

#define MAXN 16384

void web_child(int sockfd)
{
	int ntowrite;
	ssize_t nread;
	char line[MAXLINE], result[MAXLINE];

	while(true)
	{
		if((nread = readline(sockfd, line, MAXLINE)) == 0)
			return;
		ntowrite = atol(line);
		if((ntowrite <= 0 || ntowrite > MAXN))
			err_quit("client request too much or too less bytes");
		write(sockfd, result, ntowrite);
	}
}

long * meter(int nchildren)
{
	int fd;
	long *ptr;

	ptr = mmap(0, nchildren * sizeof(long), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, NULL);

	return ptr;
}

static struct flock lock_it, unlock_it;
static int lock_fd = -1;

void my_lock_init(char *pathname)
{
	char lock_file[1024];
	strncpy(lock_file, pathname, sizeof(lock_file));
	lock_fd = mkstemp(lock_file);
	unlink(lock_file);

	lock_it.l_type = F_WRLCK;
	lock_it.l_whence = SEEK_SET;
	lock_it.l_start = 0;
	lock_it.l_len = 0;

	unlock_it.l_type = F_UNLCK;
	unlock_it.l_whence = SEEK_SET;
	unlock_it.l_start = 0;
	unlock_it.l_len = 0;
}

void my_lock_wait()
{
	int rc;
	while((rc = fcntl(lock_fd, F_SETLKW, &lock_it)) < 0)
	{
		if(errno == EINTR) continue;
		else err_sys("fcntl error for my_lock_wait");
	}
}

void my_lock_release()
{
	if(fcntl(lock_fd, F_SETLKW, &unlock_it) < 0)
		err_sys("fcntl error for my_lock_release");
}

static pthread_mutex_t *mptr;
void my_lock_init2(char *pathname)
{
	int fd;
	pthread_mutexattr_t mattr;

	fd = open("/dev/zero", O_RDWR, 0);
	mptr = (pthread_mutex_t *)mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, fd, 0);
	close(fd);

	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(mptr, &mattr);
}

void my_lock_wait2()
{
	pthread_mutex_lock(mptr);
}

void my_lock_release2()
{
	pthread_mutex_unlock(mptr);
}

typedef struct{
	pid_t child_pid;
	int child_pipefd;
	int child_status;
	long child_count;
}Child;

Child *cptr;

pid_t child_make2(int i, int listenfd, int addrlen)
{
	int sockfd[2];
	pid_t pid;
	socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);

	if((pid = fork()) < 0) err_sys("fork error");
	else if(pid > 0)
	{
		close(sockfd[1]);
		cptr[i].child_pid = pid;
		cptr[i].child_pipefd = sockfd[0];
		cptr[i].child_status = 0;

		return pid;
	} 

	dup2(sockfd[1], STDERR_FILENO);
	close(sockfd[0]);
	close(sockfd[1]);

	close(listenfd);
	child_main3(i, listenfd, addrlen);
}


void child_main3(int i, int listenfd, int addrlen)
{
	char c; ssize_t n;
	int connfd;
	socklen_t clilen;

	printf("child %ld starting\n", (long)getpid());

	while(true)
	{
		if((n = read_fd(STDERR_FILENO, &c, 1, &connfd)) == 0) 
			err_quit("read_fd returned 0");

		if(connfd < 0) err_quit("no descriptor from read_fd");

		web_child(connfd);
		close(connfd);

		write(STDERR_FILENO, "", 1);
	}
}

int main(int argc, char*argv[])
{
	int listenfd, i;
	socklen_t addrlen;
	void sig_int(int);
	pid_t child_make(int, int, int);

	if(argc == 3) listenfd = tcp_listen(NULL, argv[1], &addrlen);
	else if(argc == 4) listenfd = tcp_listen(argv[1], argv[2], &addrlen);
	else err_quit("usage");

	nchildren = atoi(argv[argc-1]);
	pids = (pid_t*)calloc(nchildren, sizeof(pid_t));

	my_lock_init("/tmp/lock.XXXXXX");

	for(i = 0;i < nchildren;i++)
		pids[i] = child_make(i, listenfd, addrlen);

	signal(SIGINT, sig_int);
	while(true) pause();

	exit(0);
}

static int nchildren;

int main2(int argc, char*argv[])
{
	int listenfd, i, navail, maxfd, nsel, connfd, rc;
	socklen_t addrlen, clilen;
	struct sockaddr *cliaddr;
	ssize_t n; 
	fd_set rset, masterset;

	if(argc == 3) listenfd = tcp_listen(NULL, argv[1], &addrlen);
	else if(argc == 4) listenfd = tcp_listen(argv[1], argv[2], &addrlen);
	else err_quit("usage");

	FD_ZERO(&masterset);
	FD_SET(listenfd, &masterset);
	maxfd = listenfd;
	cliaddr = (struct sockaddr *)malloc(addrlen);

	navail = nchildren;
	nchildren = atoi(argv[argc-1]);
	pids = (pid_t*)calloc(nchildren, sizeof(pid_t));

	for(i = 0;i < nchildren;i++)
	{
		pids[i] = child_make(i, listenfd, addrlen);
		FD_SET(cptr[i].child_pipefd, &masterset);
		maxfd = max(maxfd, cptr[i].child_pipefd);
	}

	signal(SIGINT, sig_int);
	while(true)
	{
		rset = masterset;
		if(navail <= 0) FD_CLR(listenfd, &rset);

		nsel = select(maxfd + 1, &rset, NULL, NULL, NULL);
		if(FD_ISSET(listenfd, &rset))
		{
			clilen = addrlen;
			connfd = accpet(listenfd, cliaddr, &clilen);

			for(i=0;i<nchildren;i++)
			{
				if(cptr[i].child_status == 0)
					break;
			}

			if(i==nchildren) err_quit("no available children");

			cptr[i].child_status = 1;
			cptr[i].child_count++;
			navail--;			

			n = write_fd(cptr[i].child_pipefd, "", 1, connfd);
			close(connfd);
			if(--nsel == 0) continue;
		}

		for(i=0;i<nchildren;i++)
		{
			if(FD_ISSET(cptr[i].child_pipefd, &rset))
			{
				if((n = read(cptr[i].child_pipefd, &rc, 1)) == 0)
					err_quit("child %d terminated unexpectedly", i);
				
				cptr[i].child_status = 0;
				navail++;
				if(--nsel == 0) break;
			}
		}
	}
	exit(0);
}