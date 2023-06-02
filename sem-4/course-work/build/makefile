CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic
.PHONY : clean
all: main

main: ../src/util.c makefile
	$(CC) $(CFLAGS) `pkg-config --cflags gtk+-3.0` -o main ../src/util.c `pkg-config --libs gtk+-3.0`

clean:
	rm -f main
