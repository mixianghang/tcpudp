/***********************************************
*
*Filename: udpserver.c
*
*@author: Xianghang Mi
*@email: mixianghang@outlook.com
*@description: ---
*Create: 2015-09-19 10:45:09
*Last Modified: 2015-09-19 10:45:09
************************************************/
#include "udpclient.h"

int main(int argc, char* argv[]){
  int sockFd;
  struct sockaddr_in servAddr;
  socklen_t addrLen;
  int serverPort;
  fd_set readSet;
  fd_set originReadSet;
  fd_set writeSet;
  fd_set originWriteSet;
  const char NOT_FOUND[] = "<html><head></head><body><h1>404, NOT FOUND!</h1></body></html>";
  const char BAD_REQUEST[] = "<html><head></head><body><h1>400, BAD REQUEST!</h1></body></html>";

  if (argc < 2) {
	printf("usage: /udpclient serverPort\n");
	printf("--serverPort the port of server\n");
	return 1;
  }
  sockFd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockFd <= 0) {
	printf("%s %s create udp socket failed\n", __FILE__, __func__);
	return 1;
  }
  addrLen                  = sizeof (struct sockaddr_in);
  serverPort               = atoi(argv[1]);
  servAddr.sin_port        = htons(serverPort);
  servAddr.sin_family      = AF_INET;
  servAddr.sin_addr.s_addr = INADDR_ANY;
  if (bind(sockFd, (struct sockaddr *) &servAddr, addrLen) != 0) {
	printf("%s %s failed to bind to port %d\n", __FILE__, __func__, serverPort);
	shutdown(sockFd, SHUT_RDWR);
	close(sockFd);
	return 1;
  } else {
	printf("%s %s succeed to bind to port %d\n", __FILE__, __func__, serverPort);
  }

  FD_ZERO(&originReadSet);
  FD_ZERO(&originWriteSet);
  FD_SET(sockFd, &originReadSet);
  FD_SET(sockFd, &originWriteSet);
  int maxFd = sockFd + 1;
  struct timeval waitInterval;
  waitInterval.tv_sec = 0;
  waitInterval.tv_usec = 50000;
  while (1) {
	readSet = originReadSet;
	select(maxFd, &readSet, NULL, NULL, &waitInterval);
	if (!FD_ISSET(sockFd, &readSet)) {
	  continue;
	}
	char clientIp[20];
	int  clientPort;
	struct sockaddr_in clientAddr;
	int  recvLen;
	char buffer[1500];
	RequestLine requestLine;
	memset(buffer, 0, sizeof buffer);
	if ((recvLen = recvfrom(sockFd, buffer, sizeof buffer -1, 0, (struct sockaddr *)&clientAddr, &addrLen)) <= 0) {
	  printf("%s %s receive request failed\n", __FILE__, __func__);
	  shutdown(sockFd, SHUT_RDWR);
	  close(sockFd);
	  return 1;
	}


	if (recvLen <= 2 || parseHttpRequestLine(buffer, recvLen - 2, &requestLine) == -1) {
	  fprintf(stderr, " parse request line error\n %s", buffer);
	  memset(buffer, 0, sizeof buffer);
	  sprintf(buffer, "HTTP/1.0 400 Bad Request\n\r");
	  if (sendto(sockFd, buffer, strlen(buffer), 0, (struct sockaddr *) &clientAddr, addrLen) <= 0) {
		printf("failed to send response to client: %s\n", buffer); 
		shutdown(sockFd, SHUT_RDWR);
		close(sockFd);
		return 1;
	  }

	  // send blank line which separate the header and the content
	  memset(buffer, 0, sizeof buffer);
	  sprintf(buffer, "Content-Length: %lu\n\r",sizeof BAD_REQUEST);
	  if (sendto(sockFd, buffer, strlen(buffer), 0, (struct sockaddr *) &clientAddr, addrLen) <= 0) {
		printf("failed to send response to client: %s\n", buffer); 
		shutdown(sockFd, SHUT_RDWR);
		close(sockFd);
		return 1;
	  }

	  memset(buffer, 0, sizeof buffer);
	  sprintf(buffer, "\n\r");
	  if (sendto(sockFd, buffer, strlen(buffer),0,  (struct sockaddr *) &clientAddr, addrLen) <= 0) {
		printf("failed to send response to client: %s\n", buffer); 
		shutdown(sockFd, SHUT_RDWR);
		close(sockFd);
		return 1;
	  }

	  memset(buffer, 0, sizeof buffer);
	  sprintf(buffer, "%s", BAD_REQUEST);
	  if (sendto(sockFd, buffer, strlen(buffer), 0, (struct sockaddr *) &clientAddr, addrLen) <= 0) {
		printf("failed to send response to client: %s\n", buffer); 
		shutdown(sockFd, SHUT_RDWR);
		close(sockFd);
		return 1;
	  }

	  continue;
	}

	// recv header and blank line
	memset(buffer, 0, sizeof buffer);
	while ((recvLen = recvfrom(sockFd, buffer, sizeof buffer -1, 0, (struct sockaddr *)&clientAddr, &addrLen)) > 2) {
	  //process header
	}

	if (!checkFileExist(requestLine.path)) {
	  fprintf(stderr, "request file %s not exist\n", requestLine.path);
	  // send 404 not found
	  // send response status line
	  memset(buffer, 0, sizeof buffer);
	  sprintf(buffer, "HTTP/1.0 404 NOT FOUND\n\r");
	  if (sendto(sockFd, buffer, strlen(buffer), 0, (struct sockaddr *) &clientAddr, addrLen) <= 0) {
		printf("failed to send response to client: %s\n", buffer); 
		shutdown(sockFd, SHUT_RDWR);
		close(sockFd);
		return 1;
	  }

	  // send blank line which separate the header and the content
	  memset(buffer, 0, sizeof buffer);
	  sprintf(buffer, "Content-Length: %lu\n\r",sizeof NOT_FOUND);
	  if (sendto(sockFd, buffer, strlen(buffer), 0, (struct sockaddr *) &clientAddr, addrLen) <= 0) {
		printf("failed to send response to client: %s\n", buffer); 
		shutdown(sockFd, SHUT_RDWR);
		close(sockFd);
		return 1;
	  }

	  memset(buffer, 0, sizeof buffer);
	  sprintf(buffer, "\n\r");
	  if (sendto(sockFd, buffer, strlen(buffer), 0, (struct sockaddr *) &clientAddr, addrLen) <= 0) {
		printf("failed to send response to client: %s\n", buffer); 
		shutdown(sockFd, SHUT_RDWR);
		close(sockFd);
		return 1;
	  }

	  memset(buffer, 0, sizeof buffer);
	  sprintf(buffer, "%s", NOT_FOUND);
	  if (sendto(sockFd, buffer, strlen(buffer), 0, (struct sockaddr *) &clientAddr, addrLen) <= 0) {
		printf("failed to send response to client: %s\n", buffer); 
		shutdown(sockFd, SHUT_RDWR);
		close(sockFd);
		return 1;
	  }
	  continue;
	}

	int fileSize = getFileSize(requestLine.path);
	clientPort = ntohs(clientAddr.sin_port);
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIp, 19);
	printf(" %s %s start to response request from %s:%d for file %s\n",__FILE__, __func__,  clientIp, clientPort, requestLine.path);

	// send 200 response to client 
	memset(buffer, 0, sizeof buffer);
	sprintf(buffer, "HTTP/1 200 OK\r\n");
	if (sendto(sockFd, buffer, strlen(buffer), 0, (struct sockaddr *) &clientAddr, addrLen) <= 0) {
	  printf("failed to send response to client: %s\n", buffer); 
	  shutdown(sockFd, SHUT_RDWR);
	  close(sockFd);
	  return 1;
	}

	// send content-length header to client
	memset(buffer, 0, sizeof buffer);
	sprintf(buffer, "Content-Length: %d\r\n", fileSize);
	if (sendto(sockFd, buffer, strlen(buffer), 0, (struct sockaddr *) &clientAddr, addrLen) <= 0) {
	  printf("failed to send response to client: %s\n", buffer); 
	  shutdown(sockFd, SHUT_RDWR);
	  close(sockFd);
	  return 1;
	}

	// send blank line to client
	memset(buffer, 0, sizeof buffer);
	sprintf(buffer, "\r\n");
	if (sendto(sockFd, buffer, strlen(buffer), 0, (struct sockaddr *) &clientAddr, addrLen) <= 0) {
	  printf("failed to send response to client: %s\n", buffer); 
	  shutdown(sockFd, SHUT_RDWR);
	  close(sockFd);
	  return 1;
	}

	//printf("%s %s send %d data %s\n", __FILE__, __func__, strlen(buffer), buffer);
	int len = 0;
	FILE * fp;
	fp = fopen(requestLine.path,"r");
	if (fp == NULL) {
	  printf("failed open file %s for reading\n", requestLine.path);
	  shutdown(sockFd, SHUT_RDWR);
	  close(sockFd);
	  return 1;
	}
	memset(buffer, 0, sizeof buffer);
	while((len = fread(buffer,sizeof(char), sizeof buffer -1,fp) ) > 0) {
	  if (sendto(sockFd, buffer, len, 0, (struct sockaddr *) &clientAddr, addrLen) != len) {
		printf("failed to send response to client: %s\n", buffer); 
		shutdown(sockFd, SHUT_RDWR);
		close(sockFd);
		return 1;
	  }
	  memset(buffer,0 ,sizeof buffer );
	}
	fclose(fp);
	printf(" %s %s finish responsing request from %s:%d for file %s\n",__FILE__, __func__,  clientIp, clientPort, requestLine.path);
	printf("ready to accept another request, if wanna stopping me, ctrl-c please!\n");
  }
}
