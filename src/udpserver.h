/***********************************************
*
*Filename: udpserver.h
*
*@author: Xianghang Mi
*@email: mixianghang@outlook.com
*@description: ---
*Create: 2015-09-19 10:40:30
*Last Modified: 2015-09-19 10:40:30
************************************************/
#ifndef UDPSERVER_H
#define UDPSERVER_H
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

#define BUFFER_SIZE 1500

#endif
