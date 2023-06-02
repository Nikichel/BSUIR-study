#makefile
CC = gcc
CFLAGS = -g -pedantic -std=c11 -W -Wall -Wextra
.PHONY : clean
all : main.o
main.o: ./lab01.c 
	$(CC) $(CFLAGS) -o ./lab1  ./lab01.c

clean:
	rm -rf ./*.o ./lab1
