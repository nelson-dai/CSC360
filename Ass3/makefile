#
# "makefile" for the CPU scheduler simulation.
#

CC=gcc
CFLAGS=-c -Wall -g -DDEBUG
CFLAGS_STATS=-c -Wall -g -DSTATS

all: rrsim simgen testlist

simgen: simgen.o
	$(CC) simgen.o -o simgen

simgen.o: simgen.c
	$(CC) $(CFLAGS) simgen.c

rrsim: rrsim.o linkedlist.o
	$(CC) rrsim.o linkedlist.o -o rrsim

rrsim.o: rrsim.c linkedlist.h
	$(CC) $(CFLAGS) rrsim.c

testlist: testlist.o linkedlist.o
	$(CC) testlist.o linkedlist.o -o testlist

testlist.o: testlist.c linkedlist.h
	$(CC) $(CFLAGS) testlist.c

linkedlist.o: linkedlist.c linkedlist.h
	$(CC) $(CFLAGS) linkedlist.c

clean:
	rm -rf *.o rrsim simgen testlist

rrsim-stats: stats-obj linkedlist.o
	$(CC) rrsim.o linkedlist.o -o rrsim
	
stats-obj: rrsim.c linkedlist.h
	$(CC) $(CFLAGS_STATS) rrsim.c
	
	
	