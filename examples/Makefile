CFLAGS=-Wall -Wextra -Wswitch-enum -std=c99 -pedantic -ggdb

.PHONY: all
all: 01_from_readme 02_binary_tree

01_from_readme: 01_from_readme.c ../jim.h
	$(CC) $(CFLAGS) -o 01_from_readme 01_from_readme.c

02_binary_tree: 02_binary_tree.c fruits.h ../jim.h
	$(CC) $(CFLAGS) -o 02_binary_tree 02_binary_tree.c
