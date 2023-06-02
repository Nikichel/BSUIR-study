#makefile
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -Wpointer-to-int-cast -Wunused-variable
.PHONY : clean

all : file_prog generator thread
file_prog: file_prog.c
	$(CC) $(CFLAGS) file_prog.c -o file_prog
	
generator: file_generator.c
	$(CC) $(CFLAGS) file_generator.c -o generator
	
thread: thread.c
	$(CC) $(CFLAGS) thread.c -o thread

clean:
	rm -f file_prog generator thread file.txt
	
