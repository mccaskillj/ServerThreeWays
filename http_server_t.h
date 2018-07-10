/*
* Jordan McCaskill
* 1722645
* Holds the includes and function prototypes for http_serve.c
*/

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>

struct threadArgs {
	int arg1;
	struct sockaddr_in arg2;
	socklen_t arg3;

	FILE **arg4;
};

void *thread_loop(void *argum);
void create(pthread_t *threads, struct threadArgs *tArgs,
	struct sockaddr_in clientaddr, socklen_t addrlen,
	FILE *logFile);
void destroy(pthread_t *threads);
