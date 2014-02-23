#include "../include/unp.h"

#include "server.h"

static int nchildren;
static pid_t *pids;

void  child_main(int i, int listenfd, int addrlen);
void  child_main2(int i, int listenfd, int addrlen);

void sig_int(int signo)
{
    int i;

    for(i=0; i<nchildren; i++)
        kill(pids[i], SIGTERM);
    while(wait(NULL) > 0);

    if(errno != ECHILD) err_sys("wait error");

    pr_cpu_time();
    exit(0);
}

pid_t child_make(int i, int listenfd, int addrlen)
{
    pid_t pid;

    if((pid = fork()) < 0)
        err_sys("fork error");
    else if(pid > 0)
        return pid;

    child_main(i, listenfd, addrlen);

    return pid;
}

/**All proceses waited for the connection**/
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
        Select(listenfd + 1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(listenfd, &rset) == 0)
            err_quit("listenfd readable");

        clilen = addrlen;

        my_lock_wait();
        connfd = Accept(listenfd, cliaddr, &clilen);
        my_lock_release();

        web_child(connfd);
        close(connfd);
    }
}

int main(int argc, char*argv[])
{
    int listenfd, i;
    socklen_t addrlen;

    if(argc == 3)
        listenfd = Tcp_listen(NULL, argv[1], &addrlen);
    else if(argc == 4)
        listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
    else
        err_quit("usage");

    nchildren = atoi(argv[argc-1]);
    pids = (pid_t*)calloc(nchildren, sizeof(pid_t));

    my_lock_init("/tmp/lock.XXXXXX");

    for(i = 0; i < nchildren; i++)
        pids[i] = child_make(i, listenfd, addrlen);

    signal(SIGINT, sig_int);
    while(true)
        pause();

    exit(0);
}

typedef struct
{
    pid_t child_pid;
    int child_pipefd;
    int child_status;
    long child_count;
} Child;

Child *cptr;

pid_t child_make2(int i, int listenfd, int addrlen)
{
    int sockfd[2];
    pid_t pid;
    socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);

    if((pid = fork()) < 0)
        err_sys("fork error");
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
    child_main2(i, listenfd, addrlen);

    return pid;
}

void child_main2(int i, int listenfd, int addrlen)
{
    char c;
    ssize_t n;
    int connfd;

    printf("child %ld starting\n", (long)getpid());

    while(true)
    {
        if((n = Read_fd(STDERR_FILENO, &c, 1, &connfd)) == 0)
            err_quit("read_fd returned 0");

        if(connfd < 0)
            err_quit("no descriptor from read_fd");

        web_child(connfd);
        close(connfd);

        write(STDERR_FILENO, "", 1);
    }
}

int main2(int argc, char*argv[])
{
    int listenfd, i, navail, maxfd, nsel, connfd, rc;
    socklen_t addrlen, clilen;
    struct sockaddr *cliaddr;
    fd_set rset, masterset;

    size_t n;

    if(argc == 3)
        listenfd = Tcp_listen(NULL, argv[1], &addrlen);
    else if(argc == 4)
        listenfd = Tcp_listen(argv[1], argv[2], &addrlen);
    else
        err_quit("usage");

    FD_ZERO(&masterset);
    FD_SET(listenfd, &masterset);
    maxfd   = listenfd;
    cliaddr = (struct sockaddr *)malloc(addrlen);

    navail    = nchildren;
    nchildren = atoi(argv[argc-1]);
    pids      = (pid_t*)calloc(nchildren, sizeof(pid_t));

    for(i = 0; i < nchildren; i++)
    {
        pids[i] = child_make2(i, listenfd, addrlen);
        FD_SET(cptr[i].child_pipefd, &masterset);
        maxfd = max(maxfd, cptr[i].child_pipefd);
    }

    signal(SIGINT, sig_int);

    while(true)
    {
        rset = masterset;
        if(navail <= 0)
            FD_CLR(listenfd, &rset);

        nsel = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(listenfd, &rset))
        {
            clilen = addrlen;
            connfd = Accept(listenfd, cliaddr, &clilen);

            for(i=0; i < nchildren; i++)
            {
                if(cptr[i].child_status == 0)
                    break;
            }

            if(i==nchildren)
                err_quit("no available children");

            cptr[i].child_status = 1;
            cptr[i].child_count++;
            navail--;

            n = Write_fd(cptr[i].child_pipefd, NULL, 1, connfd);

            close(connfd);
            if(--nsel == 0)
                continue;
        }

        for(i=0; i<nchildren; i++)
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