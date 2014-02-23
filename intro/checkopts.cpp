#include "unp.h"

union val
{
    int i_val;
    long l_val;
    struct linger linger_val;
    struct timeval timeval_val;
} val;

static char *sock_str_flag(union val *, int);
static char *sock_str_int(union val *, int);
static char *sock_str_timeval(union val *, int);

struct sock_opts
{
    const char *opt_str;
    int opt_level;
    int opt_name;
    char *(*opt_val_str)(union val *, int);
} sock_opts[] =
{
    {"SO_BROADCAST", SOL_SOCKET, SO_BROADCAST, sock_str_flag},
    {"SO_DEBUG", SOL_SOCKET, SO_DEBUG, sock_str_flag},
    {"SO_ERROR", SOL_SOCKET, SO_ERROR, sock_str_int},
    {"SO_KEEPALIVE", SOL_SOCKET, SO_KEEPALIVE, sock_str_flag},
    {"SO_RCVBUF", SOL_SOCKET, SO_RCVBUF, sock_str_int},
    {"SO_SNDBUF", SOL_SOCKET, SO_SNDBUF, sock_str_int},
    {"SO_SNDTIMEO", SOL_SOCKET, SO_SNDTIMEO, sock_str_timeval},
    {"SO_TYPE", SOL_SOCKET, SO_TYPE, sock_str_int},
    {"IP_TTL", IPPROTO_IP, IP_TTL, sock_str_int},
    {"TCP_MAXSEG", IPPROTO_TCP, TCP_MAXSEG, sock_str_int},
    {"TCP_NODELAY", IPPROTO_TCP, TCP_NODELAY, sock_str_flag},
#ifdef	SCTP_MAXSEG
    { "SCTP_MAXSEG",		IPPROTO_SCTP,SCTP_MAXSEG,	sock_str_int },
#else
    { "SCTP_MAXSEG",		0,			0,				NULL },
#endif
#ifdef	SCTP_NODELAY
    { "SCTP_NODELAY",		IPPROTO_SCTP,SCTP_NODELAY,	sock_str_flag },
#else
    { "SCTP_NODELAY",		0,			0,				NULL },
#endif
    {NULL, 0, 0, NULL}
};

int main()
{
    int fd;
    socklen_t len;
    struct sock_opts *ptr;

    for(ptr = sock_opts; ptr -> opt_str != NULL; ptr++)
    {
        printf("%s: ", ptr->opt_str);
        if(ptr->opt_val_str == NULL)
            printf("(undefined)\n");
        else
        {
            switch(ptr->opt_level)
            {
            case SOL_SOCKET:
            case IPPROTO_IP:
            case IPPROTO_TCP:
                fd = socket(AF_INET, SOCK_STREAM, 0);
                break;
#ifdef IPPROTO_SCTP
            case IPPROTO_SCTP:
                fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
                break;
#endif
            default:
                err_quit("can't create fd for level %d\n", ptr->opt_level);
            }

            len = sizeof(val);
            if(getsockopt(fd, ptr->opt_level, ptr->opt_name, &val, &len) == -1)
                err_ret("getsockopt error");
            else
                printf("default = %s\n", (*ptr->opt_val_str)(&val,len));
            close(fd);
        }
    }
    exit(0);
}

static char strres[128];

static char * sock_str_flag(union val *ptr, int len)
{
    if(len != sizeof(int))
        snprintf(strres, sizeof(strres),"wrong type conversion");
    else
        snprintf(strres, sizeof(strres),"%s",(ptr->i_val == 0)?"off":"on");
    return strres;
}

static char	* sock_str_int(union val *ptr, int len)
{
    if (len != sizeof(int))
        snprintf(strres, sizeof(strres), "size (%d) not sizeof(int)", len);
    else
        snprintf(strres, sizeof(strres), "%d", ptr->i_val);
    return(strres);
}

static char	* sock_str_timeval(union val *ptr, int len)
{
    struct timeval	*tvptr = &ptr->timeval_val;

    if (len != sizeof(struct timeval))
        snprintf(strres, sizeof(strres),
                 "size (%d) not sizeof(struct timeval)", len);
    else
        snprintf(strres, sizeof(strres), "%d sec, %d usec",
                 (int)tvptr->tv_sec, (int)tvptr->tv_usec);
    return(strres);
}
