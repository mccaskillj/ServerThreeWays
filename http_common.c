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

#include "http_common.h"

void handler(int signo)
{
	switch (signo) {
	case SIGINT:
		die = 1;
		resume = 0;
		break;
	case SIGHUP:
		die = 1;
		hup = 1;
		break;
	default: /*Should never get this case*/
		break;
	}
}

/*
* The function which starts the server running and listening on a
* specific port
*/
void startServer(char *port)
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *p;

	// getaddrinfo for host
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, port, &hints, &res) != 0) {
		perror("getaddrinfo() error");
		resume = 0;
		return;
	}
	// socket and bind
	for (p = res; p != NULL; p = p->ai_next) {
		listenfd = socket(p->ai_family, p->ai_socktype, 0);
		if (listenfd == -1)
			continue;
		if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
			break;
	}
	if (p == NULL) {
		perror("socket() or bind()");
		resume = 0;
		return;
	}

	freeaddrinfo(res);

	// listen for incoming connections
	if (listen(listenfd, 1000000) != 0) {
		perror("listen() error");
		resume = 0;
		return;
	}
}

/*
* Function which responds to the client after a request is made
*/
void respond(int n, FILE *logFile)
{
	char *mesg = calloc(99999, sizeof(char));
	char *path = calloc(99999, sizeof(char));
	char *data_to_send = calloc(BYTES, sizeof(char));
	char **reqline = calloc(3, sizeof(char *));

	reqline[0] = calloc(99999, sizeof(char));
	reqline[1] = calloc(99999, sizeof(char));
	reqline[2] = calloc(99999, sizeof(char));

	int rcvd;

	memset((void *)mesg, (int)'\0', 99999);

	rcvd = recv(clients[n], mesg, 99999, 0);

	if (rcvd < 0)    // receive error
		fprintf(stderr, ("recv() error\n"));
	else if (rcvd == 0)   // receive socket closed
		fprintf(stderr, "Client disconnected upexpectedly.\n");
	else {
		strncpy(reqline[0], strtok(mesg, " \t\n"), 99999);
		if (strncmp(reqline[0], "GET\0", 4) == 0)
			respond_inner(reqline, path,
				data_to_send, n, logFile);
		else{
			write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
			if (verbose)
				fflush(logFile);
				fprintf(logFile, "GET %s [400 Bad Request]\n",
				reqline[1]);
		}
	}

	free(reqline[2]);
	free(reqline[1]);
	free(reqline[0]);
	free(reqline);
	free(data_to_send);
	free(path);
	free(mesg);

	//Closing SOCKET
	//All further send and recieve operations are DISABLED...
	shutdown(clients[n], SHUT_RDWR);
	close(clients[n]);
	clients[n] = -1;
}

/*
*Function to remove some of the nesting in the respond function
*/
void respond_inner(char **reqline, char *path,
	char *data_to_send, int n, FILE *logFile)
{
	char *token;
	int fd, bytes_read;

	token = strtok(NULL, " \t");
	if (token != NULL) {
		strncpy(reqline[1], token, 99999);
		//if (reqline[1][strlen(reqline[1])-1] == '\n')
		//  reqline[1][strlen(reqline[1])-1] = "\0";
	}

	token = strtok(NULL, " \t\n");
	if (token != NULL)
		strncpy(reqline[2], token, 99999);

	if (strncmp(reqline[2], "HTTP/1.0", 8) != 0 &&
		strncmp(reqline[2], "HTTP/1.1", 8) != 0) {
		write(clients[n], "HTTP/1.0 400 Bad Request\n", 25);
		if (verbose) {
			fflush(logFile);
			fprintf(logFile, "GET %s [400 Bad Request]\n",
				reqline[1]);
		}
	} else {
		if (strncmp(reqline[1], "/\0", 2) == 0)
			reqline[1] = "/index.html";

		strncpy(path, ROOT, 99999);
		strncpy(&path[strlen(ROOT)], reqline[1], 99999 - strlen(ROOT));

		fd = open(path, O_RDONLY);
		if (fd != -1) {
			send(clients[n], "HTTP/1.0 200 OK\n\n", 17, 0);
			if (verbose) {
				fflush(logFile);
				fprintf(logFile, "GET %s [200 OK]\n",
					reqline[1]);
			}
			while ((bytes_read =
				read(fd, data_to_send, BYTES)) > 0)
				write(clients[n], data_to_send, bytes_read);
		} else
			error_check(reqline, n, logFile);
	}
}

/*
* This function was added in order to implement the ability to tell
* if the error thrown was a 403 or a 404 error.
*/
void error_check(char **reqline, int n, FILE *logFile)
{
	if (errno == ENOENT) {
		//FILE NOT FOUND
		write(clients[n], "HTTP/1.0 404 Not Found\n", 23);
		if (verbose) {
			fflush(logFile);
			fprintf(logFile, "GET %s [404 Not Found]\n",
				reqline[1]);
		}
	} else if (errno == EACCES) {
		write(clients[n], "HTTP/1.0 403 Access Denied\n", 27);
		if (verbose) {
			fflush(logFile);
			fprintf(logFile, "GET %s [403 Access Denied]\n",
				reqline[1]);
		}
	}
}

FILE *parser(char *port, FILE *logFile)
{
	FILE *file = fopen("server.conf", "rt");
	char *input = calloc(99999, sizeof(char));

	if (file == NULL) {
		fflush(logFile);
		fprintf(logFile, "File not found. Using Default values\n");
		return stdout;
	}

	purge_extra(5, file);
	fgets(input, 9, file);
	strncpy(port, input, strlen(input)-1);

	purge_extra(8, file);
	fgets(input, 99998, file);
	strncpy(ROOT, input, strlen(input)-1);

	purge_extra(8, file);
	fgets(input, 99998, file);
	if (input[strlen(input)-1] == '\n')
		input[strlen(input)-1] = '\0';
	logFile = fopen(input, "w+");
	freopen(input, "w+", stdout);
	verbose = 1;

	if (file != NULL)
		fclose(file);

	hup = 0;

	return logFile;
}

void purge_extra(int x, FILE *file)
{
	int i;

	for (i = 0; i < x; i++)
		fgetc(file);
}

void set_port(char *PORT, char *ROOT, FILE *logFile)
{
	if (verbose) {
		fflush(logFile);
		fprintf(logFile, "Server started at port no. %s ", PORT);
		fflush(logFile);
		fprintf(logFile, "with root directory as %s\n", ROOT);
	}

	// Setting all elements to -1: signifies there is no client connected
	int i;

	for (i = 0; i < CONNMAX; i++)
		clients[i] = -1;

	startServer(PORT);
}
