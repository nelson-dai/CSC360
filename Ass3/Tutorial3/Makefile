CFLAGS=-g -D_REENTRANT 
LIBS=-lpthread -lrt 
CC=gcc

all: fixed broken procon

fixed : fixcnt.c
	$(CC) $< -o $@  $(CFLAGS) $(LIBS) 

broken : badcnt.c 
	$(CC) $< -o $@  $(CFLAGS) $(LIBS)

procon: producerconsumer.c
	$(CC) $< -o $@  $(CFLAGS) $(LIBS)


clean:
	-rm fixed broken procon	
