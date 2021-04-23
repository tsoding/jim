CFLAGS=-Wall -Wextra -Wswitch-enum -std=c99 -pedantic -ggdb

.PHONY: all
all: example test

example: example.c jim.h
	$(CC) $(CFLAGS) -o example example.c 

test: test.c jim.h
	$(CC) $(CFLAGS) -o test test.c 
