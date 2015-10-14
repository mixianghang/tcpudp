
/***********************************************
*
*Filename: tcpclient.h
*
*@author: Xianghang Mi
*@email: mixianghang@outlook.com
*@description: ---
*Create: 2015-09-18 10:55:40
*Last Modified: 2015-09-18 10:55:40
************************************************/
#ifndef TCPCLIENT_H
#define TCPCLIENT_H

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
#include <sys/time.h>
#include "util.h"
#include "httputil.h"

#define CLIENT_READ_BUFFER 1024
#define CLIENT_WRITE_BUFFER 1024

#endif
