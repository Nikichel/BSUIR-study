#makefile
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -Wpointer-to-int-cast -Wunused-variable
.PHONY : clean

all : client server

client: client.c
	$(CC) $(CFLAGS) client.c -o client
	
server: server.c
	$(CC) $(CFLAGS) server.c -o server
	
clean:
	rm -f server client
	
