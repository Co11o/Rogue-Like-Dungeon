dungeon: dungeon_generation.o dungeon_main.o dungeon_save_and_load.o priority_queue.o dijkstra.o
	gcc dungeon_generation.o dungeon_main.o dungeon_save_and_load.o priority_queue.o dijkstra.o -o dungeon

dungeon_main.o: dungeon_main.c
	gcc -Wall -Werror dungeon_main.c -g -c

dungeon_generation.o: dungeon_generation.c dungeon_generation.h
	gcc -Wall -Werror dungeon_generation.c -g -c

dungeon_save_and_load.o: dungeon_save_and_load.c dungeon_save_and_load.h
	gcc -Wall -Werror dungeon_save_and_load.c -g -c

priority_queue.o: priority_queue.c priority_queue.h
	gcc -Wall -Werror priority_queue.c -g -c

dijkstra.o: dijkstra.c dijkstra.h 
	gcc -Wall -Werror dijkstra.c -g -c

clean:
	rm -f *.o dungeon dungeon.exe *~