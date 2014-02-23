#include "../include/unp.h"

void dg_cli(int sockfd, const SA *pservaddr, socklen_t servlen)
{
    int n;
    char sendline[MAXLINE], recvline[MAXLINE];

    while(fgets(sendline, MAXLINE, stdin) != NULL)
    {
        Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);

        if(Readable_timeo(sockfd, 5) == 0)
            fprintf(stderr, "socket timeout\n");
        else
        {
            n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
            recvline[n] = 0;
            puts(recvline);
        }
    }
}

void dg_cli2(int sockfd, const SA *pservaddr, socklen_t servlen)
{
    int n;
    char sendline[MAXLINE], recvline[MAXLINE + 1];

    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while(fgets(sendline, MAXLINE, stdin) != NULL)
    {
        Sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
        n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
        if(n < 0)
        {
            if(errno == EWOULDBLOCK)
            {
                fprintf(stderr, "socket timeout\n");
                continue;
            }
            else err_sys("Recvfrom error");
        }
    }
    recvline[n] = 0;
    puts(recvline);
}

void str_echo(int sockfd)
{
    char line[MAXLINE];
    FILE *fpin, *fpout;

    fpin = fdopen(sockfd, "r");
    if(fpin == NULL) err_sys("fdopen error");

    fpout = fdopen(sockfd, "w");
    if(fpout == NULL) err_sys("fdopen error");

    while(fgets(line,MAXLINE, fpin) != NULL)
        fputs(line, fpout);
}