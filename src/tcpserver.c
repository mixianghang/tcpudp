
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
	
	if (bind(listenSockFd, (struct sockaddr *)&serverAddr_in, len_sockaddr_in) < 0) {
		error("bind to port failed \n");
		exit(1);
	}

	listen(listenSockFd,MAX_CONNECTION_QUEUE);
	acceptedSockFd = accept(listenSockFd, NULL, NULL);
	//accept new connection 
	while(1){
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
			//send requested file to client
			if (sendRequestedFile(acceptedSockFd, request) < 0) {
				printf("send requested file failed\n");
			}
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
			acceptedSockFd = accept(listenSockFd, NULL, NULL);
		}
	}
	shutdown(listenSockFd,SHUT_RDWR);
	close(listenSockFd);
}

