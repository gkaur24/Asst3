#ifndef _LIBNETFILES_H
#define _LIBNETFILES_H

#include <sys/types.h>


#define PORT_NUM 11130
#define unrestricted 0
#define exclusive 1
#define transaction 2


int cliSockFD, isInitialized, serv_mode;
struct addrinfo *cliList;  //was servinfo, 

int netopen(const char *pathname, int flags);

ssize_t netread(int fildes, void *buf, size_t nbyte);

ssize_t netwrite(int fildes, const void *buf, size_t nbyte);

int netclose(int fd);

int netserverinit(char * hostname,int filemode);

#endif