CC = gcc
CFLAGS = -Wall -g
PROG = diskTest
OBJS = diskTest.o libDisk.o

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS)

diskTest.o: diskTest.c libDisk.h
	$(CC) $(CFLAGS) -c diskTest.c

libDisk.o: libDisk.c libDisk.h
	$(CC) $(CFLAGS) -c libDisk.c

clean:
	rm -f $(PROG) $(OBJS)
