# Flags for compiling
CC = gcc
CFLAGS = -Wall -std=c99

# Compile all programs
all: http_server_f http_server_t http_server_s

http_common.o: http_common.c http_common.h
	$(CC) -Wall http_common.c -c

http_server_f: http_server_f.c http_server_f.h http_common.o
	$(CC) -Wall http_server_f.c http_common.o -o http_server_f

http_server_t: http_server_t.c http_server_t.h http_common.o
	$(CC) -Wall http_server_t.c http_common.o -o http_server_t -pthread

http_server_s: http_server_s.c http_server_s.h http_common.o
	$(CC) -Wall  http_common.o http_server_s.c -o http_server_s

clean:
	rm http_server_t http_server_f http_server_s http_common *~ *.o
