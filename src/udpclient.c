#include "udpclient.h"

int main(int argc, char* argv[]) {
	int clientSockFd;
	int serverPort;
	struct sockaddr_in addr;
	socklen_t addrLen;
	char buffer[BUFFER_SIZE];
	int  len = 0;
	fd_set readSet, originReadSet;
	fd_set writeSet, originWriteSet;
	struct timeval startTime;
	struct timeval endTime;
	gettimeofday(&startTime, NULL);
	if (argc < 4) {
	  printf("usage: /udpclient serverIp serverPort requestFile [localFileName]\n");
	  printf("--serverIp: the ip address of the udp server\n");
	  printf("--serverPort: the port of server\n");
	  printf("--requestFile: the file name to request\n");
	  printf("--localFileName: optional, the file name where you want to save the response\n");
	  exit(1);
	}
	clientSockFd         = socket(AF_INET, SOCK_DGRAM, 0);
	serverPort           = atoi(argv[2]);
	addrLen              = sizeof(struct sockaddr_in);
	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(serverPort);
	inet_pton(AF_INET, argv[1], &(addr.sin_addr));

	char ip[20] = {0};
	int convertedPort = ntohs(addr.sin_port);
	inet_ntop(AF_INET, &(addr.sin_addr), ip, 19);
	printf("%s %s start to request file %s from server %s:%d\n", __FILE__, __func__, argv[3], ip, convertedPort);
	FD_ZERO(&readSet);
	FD_ZERO(&originReadSet);
	FD_ZERO(&writeSet);
	FD_ZERO(&originWriteSet);
	FD_SET(clientSockFd, &originReadSet);
	FD_SET(clientSockFd, &originWriteSet);
	struct timeval waitInterval;
	waitInterval.tv_sec  = 0;
	waitInterval.tv_usec = 50000;// wait for 50 ms
	int maxFd = clientSockFd + 1;

	writeSet = originWriteSet;
	select(maxFd, NULL, &writeSet, NULL, &waitInterval);
	if (!FD_ISSET(clientSockFd, &writeSet)) {
	  printf(" the socket is not ready to write\n");
	  shutdown(clientSockFd, SHUT_RDWR);
	  close(clientSockFd);
	  return 1;
	}
	memset(buffer, 0, sizeof buffer);
	sprintf(buffer, "GET %s HTTP/1.1\r\n", argv[3]);
	printf("%s", buffer);
	if ((len = sendto(clientSockFd, buffer, strlen(buffer), 0,  (struct sockaddr *)&addr, addrLen)) != strlen(buffer)) {
	  printf("line %d: send request line  failed, dataLen %lu, sendLen %d\n", __LINE__, strlen(buffer), len);
	  shutdown(clientSockFd, SHUT_RDWR);
	  close(clientSockFd);
	  return 1;
	} else {
	  printf("send request line successfully with %d bytes\n", len);
	}

	memset(buffer, 0, sizeof buffer);
	sprintf(buffer, "\r\n");
	if ((len = sendto(clientSockFd, buffer, strlen(buffer), 0,  (struct sockaddr *)&addr, addrLen)) != strlen(buffer)) {
	  printf("line %d: send blank line  failed, dataLen %lu, sendLen %d\n", __LINE__, strlen(buffer), len);
	  shutdown(clientSockFd, SHUT_RDWR);
	  close(clientSockFd);
	  return 1;
	}

	memset(buffer, 0, sizeof buffer);
	// the response line should have more than just /r/n characters
	if ((len = recvfrom(clientSockFd, buffer, sizeof buffer - 1, 0,  (struct sockaddr *)&addr, &addrLen)) <= 2) {
	  printf("line %d: receive response line failed \n ", __LINE__);
	  shutdown(clientSockFd, SHUT_RDWR);
	  close(clientSockFd);
	  return 1;
	}

	ResponseLine responseLine;

	if (parseHttpResponseLine(buffer, len - 2, &responseLine) != 0) {
	  printf("line %d: parse response line failed, recved data is %s \n ", __LINE__, buffer);
	  shutdown(clientSockFd, SHUT_RDWR);
	  close(clientSockFd);
	  return 1;
	}

	printf("recved response status line as:\n %s", buffer);
	printf("start to recv response headers...\n");

	int contentLen = -1;
	memset(buffer, 0, sizeof buffer);
	while ((len = recvfrom(clientSockFd, buffer, sizeof buffer - 1, 0,  (struct sockaddr *)&addr, &addrLen)) > 2) {
	  Header header;
	  parseHeader(buffer, len - 2, &header); //the last two chars are \r\n
	  if (strcmp(header.name, "Content-Length") == 0) {
		contentLen = atoi(header.value);
	  }
	  memset(buffer, 0, sizeof buffer);
	}

	printf("start to recv response content...\n");

	int waitNum  = 0;
	int tempLen  = 0;
	int leftLen  = contentLen;
	FILE *localFile = NULL;
	if (argc >= 5) {
	  localFile = fopen(argv[4], "w+");
	}
	while (leftLen > 0) {
	  readSet = originReadSet;
	  select(maxFd, &readSet, NULL, NULL, &waitInterval);
	  if (!FD_ISSET(clientSockFd, &readSet)) {
		waitNum++;
		if (waitNum >= MAX_WAIT_NUM) {// only wait consecutive MAX_WAIT_NUM
		  shutdown(clientSockFd, SHUT_RDWR);
		  close(clientSockFd);
		  printf("wait too long without data coming, maybe data lost\n");
		  break;
		}
		continue;
	  } else {
		waitNum = 0;
	  }
	  if (leftLen >= sizeof buffer - 1) {
		tempLen = sizeof buffer - 1;
	  } else {
		tempLen = leftLen;
	  }
	  memset(buffer, 0, sizeof buffer);
	  if ((len = recvfrom(clientSockFd, buffer, tempLen, 0,  (struct sockaddr *)&addr, &addrLen)) <= 0) {
		printf("%d recv data failed \n", __LINE__);
		if (localFile != NULL) {
		  fclose(localFile);
		}
		shutdown(clientSockFd, SHUT_RDWR);
		close(clientSockFd);
		break;
	  }
	  if (localFile != NULL) {
		printf("recved %d bytes into file %s\n", len, argv[4]);
		fprintf(localFile, "%s", buffer);
	  } else {
		printf("%s", buffer);
	  }
	  leftLen -= len;
	}

	if (leftLen <= 0) {
	  printf("successfully recved all the %d bytes data from server %s:%d for file %s\n", contentLen, ip, convertedPort, argv[3]);
	} else {
	  printf("falied recved all the %d , but %d bytes data from server %s:%d for file %s\n", contentLen, contentLen - leftLen,  ip, convertedPort, argv[3]);
	}
	if (localFile != NULL) {
	  fclose(localFile);
	}
	shutdown(clientSockFd,SHUT_RDWR);
	close(clientSockFd);
	gettimeofday(&endTime, NULL);
	int timecost = (endTime.tv_sec - startTime.tv_sec) * 1000000 + (endTime.tv_usec - startTime.tv_usec);
	printf(" timecost is %d us\n", timecost);
	return 0;
}
