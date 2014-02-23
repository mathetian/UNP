#include "../include/unp.h"

#define SERV_PORT 8085

int main(int argc, char*argv[])
{
    int listenfd, connfd, sockfd;

    int nready, client[FD_SETSIZE];
    int i, maxi, maxfd;

    ssize_t n;
    fd_set rset, allset;

    char buf[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    Bind(listenfd, (SA*)&servaddr, sizeof(servaddr));

    Listen(listenfd, LISTENQ);

    clilen = sizeof(cliaddr);

    printf("Begin Listen in the port %d\n", SERV_PORT);

    maxfd = listenfd;
    maxi  = -1;
    memset(client,-1,sizeof(client));

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    while(true)
    {
        rset   = allset;
        nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(listenfd, &rset))
        {
            connfd = Accept(listenfd, (SA*)&cliaddr, &clilen);

            for(i = 0; i < FD_SETSIZE; i++)
            {
                if(client[i] < 0)
                {
                    client[i] = connfd;
                    break;
                }
            }

            if(i == FD_SETSIZE)
                err_quit("too many client");

            FD_SET(connfd, &allset);

            maxi  = max(maxi, i);
            maxfd = max(maxfd, connfd);
            if(--nready <= 0)
                continue;
        }

        for(i = 0; i <= maxi; i++)
        {
            if((sockfd = client[i]) < 0)
                continue;

            if(FD_ISSET(sockfd, &rset))
            {
                if((n=read(sockfd, buf, MAXLINE)) == 0)
                {
                    /**connection closed by the client**/
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                }
                else
                    Writen(sockfd, buf, n);

                if(--nready <= 0) break;
            }
        }
    }
    return 0;
}