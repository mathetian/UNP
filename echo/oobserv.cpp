#include "unp.h"

int connfd, listenfd;

void sig_urg(int signo)
{
    int n;
    char buff[100];

    printf("SIGURG received\n");
    n = recv(connfd, buff, sizeof(buff) - 1, MSG_OOB);
    buff[n] = 0;
    printf("read %d OOB byte: %s\n", n, buff);
}

int serv(int argc, char *argv[])
{
    int n;
    char buff[100];
    if(argc == 2)
        listenfd = Tcp_listen(NULL, argv[1], NULL);
    else if(argc == 3)
        listenfd = Tcp_listen(argv[1], argv[2], NULL);
    else
        err_quit("usage: oobserv [<host>][<port#>]");

    connfd = Accept(listenfd, NULL, NULL);
    signal(SIGURG, sig_urg);
    fcntl(connfd, F_SETOWN, getpid());

    while(true)
    {
        if((n = read(connfd, buff, sizeof(buff) - 1)) == 0)
        {
            printf("received EOF\n");
            exit(0);
        }
        buff[n] = 0;
        printf("read %d bytes\n", n);
    }
}

int main(int argc, char *argv[])
{
    serv(argc, argv);
    return 0;
}

