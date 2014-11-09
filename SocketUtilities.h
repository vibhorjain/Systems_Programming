#ifndef SOCK_UTIL_INCLUDED
#define SOCK_UTIL_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

class SocketUtilities{

public:
static int make_socket_non_blocking (int sfd);
static int create_and_bind (char *port);

};

#endif
