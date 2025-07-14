CC= gcc
CFLAGS= -g -Werror -fsanitize=address -o

server: server.c 
	$(CC) server.c $(CFLAGS) server
	
gdb:server
	gdb server

clean:
	rm server



