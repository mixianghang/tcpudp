/***********************************************
*
*Filename: tcpserver.h
*
*@author: Xianghang Mi
*@email: mixianghang@outlook.com
*@description: a tcp server support multithread + persistent and short-live connections
*Create: 2015-09-18 10:55:18
*Last Modified: 2015-09-18 10:55:18
************************************************/
#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "util.h"
#include "httputil.h"

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 10
#endif

#ifndef MAX_CONNECTION_QUEUE
#define MAX_CONNECTION_QUEUE 5
#endif

#ifndef MAX_READ_BUFFER
#define MAX_READ_BUFFER 1024
#endif

#ifndef MAX_WRITE_BUFFER
#define MAX_WRITE_BUFFER 1024
#endif

#endif//tcpserver.h
