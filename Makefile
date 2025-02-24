dungeon: dungeon_generation.o dungeon_main.o
	gcc dungeon_generation.o dungeon_main.o -o dungeon

dungeon_main.o: dungeon_main.c
	gcc -Wall -Werror dungeon_main.c -g -c

dungeon_generation.o: dungeon_generation.c dungeon_generation.h
	gcc -Wall -Werror dungeon_generation.c -g -c

clean:
	rm -f *.o dungeon dungeon.exe *~