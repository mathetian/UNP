#include "unpifi.h"

struct ifi_info *get_ifi_info(int family, int doaliases)
{
	struct ifi_info *ifi, *ifihead, **ifipnext;

	int sockfd, len, lastlen, flags, myflags, idx = 0, hlen = 0;
	char *ptr, *buf, lastname[IFNAMSIZ], *cptr, *haddr, *sdlname;
	struct ifconf ifc;
	struct ifreq *ifr, ifrcopy;

	struct sockaddr_in *sinptr;
	struct sockaddr_in6 *sin6ptr;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	lastlen = 0;
	len = 100*sizeof(struct ifreq);

	while(true)
	{
		buf = (char*)malloc(len);
		if(buf == NULL) err_sys("malloc error");
		ifi.ifc_len = len;
		ifi.ifc_buf = buf;

		if(ioctl(sockfd, SIOCGIFCONF, &ifc) < 0)
		{
			if(errno != EINVAL || lastlen != 0)
				err_sys("ioctl error");
		}
		else
		{
			if(ifc.ifc_len == lastlen) break;
			lastlen = ifc.ifc_len;
		}
		len += 10*sizeof(struct ifreq);
		free(buf);
	}

	ifihead = NULL;
	ifipnext = &ifihead;
	lastname[0] = 0;
	sdlname = NULL;

	for(ptr = buf;ptr < buf + ifc.ifc_len;)
	{
		ifr = (struct ifreq *)ptr;
		len = max(sizeof(struct sockaddr), ifr->ifr_addr.sa_len);
		ptr += sizeof(ifr->ifr_name) + len;
	
		if(ifr->ifr_addr.sa_family != family) continue;
		myflags = 0;

		if((cptr = strchr(ifr->ifr_name, ':')) != NULL) *cptr = 0;
		if(strncmp(lastname, ifr->ifr_name, IFNAMSIZ) == 0)
		{
			if(doaliases == 0) continue;
			myflags = IFI_ALIAS;
		}
		memcpy(lastname, ifr->ifr_name, IFNAMSIZ);

		ifrcopy = *ifr;
		ioctl(sockfd, SIOCGIFCONF, &ifrcopy);
		flags = ifrcopy.ifr_flags;

		if((flags & IFF_UP) == 0) continue;
	}
}