#makefile
CC = gcc
CFLAGS = -g -pedantic -std=c11 -W -Wall -Wextra -pthread -Wreturn-type
.PHONY : clean

all : lab5
lab5: lab05.c 
	$(CC) $(CFLAGS) lab05.c -o lab5

clean:
	rm -f lab5
