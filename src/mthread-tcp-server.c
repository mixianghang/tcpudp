/***********************************************
*
*Filename: mthread-tcp-server.c
*
*@author: Xianghang Mi
*@email: mixianghang@outlook.com
*@description: ---
*Create: 2015-09-19 16:41:42
*Last Modified: 2015-09-19 16:41:42
************************************************/

#include "mthread-tcp-server.h"

#if 0
//defined in <netinet/in.h>
struct sockaddr_in{
	sa_family_t    sin_family
	in_port_t      sin_port
	struct in_addr sin_addr
	unsigned char  sin_zero[8]
}
#endif

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
int numAcceptedConns = 0;

int main(int argc, char* argv[]) {

	int listenSockFd, listenPort;

	struct sockaddr_in serverAddr_in;
	int len_sockaddr_in = sizeof(struct sockaddr_in);
	if (argc < 2) {
		error("usage: ./mttserver port\n");
		exit(1);
	}
	listenSockFd = socket(AF_INET,SOCK_STREAM,0);//create socket 
	listenPort = atoi(argv[1]);
	printf("the listen port is %d \n", listenPort);
	memset(&serverAddr_in,'\0',len_sockaddr_in);
	serverAddr_in.sin_family         = AF_INET;
    serverAddr_in.sin_addr.s_addr    = INADDR_ANY;
	serverAddr_in.sin_port           = htons(listenPort);

	// set address reuse option
	int reuseOn = 1;
	if (setsockopt(listenSockFd, SOL_SOCKET, SO_REUSEADDR, &reuseOn, sizeof reuseOn) < 0) {
	  printf("set reuse option failed\n");
	  return 1;
	}
	
	if (bind(listenSockFd, (struct sockaddr *)&serverAddr_in, len_sockaddr_in) < 0) {
		printf("bind to port %d failed \n", listenPort);
		exit(1);
	} else {
	  printf("bind to port %d successfully\n", listenPort);
	}

	listen(listenSockFd,MAX_CONNECTION_QUEUE);
	while (1) {
		if (numAcceptedConns < MAX_CONNECTIONS) {
			socklen_t addrLen = sizeof(struct sockaddr_in);
			struct connInfo * tempConnInfo = (struct connInfo *) malloc(sizeof (struct connInfo));
			if (tempConnInfo == NULL) {
			  printf("create new connection failed\n");
			  shutdown(listenSockFd, SHUT_RDWR);
			  close(listenSockFd);
			  return 1;
			  continue;
			}
			if (numAcceptedConns == 0) {
			  printf("I am idle, waiting for new connections, if you wanna stop me, ctrl-c\n"); 
			}
			tempConnInfo->sockFd = accept(listenSockFd, (struct sockaddr *)&(tempConnInfo->addr), &addrLen);
			if (tempConnInfo->sockFd < 0) {
			  printf("create new connection faied\n");
			  shutdown(listenSockFd, SHUT_RDWR);
			  close(listenSockFd);
			  return 1;
			}
			pthread_mutex_lock(&mutex1);
			pthread_create(&(tempConnInfo->threadId), NULL, newsockthread, (void*)tempConnInfo);
			numAcceptedConns++;
			printf("accept a new conn, current conn num is %d\n",numAcceptedConns);
			pthread_mutex_unlock(&mutex1);
		}
	}
	shutdown(listenSockFd,SHUT_RDWR);
	close(listenSockFd);
	exit(0);
}

