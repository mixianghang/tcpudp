/***********************************************
*
*Filename: udpclient.h
*
*@author: Xianghang Mi
*@email: mixianghang@outlook.com
*@description: ---
*Create: 2015-09-19 10:03:51
*Last Modified: 2015-09-19 10:03:51
************************************************/
#ifndef UDPCLIENT_H
#define UDPCLIENT_H

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
#define MAX_WAIT_NUM 40

#endif
