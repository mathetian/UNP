#include "unp.h"

size_t Readn(int fd, void * vptr, size_t n)
{
	size_t nleft;
	size_t nread;
	char * ptr;

	ptr = (char*)vptr;
	nleft = n;
	while(nleft>0)
	{
		if((nread=read(fd,ptr,nleft))<0)
		{
			if(errno == EINTR) nread = 0;
			else return -1;
		}
		else if(nread == 0)
			break;
		
		nleft -= nread;
		ptr += nread;
	}
	return n - nleft;
}

size_t Writen(int fd, const void * vptr, size_t n)
{
	size_t nleft;
	size_t nwritten;
	const char * ptr;

	ptr = (char*)vptr;
	nleft = n;
	while(nleft > 0)
	{
		if((nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if(nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}

	return n;
}

static int read_cnt = 0;
static char * read_ptr;
static char read_buf[MAXLINE];

static size_t my_read(int fd, char * ptr)
{
	if(read_cnt <= 0)
	{
		while((read_cnt=read(fd, read_buf, sizeof(read_buf)))<0)
		{
			if(errno == EINTR)
				continue;
			return -1;
		}

		if(read_cnt == 0)
			return 0;
		read_ptr = read_buf;
	}

	read_cnt--;
	*ptr = *read_ptr++;
	return 1;
}

ssize_t Readline(int fd,void*vptr,size_t maxlen)
{
	ssize_t n, rc;
	char c, *ptr;

	ptr = (char*)vptr;
	for(n=1;n<maxlen;n++)
	{
		if((rc=my_read(fd,&c))==1)
		{
			*ptr++=c;
			if(c=='\n') break;
		}
		else if(rc==0)
		{
			*ptr=0;
			return n-1;
		}
		else return -1;
	}
	*ptr=0;
	return n;
}