CFLAGS=-Wall -Wextra -Wswitch-enum -ggdb

.PHONY: all
all: 01_from_readme 02_binary_tree 03_parsing_database 04_parsing_defaults

01_from_readme: 01_from_readme.c ../jim.h
	$(CC) $(CFLAGS) -o 01_from_readme 01_from_readme.c

02_binary_tree: 02_binary_tree.c fruits.h ../jim.h
	$(CC) $(CFLAGS) -o 02_binary_tree 02_binary_tree.c

03_parsing_database: 03_parsing_database.c ../jimp.h ../thirdparty/nob.h
	$(CC) $(CFLAGS) -o 03_parsing_database 03_parsing_database.c

04_parsing_defaults: 04_parsing_defaults.c ../jimp.h ../thirdparty/nob.h
	$(CC) $(CFLAGS) -o 04_parsing_defaults 04_parsing_defaults.c
