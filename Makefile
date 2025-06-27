CFLAGS=-Wall -Wextra -Wswitch-enum -ggdb

.PHONY: all
all: examples test jimp

test: test.c jim.h
	$(CC) $(CFLAGS) -o test test.c 

jimp: jimp.c nob.h
	$(CC) $(CFLAGS) -o jimp jimp.c

.PHONY: examples
examples: 
	$(MAKE) -C examples/
