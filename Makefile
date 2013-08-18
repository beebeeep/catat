PROG=catat
CC=gcc
LD=gcc -o
CFLAGS=-Wall -g 

all: $(PROG).c 
	$(CC) $(CFLAGS) $(FUSEFLAGS) -c $(PROG).c
	$(LD) $(CFLAGS) $(FUSEFLAGS) -o $(PROG) $(PROG).o 

clean: 
	rm -f *.o $(PROG)

