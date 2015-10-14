
/***********************************************
*
*Filename: tcpserver.c
*
*@author: Xianghang Mi
*@email: mixianghang@outlook.com
*@description: ---
*Create: 2015-09-18 11:14:44
*Last Modified: 2015-09-18 11:14:44
************************************************/

#include "tcpserver.h"
int main(int argc, char* argv[]) {

	int listenSockFd, listenPort;
	int acceptedSockFd;
	char readBuffer[MAX_READ_BUFFER];
	char fileNotExistMsg[] = " sorry, the file you request do not exist \n";
	char stopSock[] = "stop";
	int numHeaders = 0;

	//sockaddr_in struct
	//sa_family_t    sin_family
	//in_port_t      sin_port
	//struct in_addr sin_addr
	//unsigned char  sin_zero[8]
	struct sockaddr_in serverAddr_in;
	int len_sockaddr_in = sizeof(struct sockaddr_in);
	if (argc < 2) {
		error("usage: ./tcpserver port\n");
		exit(1);
	}
	listenSockFd = socket(AF_INET,SOCK_STREAM,0);//create socket 
	listenPort = atoi(argv[1]);
	printf("the listen port is %d \n", listenPort);
	memset((char*) &serverAddr_in, '\0', sizeof(serverAddr_in));
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
		error("bind to port failed \n");
		exit(1);
	}

	listen(listenSockFd,MAX_CONNECTION_QUEUE);
	char clientIp[20];
	int clientPort;
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof (struct sockaddr_in);
	acceptedSockFd = accept(listenSockFd, (struct sockaddr *)&clientAddr, &addrLen);
	if (acceptedSockFd >= 0) {
	  clientPort = ntohs(clientAddr.sin_port);
	  memset(clientIp, 0, sizeof clientIp);
	  inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, 19);
	  printf("start to serve request from client %s:%d\n", clientIp, clientPort);
	} else {
	  printf("failed to accept new request\n");
	  shutdown(listenSockFd, SHUT_RDWR);
	  close(listenSockFd);
	  return 1;
	}
	//accept new connection 
	while (1) {
		memset(readBuffer,'\0',sizeof(readBuffer));
		Request * request = (Request*) malloc(sizeof(Request));
		request->isResolved = 0;
		request->isAlive = no_persistent;
		request->content_len = 0;
		if (getLineFromSock(acceptedSockFd, readBuffer, sizeof(readBuffer)-1) > 0) {//read request line
			//parse requestline successfully
			if (parseHttpRequestLine(readBuffer, strlen(readBuffer), &(request->requestLine)) >= 0){
				printf("parsed requestline path is %s\n", request->requestLine.path);
				//read each header per line, end by a blank line
				numHeaders = 0;
				memset(readBuffer,'\0',sizeof(readBuffer));
				while (getLineFromSock(acceptedSockFd, readBuffer, sizeof(readBuffer)-1) > 0) { 
					request->headers[numHeaders] = (Header*) malloc(sizeof(Header));
					if ( parseHeader(readBuffer, strlen(readBuffer), request->headers[numHeaders]) > 0) {
						printf("parsed header name %s  value %s\n",request->headers[numHeaders]->name, request->headers[numHeaders]->value);
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
			printf("start to send reques file %s to client %s:%d\n", request->requestLine.path, clientIp, clientPort);
			//send requested file to client
			if (sendRequestedFile(acceptedSockFd, request) < 0) {
				printf("\nsend requested file %s  to client %s:%d failed\n", request->requestLine.path, clientIp, clientPort);
			} else {
				printf("\nsucceed to send requested file %s  to client %s:%d\n", request->requestLine.path, clientIp, clientPort);
			}
		} else {
		  printf("no request from client %s:%d for %d seconds, so close it\n", clientIp, clientPort, MAX_WAIT_SECONDS);
		  shutdown(acceptedSockFd, SHUT_RDWR);
		  close(acceptedSockFd);
		  if (request != NULL) {
			free(request);
		  }
		  printf("close connection from client %s:%d\n", clientIp, clientPort);
		  printf("ready to accept new connection, if you want to stop me, ctrl-C\n");
		  acceptedSockFd = accept(listenSockFd, (struct sockaddr *)&clientAddr, &addrLen);
		  if (acceptedSockFd >= 0) {
			clientPort = ntohs(clientAddr.sin_port);
			memset(clientIp, 0, sizeof clientIp);
			inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, 19);
			printf("start to serve request from client %s:%d\n", clientIp, clientPort);
		  } else {
			printf("failed to accept new request\n");
			shutdown(listenSockFd, SHUT_RDWR);
			close(listenSockFd);
			return 1;
		  }
		  continue;
		}


		enum KeepAlive isAlive = request->isAlive; 
		//free dynamically allocated memory
		int i;
		for (i = 0; i< numHeaders; i++) {
			free(request->headers[i]);
		}
		free(request);

		if (isAlive != persistent) {
		  shutdown(acceptedSockFd, SHUT_RDWR);
		  close(acceptedSockFd);
		  printf("close connection from client %s:%d\n", clientIp, clientPort);
		  printf("read to accept new connection, if you want to stop me, ctrl-C\n");
		  acceptedSockFd = accept(listenSockFd, (struct sockaddr *)&clientAddr, &addrLen);
		  if (acceptedSockFd >= 0) {
			clientPort = ntohs(clientAddr.sin_port);
			memset(clientIp, 0, sizeof clientIp);
			inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, 19);
			printf("start to serve request from client %s:%d\n", clientIp, clientPort);
		  } else {
			printf("failed to accept new request\n");
			shutdown(listenSockFd, SHUT_RDWR);
			close(listenSockFd);
			return 1;
		  }
		}
	}
	shutdown(listenSockFd,SHUT_RDWR);
	close(listenSockFd);
}

