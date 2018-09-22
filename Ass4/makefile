#
# "makefile" for the disk-image file-system assignment
#

CC=gcc
CFLAGS=-c -Wall -g -DDEBUG

all: statuvfs lsuvfs catuvfs storuvfs rmuvfs

ufvsutils.o: uvfsutils.c uvfsutils.h
	$(CC) $(CFLAGS) uvfsutils.c
	
statuvfs: statuvfs.o  uvfsutils.o
	$(CC) statuvfs.o uvfsutils.o -o statuvfs

statuvfs.o: statuvfs.c disk.h
	$(CC) $(CFLAGS) statuvfs.c

lsuvfs: lsuvfs.o uvfsutils.o
	$(CC) lsuvfs.o uvfsutils.o -o lsuvfs

lsuvfs.o: lsuvfs.c disk.h
	$(CC) $(CFLAGS) lsuvfs.c

catuvfs: catuvfs.o uvfsutils.o
	$(CC) catuvfs.o uvfsutils.o -o catuvfs

catuvfs.o: catuvfs.c disk.h
	$(CC) $(CFLAGS) catuvfs.c

storuvfs: storuvfs.o uvfsutils.o
	$(CC) storuvfs.o uvfsutils.o -o storuvfs

storuvfs.o: storuvfs.c disk.h
	$(CC) $(CFLAGS) storuvfs.c
	
rmuvfs: rmuvfs.o uvfsutils.o
	$(CC) rmuvfs.o uvfsutils.o -o rmuvfs
	
rmuvfs.o: rmuvfs.c disk.h
	$(CC) $(CFLAGS) rmuvfs.c

clean:
	rm -rf *.o statuvfs lsuvfs catuvfs storuvfs rmuvfs
