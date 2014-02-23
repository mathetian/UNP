#include "../include/unp.h"

#define SERV_PORT 8086

void sig_chld(int signo)
{
    pid_t pid;
    int stat;

    pid = wait(&stat);
    printf("child %d terminated\n", pid);
    return;

}

void str_echo(int sockfd)
{
    ssize_t n;
    char buf[MAXLINE];

    while((n=read(sockfd, buf, MAXLINE)) != 0)
    {
        if(n < 0 && errno == EINTR)
            continue;
        else if(n < 0)
            break;
        Writen(sockfd, buf, n);
    }

    if(n < 0) err_sys("str_echo: read_error");
}

int main(int argc, char*argv[])
{
    int listenfd, connfd;
    struct sockaddr_in cliaddr, servaddr;
    socklen_t clilen;
    pid_t childpid;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (SA*)&servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    clilen = sizeof(cliaddr);

    printf("Begin Listen in the port %d\n", SERV_PORT);

    signal(SIGCHLD, sig_chld);

    while(true)
    {
        connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);

        if((childpid = fork()) < 0)
            err_sys("Fork error");
        else if(childpid == 0)
        {
            close(listenfd);
            printf("Start connection with %d\n",connfd);
            str_echo(connfd);
            printf("End connection with %d\n", connfd);
            close(connfd);
            exit(0);
        }
        close(connfd);
    }
    return 0;
}