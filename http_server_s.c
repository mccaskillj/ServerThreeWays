/*
* Code is pulled from:
* http://blog.abhijeetr.com/2010/04/very-simple-http-server-writen-in-c.html
* and modified to remove the fork command to make it single threaded
* instead of multi-threaded
*
* AUTHOR: Abhijeet Rastogi (http://www.google.com/profiles/abhijeet.1989)
*
* This is a very simple HTTP server. Default port is 10000 and ROOT for
* the server is your current working directory..
*
* You can provide command line arguments like:- $./a.aout -p [port] -v
*
* for ex.
* $./a.out -p 50000
* to start a server at port 50000
*/

/*
* Jordan McCaskill
* 1722645
* CMPT360
* Program to create a simple single threaded http server
*/

#include "http_server_s.h"
#include "http_common.h"


int main(int argc, char *argv[])
{
	struct sockaddr_in clientaddr;
	socklen_t addrlen = 0;
	char c;

	resume = 1;
	hup = 0;

	struct sigaction pSig;

	//memset(&pSig, '\0', sizeof(pSig));

	pSig.sa_handler = handler;
	sigemptyset(&pSig.sa_mask);
	pSig.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &pSig, NULL);
	sigaction(SIGHUP, &pSig, NULL);


	//Default Values PATH = ~/ and PORT=10000
	char *PORT = calloc(10, sizeof(char));

	ROOT = calloc(99999, sizeof(char));

	strcpy(ROOT, getenv("PWD"));
	strncpy(PORT, "8000", 10);
	FILE *logFile = stdout;

	verbose = 0;
	int slot = 0;

	if (!argv[1])
		logFile = parser(PORT, logFile);

	//Parsing the command line arguments
	while ((c = getopt(argc, argv, "p:v")) != -1)
		switch (c) {
		case 'v':
			verbose = 1;
			break;
		case 'p':
			strncpy(PORT, optarg, 10);
			break;
		case '?':
			fprintf(stderr, "Wrong arguments given!!!\n");
			exit(1);
		default:
			exit(1);
		}

	set_port(PORT, ROOT, logFile);

	fd_set set, activeSet;
	int retval;
	int fdHigh = listenfd+1;

	FD_ZERO(&activeSet);
	FD_SET(listenfd, &activeSet);

	// ACCEPT connections
	while (resume) {
		if (hup) {
			shutdown(listenfd, SHUT_RDWR);
			parser(PORT, logFile);
			set_port(PORT, ROOT, logFile);
			FD_ZERO(&activeSet);
			FD_SET(listenfd, &activeSet);
			hup = 0;
		}

		set = activeSet;
		retval = select(fdHigh, &set, NULL, NULL, 0);
		if (resume) {
			if (retval == -1)
				retval = -1;
			else if (retval) {
				returnVal(&fdHigh, &set, &activeSet,
					&slot, &addrlen, &clientaddr, logFile);
			}
		}

	}

	free(ROOT);
	fclose(logFile);
	free(PORT);
	return 0;
}

void returnVal(int *fdHigh, fd_set *set,
	fd_set *activeSet, int *slot,
	socklen_t *addrlen, struct sockaddr_in *clientaddr,
	FILE *logFile)
{
	int i;

	for (i = 0; i < *fdHigh; ++i) {
		if (FD_ISSET(i, set)) {
			if (i == listenfd) {
				*addrlen = sizeof(*clientaddr);
				clients[*slot] = accept(listenfd,
					(struct sockaddr *) clientaddr,
					addrlen);
				if (clients[*slot] < 0) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				if (clients[*slot]+1 > *fdHigh)
					*fdHigh = clients[*slot]+1;

				FD_SET(clients[*slot], activeSet);
			} else {
				clients[*slot] = i;
				respond(0, logFile);
				close(i);
				FD_CLR(i, activeSet);
			}
		}
	}
}
