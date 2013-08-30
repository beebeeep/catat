TARGET=catat
LIBS=-lm

CC=gcc
LD=gcc
CFLAGS=-std=gnu99 -Wall -Werror -pedantic -g

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

.PHONY: default all clean

default: $(TARGET)

all: default

%.o: %.c $(HEADERS)
	$(LD) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
