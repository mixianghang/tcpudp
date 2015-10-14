#include <stdio.h>
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
#include <sys/time.h>

void * runCommand(void * cmd);
  
int main(int argc, char * argv[]) {
  char command[256] = {0};
  int threadNum = 0;
  struct timeval startTime;
  struct timeval endTime;
  pthread_t threads[256];


  gettimeofday(&startTime, NULL);
  if (argc < 3) {
	printf("Usage : ./test threadNum commandStr\n");
	return 1;
  }
  printf("command is %s\n", argv[2]);
  threadNum = atoi(argv[1]);
  int i = 0;
  for (i = 0; i< threadNum; i++) {
	pthread_create(&(threads[i]), NULL, runCommand, (void *) argv[2]);
  }
  for (i = 0; i< threadNum; i++) {
	pthread_join((threads[i]), NULL);
  }

  gettimeofday(&endTime, NULL);
  int timeCost = (endTime.tv_sec - startTime.tv_sec) * 1000000 + (endTime.tv_usec - startTime.tv_usec);
  printf("time cost of test is %d\n", timeCost);
  return 0;
}

void * runCommand(void * cmd) {
   system((char *) cmd);
   return NULL;
}
