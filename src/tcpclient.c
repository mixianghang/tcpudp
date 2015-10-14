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
	
	clientSockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (argc < 5) {
		error("usage: tcpclient hostname port isPersistent filename\n");
	}
	len_sockaddr_in = sizeof( struct sockaddr_in);
	serverIp = inet_addr(argv[1]);
	serverPort = atoi(argv[2]);
	serverAddr_in.sin_family      = AF_INET;
	serverAddr_in.sin_port        = htons(serverPort);
	serverAddr_in.sin_addr.s_addr = serverIp;
	if (strcmp(argv[3],"p") == 0) {
		isAlive = persistent;
	}
	if (connect(clientSockFd,(struct sockaddr*) &serverAddr_in,(socklen_t)len_sockaddr_in) != 0){
		error("failed to connect\n");
		exit(1);
	}

	memset(filePath, '\0', sizeof(filePath));
	snprintf(filePath, sizeof(filePath)-1, "./%s",argv[4]);
	if (isAlive == persistent) {
		if (checkFileExist(filePath)) {
			FILE * fp;
			fp = fopen(filePath, "r");
			if (fp == NULL) {
				printf("open file %s failed, please check file permission\n", filePath);
			} else {
				memset(tempFilePath, '\0', sizeof(tempFilePath));
				memset(tempFilePath1, '\0', sizeof(tempFilePath1));
				printf("download file using persistent connection\n");
				while (fgets(tempFilePath, sizeof(tempFilePath)-1, fp)) {
					tempFilePath[strlen(tempFilePath) - 1] = '\0';
					snprintf(tempFilePath1, sizeof(tempFilePath1) - 1, "./%s", tempFilePath);
					printf("start download file %s ...\n",tempFilePath1);
					downloadSingleFile(tempFilePath1, clientSockFd, isAlive);
					memset(tempFilePath, '\0', sizeof(tempFilePath));
					memset(tempFilePath1, '\0', sizeof(tempFilePath1));
				}
			}
		} else {
			printf("cannot find file %s in the current dir\n", argv[4]);
		}
	} else {
		printf("start download file %s ...\n",argv[4]);
		downloadSingleFile(filePath,clientSockFd, isAlive);
	}
	shutdown(clientSockFd,SHUT_RDWR);
}
