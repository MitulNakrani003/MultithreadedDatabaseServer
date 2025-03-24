#
# file:        Makefile - project 2
# description: compile, link with pthread and zlib (crc32) libraries
#

LDLIBS=-lz -lpthread
CFLAGS=-ggdb3 -Wall -Wno-format-overflow

EXES = dbserver dbtest

all: $(EXES)

dbtest: dbtest.o

dbserver: dbserver.o dbservices.o queue.o dboperations.o
	gcc -ggdb3 dbserver.o dbservices.o queue.o dboperations.o -o dbserver

CFLAGS = -Wall -ggdb3

dbtest.o: dbtest.c
	gcc $(CLAGS) -c dbtest.c -o dbtest.o

dbserver.o: dbserver.c dbservices.h
	gcc $(CFLAGS) -c dbserver.c -o dbserver.o

dbservices.o: dbservices.c dbservices.h queue.h
	$(CC) $(CFLAGS) -c dbservices.c -o dbservices.o

queue.o: queue.c queue.h
	gcc $(CFLAGS) -c queue.c -o queue.o

dboperations.o: dboperations.c dboperations.h
	gcc $(CFLAGS) -c dboperations.c -o dboperations.o

clean:
	rm -f $(EXES) *.o data.[0-9]*
