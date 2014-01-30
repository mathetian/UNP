#ifndef _UNP_IFI_H
#define _UNP_IFI_H

#include "unp.h"
#include <net/if.h>

#define IFI_NAME 16
#define IFI_HADDR 8

struct ifi_info{
	char ifi_name[IFI_NAME];
	short ifi_index;
	short ifi_mtu;
	u_char ifi_haddr[IFI_HADDR];
	u_short ifi_hlen;

	short ifi_flags;
	short ifi_myflags;

	struct sockaddr *ifi_addr;
	struct sockaddr *ifi_brdaddr;
	struct sockaddr *ifi_dstaddr;
	struct sock
};
#endif