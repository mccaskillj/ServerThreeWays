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

#include "http_server_t.h"
#include "http_common.h"

/*
* The main program function for the running of the function
*/

int main(int argc, char *argv[])
{
	struct threadArgs *tArgs = calloc(10, sizeof(struct threadArgs));
	struct sockaddr_in clientaddr;
	socklen_t addrlen;
	char c;

	resume = 1;
	hup = 0;
	pthread_t *threads = calloc(10, sizeof(threads));
	FILE *logFile = stdout;

	struct sigaction pSig;

	memset(&pSig, '\0', sizeof(pSig));

	pSig.sa_handler = handler;
	sigemptyset(&pSig.sa_mask);
	sigaction(SIGINT, &pSig, NULL);
	sigaction(SIGHUP, &pSig, NULL);

	//Default Values PATH = ~/ and PORT=10000
	char *PORT = calloc(10, sizeof(char));

	ROOT = calloc(99999, sizeof(char));

	strcpy(ROOT, getenv("PWD"));
	strncpy(PORT, "8000", 10);

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
	addrlen = 0;

	create(threads, tArgs, clientaddr, addrlen, logFile);

	// ACCEPT connections
	while (resume) {
		if (hup) {
			destroy(threads);
			close(listenfd);
			die = 0;
			parser(PORT, logFile);
			set_port(PORT, ROOT, logFile);
			addrlen = 0;
			create(threads, tArgs, clientaddr, addrlen, logFile);
			hup = 0;
		}
	}

	destroy(threads);
	free(threads);
	free(tArgs);
	free(ROOT);
	free(PORT);
	fclose(logFile);
	return 0;
}

void create(pthread_t *threads, struct threadArgs *tArgs,
	struct sockaddr_in clientaddr, socklen_t addrlen,
	FILE *logFile)
{
	int i;
	int rc;

	for (i = 0; i < 10; i++) {
		tArgs[i].arg1 = i;
		tArgs[i].arg2 = clientaddr;
		tArgs[i].arg3 = addrlen;
		tArgs[i].arg4 = &logFile;
		rc = pthread_create(&threads[i], NULL,
			thread_loop, (void *)&tArgs[i]);
		if (rc) {
			printf("ERROR; pthread_create(): %d\n", rc);
			exit(-1);
		}
	}
}

void destroy(pthread_t *threads)
{
	shutdown(listenfd, SHUT_RDWR);

	int i;

	for (i = 0; i < 10; ++i)
		pthread_join(threads[i], NULL);

}

void *thread_loop(void *argum)
{
	struct threadArgs *tArgs = argum;
	int slot = tArgs->arg1;
	struct sockaddr_in clientaddr = tArgs->arg2;
	socklen_t addrlen = tArgs->arg3;
	FILE **logFile = tArgs->arg4;

	addrlen = sizeof(clientaddr);

	fd_set set;

	FD_ZERO(&set);

	while (!die) {

		clients[slot] = accept(listenfd,
			(struct sockaddr *) &clientaddr, &addrlen);
		if  (errno == EINTR)
			shutdown(listenfd, SHUT_RDWR);

		if (clients[slot] >= 0) {
			FD_SET(clients[slot], &set);
			while (fcntl(clients[slot], F_GETFD) >= 0)
				respond(slot, *logFile);
		}
		FD_ZERO(&set);
	}
	pthread_exit(NULL);
}
