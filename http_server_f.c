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

#include "http_server_f.h"
#include "http_common.h"

/*
* The main program function for the running of the function
*/

int main(int argc, char *argv[])
{
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;

	resume = 1;
	hup = 0;
	struct sigaction pSig;

	pSig.sa_handler = handler;
	sigemptyset(&pSig.sa_mask);
	pSig.sa_flags = SA_SIGINFO;
	sigaction(SIGINT, &pSig, NULL);
	sigaction(SIGHUP, &pSig, NULL);


	//Default Values PATH = ~/ and PORT=10000
	char *PORT = calloc(10, sizeof(char));

	ROOT = calloc(99999, sizeof(char));

	strncpy(ROOT, getenv("PWD"), 99998);
	strncpy(PORT, "8000", 10);
	FILE *logFile = stdout;

	verbose = 0;
	int slot = 0;

	if (!argv[1])
		parser(PORT, logFile);

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

	fd_set set;
	int stat;
	int pos;

	// ACCEPT connections
	while (resume) {
		if (hup) {
			shutdown(listenfd, SHUT_RDWR);
			parser(PORT, logFile);
			set_port(PORT, ROOT, logFile);
			hup = 0;
		}
		addrlen = sizeof(clientaddr);

		FD_ZERO(&set);
		FD_SET(listenfd, &set);

		if (select(FD_SETSIZE, &set, NULL, NULL, NULL) == 1) {
			clients[slot] = accept(listenfd,
				(struct sockaddr *) &clientaddr, &addrlen);

			if (clients[slot] < 0)
				error("accept() error");

			else
				if (fork() == 0) {
					respond(slot, logFile);
					resume = 0;
				}
		}

		while (waitpid(-1, &stat, WNOHANG) > 0) {
			pos = WEXITSTATUS(stat);
			clients[pos] = -1;
		}
		while (clients[slot] != -1)
			slot = (slot+1)%CONNMAX;
	}

	fclose(logFile);
	free(PORT);
	free(ROOT);
	return slot;
}

