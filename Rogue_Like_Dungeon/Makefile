dungeon: dungeon_generation.o dungeon_main.o dungeon_save_and_load.o items.o priority_queue.o dijkstra.o characters.o display.o description_parsing.o
	g++ dungeon_generation.o dungeon_main.o dungeon_save_and_load.o items.o priority_queue.o dijkstra.o characters.o display.o description_parsing.o -o dungeon -lncurses

dungeon_main.o: dungeon_main.cpp
	g++ -Wall -Werror dungeon_main.cpp -g -c

dungeon_generation.o: dungeon_generation.cpp dungeon_generation.h 
	g++ -Wall -Werror dungeon_generation.cpp -g -c

dungeon_save_and_load.o: dungeon_save_and_load.cpp dungeon_save_and_load.h
	g++ -Wall -Werror dungeon_save_and_load.cpp -g -c

priority_queue.o: priority_queue.cpp priority_queue.h
	g++ -Wall -Werror priority_queue.cpp -g -c

dijkstra.o: dijkstra.cpp dijkstra.h
	g++ -Wall -Werror dijkstra.cpp -g -c

characters.o: characters.cpp characters.h 
	g++ -Wall -Werror characters.cpp -g -c

description_parsing.o: description_parsing.cpp description_parsing.h 
	g++ -Wall -Werror description_parsing.cpp -g -c

items.o: items.cpp items.h
	g++ -Wall -Werror items.cpp -g -c

display.o: display.cpp display.h
	g++ -Wall -Werror display.cpp -g -c

clean:
	rm -f *.o dungeon dungeon.exe *~