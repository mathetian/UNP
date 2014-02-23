#ifndef _SERVER_H
#define _SERVER_H

#include "../include/unp.h"

#define	MAXN	16384		/* max # bytes client can request */

void 	pr_cpu_time();
void    web_child(int sockfd);

void 	my_lock_init(const char *pathname);
void 	my_lock_wait();
void 	my_lock_release();
void 	my_lock_init2(const char *pathname);
void 	my_lock_wait2();
void 	my_lock_release2();

#endif