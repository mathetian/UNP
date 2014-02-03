#include "unp.h"
#include <pthread.h>

typedef struct{
	pthread_t thread_tid;
	long thread_count;
}Thread;

Thread *tptr;

#define MAXNCLI 32
int clifd[MAXNCLI], iget, iput;

pthread_mutex_t clifd_mutex;
pthread_cond_t clifd_cond;

int nthreads;

clifd_mutex = PTHREAD_MUTEX_INITIALIZER;
clifd_cond  =  PTHREAD_COND_INITIALIZER;


void thread_make(int i)
{
	pthread_create(&tptr[i].thread_tid, NULL, &thread_main, NULL);
}

void thread_main(void *arg)
{
	int connfd;
	printf("thread %d starting\n", (int)arg);
	while(true)
	{
		pthread_mutex_lock(&clifd_mutex);

		while(iget == iput) pthread_cond_wait(&clifd_cond, &clifd_mutex);
		connfd = clifd[iget];
		if(++iget == MAXNCLI) iget = 0;

		pthread_mutex_unlock(&clifd_mutex);
		tptr[(int)arg].thread_count++;

		web_clild(connfd);
		close(connfd);
	}	
}

int main(int argc, char*argv[])
{
	int i, listenfd, connfd;
	socklen_t addrlen, clilen;
	struct sockaddr *cliaddr;


	if(argc == 3) listenfd = tcp_listen(NULL, argv[1], &addrlen);
	else if(argc == 4) listenfd = tcp_listen(argv[1], argv[2], &addrlen);
	else err_quit("usage");

	cliaddr = (sruct sockaddr *)malloc(addrlen);
	nthreads = atoi(argv[argc-1]);
	tptr = (struct Thread*)calloc(nthreads, sizeof(Thread));
	iget = iput = 0;

	for(i = 0;i < nthreads;i++)
		thread_make(i);

	signal(SIGINT, sig_int);

	while(true)
	{
		clilen = addrlen;
		connfd = accept(listenfd, cliaddr, &clilen);

		pthread_mutex_lock(&clifd_mutex);
		clifd[iput] = connfd;

		if(++iput == MAXNCLI) iput = 0;
		if(iput == iget) err_quit("iput = iget = %d", iput);

		pthread_cond_signal(&clifd_cond);
		pthread_mutex_unlock(&clifd_mutex);
	}
	exit(0);
}