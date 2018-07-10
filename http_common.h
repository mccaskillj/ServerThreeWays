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

#define CONNMAX 100
#define BYTES 1024

int die;
char *ROOT;
int listenfd, clients[CONNMAX];
int verbose;
int resume;
int hup;
int die;

void handler(int signo);
void error(char *);
void startServer(char *);
void respond(int, FILE *logFile);
void respond_inner(char **reqline, char *path, char *data_to_send,
	int n, FILE *logFile);
void error_check(char **reqline, int n, FILE *logFile);
FILE *parser(char *port, FILE *logFile);
void purge_extra(int x, FILE *file);
void set_port(char *PORT, char *ROOT, FILE *logFile);
