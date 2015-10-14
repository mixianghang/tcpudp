/***********************************************
*
*Filename: tcpclient.c
*
*@author: Xianghang Mi
*@email: mixianghang@outlook.com
*@description: ---
*Create: 2015-09-18 18:32:32
*Last Modified: 2015-09-18 18:32:32
************************************************/

#include "tcpclient.h"

int main(int argc, char* argv[]) {
	int clientSockFd;
	struct sockaddr_in serverAddr_in;
	int len_sockaddr_in = 0;
	int serverIp;
	int serverPort;
	enum KeepAlive isAlive = no_persistent;
	char filePath[256];
	char tempFilePath[256];
	char tempFilePath1[256];
	char tempFilePath2[256];
	char serverIpStr[20];
	struct timeval startTime;
	struct timeval endTime;
	gettimeofday(&startTime, NULL);
	
	clientSockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (argc < 5) {
		error("usage: tcpclient hostname port isPersistent filename\n");
		return 1;
	}
	len_sockaddr_in = sizeof( struct sockaddr_in);
	serverIp = inet_addr(argv[1]);
	serverPort = atoi(argv[2]);
	serverAddr_in.sin_family      = AF_INET;
	serverAddr_in.sin_port        = htons(serverPort);
	serverAddr_in.sin_addr.s_addr = serverIp;
	memset(serverIpStr, 0, sizeof(serverIpStr));
	inet_ntop(AF_INET, &(serverAddr_in.sin_addr), serverIpStr, 19);
	if (strcmp(argv[3],"p") == 0) {
	  isAlive = persistent;
	}
	if (connect(clientSockFd,(struct sockaddr*) &serverAddr_in,(socklen_t)len_sockaddr_in) != 0){
	  printf("failed to connect to server %s:%d \n", serverIpStr, serverPort);
	  shutdown(clientSockFd, SHUT_RDWR);
	  close(clientSockFd);
	  return 1;
	} else {
	  printf("connect to server %s:%d successfully\n", serverIpStr, serverPort);
	}
	memset(filePath, '\0', sizeof(filePath));
	snprintf(filePath, sizeof(filePath)-1, "./%s",argv[4]);
	if (isAlive == persistent) {
	  if (checkFileExist(filePath)) {
		  FILE * fp;
		  fp = fopen(filePath, "r");
		  if (fp == NULL) {
			  printf("open filelist file %s failed, please check file permission\n", filePath);
			  shutdown(clientSockFd, SHUT_RDWR);
			  close(clientSockFd);
			  return 1;
		  } else {
			  memset(tempFilePath, '\0', sizeof(tempFilePath));
			  memset(tempFilePath1, '\0', sizeof(tempFilePath1));
			  memset(tempFilePath2, '\0', sizeof(tempFilePath2));
			  printf("http request using persistent connection\n");
			  if (fgets(tempFilePath, sizeof(tempFilePath)-1, fp) == NULL) {
				printf("no content in the filelist file %s\n", filePath);
				shutdown(clientSockFd, SHUT_RDWR);
				close(clientSockFd);
				return 1;
			  }
			  while (1) {
				  if (fgets(tempFilePath1, sizeof(tempFilePath1)-1, fp) == NULL) {
					isAlive = no_persistent;
				  }
				  tempFilePath[strlen(tempFilePath) - 1] = '\0';
				  //snprintf(tempFilePath2, sizeof(tempFilePath2) - 1, "./%s", tempFilePath);
				  printf("start send http request to download  file %s ...\n",tempFilePath);
				  downloadSingleFile(tempFilePath, clientSockFd, isAlive);
				  if (isAlive == no_persistent) {
					break;
				  } else {
					memset(tempFilePath, 0, sizeof tempFilePath);
					snprintf(tempFilePath, sizeof(tempFilePath) - 1, "./%s", tempFilePath1);
					printf("start to download file %s\n", tempFilePath);
				  }
				  memset(tempFilePath1, 0, sizeof tempFilePath1);
				  memset(tempFilePath2, '\0', sizeof(tempFilePath2));
			  }
		  }
	  } else {
		  printf("cannot find file %s in the current dir\n", argv[4]);
		  shutdown(clientSockFd, SHUT_RDWR);
		  close(clientSockFd);
		  return 1;
	  }
	} else {
		printf("using non-persistent connection to download file %s ...\n",argv[4]);
		downloadSingleFile(filePath,clientSockFd, isAlive);
	}
	shutdown(clientSockFd, SHUT_RDWR);
	close(clientSockFd);
	gettimeofday(&endTime, NULL);
	int timecost = (endTime.tv_sec - startTime.tv_sec) * 1000000 + (endTime.tv_usec - startTime.tv_usec);
	printf(" timecost is %d us\n", timecost);
	return 0;
}
