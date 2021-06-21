CFLAGS=-Wall -Wextra -Wswitch-enum -std=c99 -pedantic -ggdb

.PHONY: all
all: examples test

test: test.c jim.h
	$(CC) $(CFLAGS) -o test test.c 

.PHONY: examples
examples: 
	$(MAKE) -C examples/
