CC= gcc
CFLAGS= -o

server: server.c 
	$(CC) server.c $(CFLAGS) server
	./server

	rm -rf server


