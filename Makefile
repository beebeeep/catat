EXECUTABLE=catat
SOURCES=catat.c misc.c opcodes.c
OBJECTS=$(SOURCES:.c=.o)

CC=gcc
LD=gcc -o
CFLAGS=-std=gnu99 -Wall -Werror -pedantic -g

all: $(EXECUTABLE) $(OBJECTS) 

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(CFLAGS) -o $@ $(OBJECTS) 

*.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -f $(OBJECTS) $(EXECUTABLE) 
