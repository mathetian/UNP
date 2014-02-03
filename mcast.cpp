#include "unp.h"
#include <net/if.h>

int mcast_join(int sockfd, const SA *grp, socklen_t grplen, const char *ifname, u_int ifindex)
{
#ifdef MCAST_JOIN_GROUP
	struct group_req req;
	if(ifindex > 0) req.gr_interface = ifindex;
	else if(ifname != NULL)
	{
		if((req.gr_interface = if_nametoindex(ifname)) == 0)
		{
			errno = ENXIO;
			return -1;
		}
	}
	else req.gr_interface = 0;

	if(grplen > sizeof(req.gr_group))
	{
		errno = EINVAL;
		return -1;
	}
	memcpy(&req.gr_group, grp, grplen);
	return (setsockopt(sockfd, family_to_level(grp->sa_family), MCAST_JOIN_GROUP, &req, sizeof(req)));
#else
	
#endif
}