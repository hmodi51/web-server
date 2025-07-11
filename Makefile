CC= gcc
CFLAGS= -g -o

server: server.c 
	$(CC) server.c $(CFLAGS) server
	
gdb:server
	gdb server

clean:
	rm server



