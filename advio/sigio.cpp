#include "../include/unp.h"

static int sockfd;

#define QSIZE 8
#define MAXDG 4096

typedef struct
{
    void  *   dg_data;
    size_t    dg_len;
    struct    sockaddr *dg_sa;
    socklen_t dg_salen;
} DG;

static DG dg[QSIZE];
static long cntread[QSIZE + 1];

static int iget;
static int iput;

static int nqueue;
static socklen_t chilen;

void sig_io(int)
{
    ssize_t len;
    int nread;
    DG *ptr;

    nread = 0;
    while(true)
    {
        if(nqueue >= QSIZE)
            err_quit("receive overflow");

        ptr = &dg[iput];
        ptr->dg_salen = chilen;

        len = recvfrom(sockfd, ptr->dg_data, MAXDG, 0, ptr->dg_sa, &ptr->dg_salen);

        if(len < 0)
        {
            if(errno == EWOULDBLOCK) break;
            else err_sys("recvfrom error");
        }

        ptr->dg_len = len;
        nread++;
        nqueue++;

        if(++iput >= QSIZE) iput = 0;
    }

    cntread[nread]++;
}

void sig_hup(int)
{
    int i;
    for(i = 0; i <= QSIZE; i++)
        printf("cntread[%d] = %ld\n", i, cntread[i]);
}

void dg_echo(int sockfd_arg, SA *pcliaddr, socklen_t chilen_arg)
{
    int i;
    const int on = 1;

    sigset_t zeromask, newmask, oldmask;

    sockfd = sockfd_arg;
    chilen = chilen_arg;

    for(i = 0; i < QSIZE; i++)
    {
        dg[i].dg_data = malloc(MAXDG);
        dg[i].dg_sa = (struct sockaddr *)malloc(chilen);
        dg[i].dg_salen = chilen;
    }

    iget = iput = nqueue = 0;
    signal(SIGHUP, sig_hup);
    signal(SIGIO, sig_io);

    fcntl(sockfd, F_SETOWN, getpid());
    ioctl(sockfd, FIOASYNC, &on);
    ioctl(sockfd, FIONBIO, &on);

    sigemptyset(&zeromask);
    sigemptyset(&oldmask);
    sigemptyset(&newmask);

    sigaddset(&newmask, SIGIO);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    while(true)
    {
        while(nqueue == 0)
            sigsuspend(&zeromask);

        sigprocmask(SIG_SETMASK, &oldmask, NULL);

        sendto(sockfd, dg[iget].dg_data, dg[iget].dg_len, 0, dg[iget].dg_sa, dg[iget].dg_salen);

        if(++iget >= QSIZE) iget = 0;

        sigprocmask(SIG_BLOCK, &newmask, &oldmask);
        nqueue--;
    }
}