CFLAGS=-Wall -Wextra -Wswitch-enum -ggdb

.PHONY: all
all: examples test

test: test.c jim.h
	$(CC) $(CFLAGS) -o test test.c 

.PHONY: examples
examples: 
	$(MAKE) -C examples/
