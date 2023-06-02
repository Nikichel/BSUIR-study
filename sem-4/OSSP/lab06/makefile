#makefile
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -Wpointer-to-int-cast
.PHONY : clean

all : generator	sort_index read
generator: generator.c
	$(CC) $(CFLAGS) generator.c -o generator
sort_index: sort_index.c
	$(CC) $(CFLAGS) sort_index.c -o sort_index
read: read_file.c
	$(CC) $(CFLAGS) read_file.c -o read

clean:
	rm -f generator	sort_index read
