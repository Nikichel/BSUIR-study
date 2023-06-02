#makefile
CC = gcc
CFLAGS = -W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic	-Wbuiltin-declaration-mismatch
.PHONY : clean
all : parent consumer producer
parent: parent.c makefile
	$(CC) $(CFLAGS) parent.c -o parent
	
consumer: consumer.c makefile
	$(CC) $(CFLAGS) consumer.c -o consumer

producer: producer.c makefile
	$(CC) $(CFLAGS) producer.c -o producer

clean:
	rm -f parent consumer producer
