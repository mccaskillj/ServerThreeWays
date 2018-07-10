/*
* Jordan McCaskill
* 1722645
* Holds the includes and function prototypes for http_serve.c
*/

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

void returnVal(int *fdHigh, fd_set *set,
	fd_set *activeSet, int *slot,
	socklen_t *addrlen, struct sockaddr_in *clientaddr,
	FILE *logFile);
