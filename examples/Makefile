CFLAGS=-Wall -Wextra -Wswitch-enum -ggdb

.PHONY: all
all: 01_from_readme 02_binary_tree 03_parsing_database

01_from_readme: 01_from_readme.c ../jim.h
	$(CC) $(CFLAGS) -o 01_from_readme 01_from_readme.c

02_binary_tree: 02_binary_tree.c fruits.h ../jim.h
	$(CC) $(CFLAGS) -o 02_binary_tree 02_binary_tree.c

03_parsing_database: 03_parsing_database.c ../jimp.h ../thirdparty/nob.h
	$(CC) $(CFLAGS) -o 03_parsing_database 03_parsing_database.c

04_from_readme_round: 04_from_readme_round.c ../jimp.h ../thirdparty/nob.h
	$(CC) $(CFLAGS) -o 04_from_readme_round 04_from_readme_round.c