void * newsockthread(void* tempConnInfo) {
	struct connInfo * connectInfo = (struct connInfo*) tempConnInfo;
	char sendBuffer[MAX_SEND_BUFFER];
	char readBuffer[MAX_RECV_BUFFER];
	char clientIp[20];
	int clientPort;
	int acceptedSockFd  = connectInfo->sockFd;
	pthread_t threadId = connectInfo->threadId;
	memset(clientIp, 0, sizeof clientIp);
	clientPort = ntohs(connectInfo->addr.sin_port);
	inet_ntop(AF_INET, &(connectInfo->addr.sin_addr), clientIp, 19);
	printf("start to serve request from %s:%d\n", clientIp, clientPort);
	while(1) {
		int numHeaders = 0;
		memset(readBuffer,'\0',sizeof(readBuffer));
		Request * request = (Request*) malloc(sizeof(Request));
		request->isResolved = 0;
		request->isAlive = no_persistent;
		request->content_len = 0;
		if (getLineFromSock(acceptedSockFd, readBuffer, sizeof(readBuffer)-1) > 0) {//read request line
			//parse requestline successfully
			if (parseHttpRequestLine(readBuffer, strlen(readBuffer), &(request->requestLine)) >= 0){
				printf("parsed requestline path for %s:%d is %s\n", clientIp, clientPort, request->requestLine.path);
				//read each header per line, end by a blank line
				numHeaders = 0;
				memset(readBuffer,'\0',sizeof(readBuffer));
				while (getLineFromSock(acceptedSockFd, readBuffer, sizeof(readBuffer)-1) > 0) { 
					request->headers[numHeaders] = (Header*) malloc(sizeof(Header));
					if ( parseHeader(readBuffer, strlen(readBuffer), request->headers[numHeaders]) > 0) {
						printf("parsed header name %s  value %s from %s:%d\n",request->headers[numHeaders]->name, request->headers[numHeaders]->value, clientIp, clientPort);
						// get connect alive status
						if (strcmp(request->headers[numHeaders]->name, "Connection") == 0) {
							if (strcmp(request->headers[numHeaders]->value, "keep-alive") == 0) {
								request->isAlive = persistent;
							} else {
								request->isAlive = no_persistent;
							}
						}
						// get content length
						if (strcmp(request->headers[numHeaders]->name, "Content-Length") == 0) {
							request->content_len = atoi(request->headers[numHeaders]->value);
						}
						memset(readBuffer,'\0',sizeof(readBuffer));
						numHeaders++;
					} else {
						free(request->headers[numHeaders]);
						printf("pare header failed: \n%s\n", readBuffer);
						break;
					}
				}
				if ( numHeaders ) {
					request->isResolved = 1;
				}
				
			} 
			//send requested file to client
			if (sendRequestedFile(acceptedSockFd, request) < 0) {
				printf("\nsend requested file %s failed for client %s:%d\n", request->requestLine.path, clientIp, clientPort);
			} else {
				printf("\nsend requested file %s successfully for client %s:%d\n", request->requestLine.path, clientIp, clientPort);
			}
		} else {
		  printf(" \nno request from client %s:%d for %d seconds, so close it\n", clientIp, clientPort, MAX_WAIT_SECONDS);
		  if (request != NULL) {
			free(request);
		  }
		  if (connectInfo != NULL) {
			free(connectInfo);
		  }
		  shutdown(acceptedSockFd, SHUT_RDWR);
		  close(acceptedSockFd);
		  printf("\nclose connection from client %s:%d\n",clientIp, clientPort);
		  pthread_mutex_lock(&mutex1);
		  numAcceptedConns--;
		  printf("current connections num is %d\n",numAcceptedConns);
		  if (numAcceptedConns == 0) {
			printf("I am idle, waiting for new connections, if you wanna stop me, ctrl-c\n"); 
		  }
		  pthread_mutex_unlock(&mutex1);
		  break;
		}


		enum KeepAlive isAlive = request->isAlive; 
		//free dynamically allocated memory
		int i;
		for (i = 0; i< numHeaders; i++) {
			free(request->headers[i]);
		}
		free(request);

		if (isAlive != persistent) {
			if (connectInfo != NULL) {
			  free(connectInfo);
			}
			shutdown(acceptedSockFd, SHUT_RDWR);
			close(acceptedSockFd);
			printf("\nclose connection from client %s:%d\n",clientIp, clientPort);
			pthread_mutex_lock(&mutex1);
			numAcceptedConns--;
			printf("current connections num is %d\n",numAcceptedConns);
			if (numAcceptedConns == 0) {
			  printf("I am idle, waiting for new connections, if you wanna stop me, ctrl-c\n"); 
			}
			pthread_mutex_unlock(&mutex1);
			break;
		}
	}

	return NULL;
}
/**
*used to process new connection in new thread
*/
void * sockthread(void * tempConnInfo) {
	struct connInfo * connectInfo = (struct connInfo*) tempConnInfo;
	char sendBuffer[MAX_SEND_BUFFER];
	char recvBuffer[MAX_RECV_BUFFER];
	char * filePath = connectInfo->filePath;
	int sockFd  = connectInfo->sockFd;
	pthread_t threadId = connectInfo->threadId;
	int sendLen = 0;
	int recvLen = 0;
	printf("start to process connection %d \n", sockFd);
	memset(recvBuffer, '\0', MAX_RECV_BUFFER);
	memset(sendBuffer, '\0', MAX_SEND_BUFFER);
	while (1) {
		recvLen = recv(sockFd, recvBuffer, sizeof(recvBuffer), 0);
		if (recvLen < 0) {
			printf("recv failed on sockFd %d\n", sockFd);
			break;
		}
		printf("recieve msg successfully on sockFd %d\n with msg \n %s \n", sockFd, recvBuffer);

		if (strcmp(recvBuffer,"stop") == 0) {
			break;
		}
		if (transferFile(filePath, sockFd) != 0){
			printf("send failed on sockFd %d\n", sockFd);
			break;
		} else {
			printf("send successfully on sockFd %d\n", sockFd);
		}
		memset(recvBuffer, '\0', MAX_RECV_BUFFER);
		memset(sendBuffer, '\0', MAX_SEND_BUFFER);
	}
	shutdown(sockFd, SHUT_RD);
	pthread_mutex_lock(&mutex1);
	numAcceptedConns--;
	printf("current connections num is %d\n",numAcceptedConns);
	pthread_mutex_unlock(&mutex1);
	return NULL;
}

