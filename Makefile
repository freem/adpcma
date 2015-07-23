# quick and dirty makefile that needs to be better prepared for cross-platform stuff
CC = gcc
CFLAGS += -O3 -Wall -std=c99

.PHONY: all clean

all: adpcma

adpcma: adpcma.o

clean:
	rm -f *.o *.obj
