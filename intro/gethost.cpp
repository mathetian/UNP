#include "unp.h"

int main(int argc, char*argv[])
{
    char *ptr, **pptr;
    char str[INET_ADDRSTRLEN];
    struct hostent *hptr;

    if(argc == 1)
    {
        printf("Usage: gethost <www.baidu.com>\n");
        return 0;
    }

    while(--argc > 0)
    {
        ptr = *++argv;
        if((hptr = gethostbyname(ptr)) == NULL)
        {
            err_msg("gethostbyname error for host: %s : %s", ptr, hstrerror(h_errno));
            continue;
        }

        printf("officical hostname: %s\n",hptr->h_name);
        for(pptr = hptr -> h_aliases; *pptr != NULL; pptr++)
            printf("\t alases: %s\n", *pptr);
        switch(hptr -> h_addrtype)
        {
        case AF_INET:
            pptr = hptr -> h_addr_list;
            for(; *pptr != NULL; pptr++)
                printf("\taddress: %s\n",inet_ntop(hptr->h_addrtype,*pptr,str,sizeof(str)));
            break;
        default:
            err_ret("unknown address type");
            break;
        }
    }
    exit(0);
}