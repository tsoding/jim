CFLAGS=-Wall -Wextra -Wswitch-enum -ggdb -I./thirdparty/

.PHONY: all
all: examples test_jim

test_jim: test_jim.c jim.h
	$(CC) $(CFLAGS) -o test_jim test_jim.c 

.PHONY: examples
examples: 
	$(MAKE) -C examples/
