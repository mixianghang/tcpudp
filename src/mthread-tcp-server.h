/***********************************************
*
*Filename: mthread-tcp-server.h
*
*@author: Xianghang Mi
*@email: mixianghang@outlook.com
*@description: ---
*Create: 2015-09-19 16:41:15
*Last Modified: 2015-09-19 16:41:15
************************************************/

#ifndef MTHREAD_TCP_SERVER_H
#define MTHREAD_TCP_SERVER_H

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
#include <pthread.h>
#include "util.h"
#include "httputil.h"

#define MAX_CONNECTIONS 10
#define MAX_CONNECTION_QUEUE 5
#define MAX_RECV_BUFFER 1024
#define MAX_SEND_BUFFER 1024 

struct connInfo {
	struct sockaddr_in addr;
	int sockFd;
	pthread_t threadId;
	char * filePath;
};
void * sockthread(void * connnectInfo);
void * newsockthread(void * connectInfo);
#endif//tcpserver.h
