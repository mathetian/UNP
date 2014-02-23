#include "../include/unp.h"

#include "server.h"

typedef struct
{
    pthread_t thread_tid;
    long thread_count;
} Thread;

Thread *tptr;

#define MAXNCLI 32
int clifd[MAXNCLI], iget, iput;

pthread_mutex_t clifd_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  clifd_cond  = PTHREAD_COND_INITIALIZER;

int nthreads;

void* thread_main(void *arg);

void thread_make(int & i)
{
    pthread_create(&tptr[i].thread_tid, NULL, &thread_main, &i);
}

void* thread_main(void *arg)
{
    int connfd, thrid = *(int*)arg;
    printf("thread %d starting\n", thrid);
    while(true)
    {
        pthread_mutex_lock(&clifd_mutex);

        while(iget == iput)
            pthread_cond_wait(&clifd_cond, &clifd_mutex);
        connfd = clifd[iget];
        if(++iget == MAXNCLI)
            iget = 0;

        pthread_mutex_unlock(&clifd_mutex);
        tptr[thrid].thread_count++;

        web_child(connfd);
        close(connfd);
    }

    return NULL;
}

void sig_int(int signo)
{
    int		i;

    pr_cpu_time();

    for (i = 0; i < nthreads; i++)
        printf("thread %d, %ld connections\n", i, tptr[i].thread_count);

    exit(0);
}

int main(int argc, char*argv[])
{
    int i, listenfd, connfd;
    socklen_t addrlen, clilen;
    struct sockaddr *cliaddr;

    if(argc == 3)
        listenfd = Tcp_listen(NULL, argv[1], &addrlen);
    else if(argc == 4)
        listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
    else
        err_quit("Usage");

    cliaddr  = (struct sockaddr *)malloc(addrlen);
    nthreads = atoi(argv[argc-1]);
    tptr     = (Thread*)calloc(nthreads, sizeof(Thread));

    iget = iput = 0;

    int *args = new int[nthreads];
    for(i = 0; i < nthreads; i++)
    {
        args[i] = i;
        thread_make(args[i]);
    }

    signal(SIGINT, sig_int);

    while(true)
    {
        clilen = addrlen;
        connfd = Accept(listenfd, cliaddr, &clilen);

        pthread_mutex_lock(&clifd_mutex);
        clifd[iput] = connfd;

        if(++iput == MAXNCLI)
            iput = 0;
        if(iput == iget)
            err_quit("iput = iget = %d", iput);

        pthread_cond_signal(&clifd_cond);
        pthread_mutex_unlock(&clifd_mutex);
    }
    exit(0);
}