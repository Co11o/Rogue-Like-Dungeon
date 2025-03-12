Jackson Collalti

Characters (PC + Monsters):
Monster structs have characteristics (Intelligent, Telepathic, Tunneling, Erratic), location x and y on the grid, char that represents them on the map. Last seen PC x and y values to keep track for intelligent monsters. Speed and Alive for the speed of monster and if the monster is alive. All characteristics have 50% chance of being applied to a monster.

Character Map:
Character Map structs holds a char array of screen width and height. All spots that aren't held by PC or monsters is   ' '. This is used to print over the terain map when a grid location in character map array isn't blank. Character Maps also hold an array of character of MAX MONSTERS (20) + 1 (for PC).

Character Movement:
First Check if the character has a view of the PC. View of PC is defined as if the character is in the same room as PC or (X or Y) location is visible 10 spaces in each direction (no walls between character and PC). If eratic there's a 50% chance that the character goes to to a random valid neighboring location. If the character is Intelligent it will use dijksra's to take the shortest path to the pc if known (either through telepathy or line of sight). Non-Inteligent monsters will take staight line paths if its on the same x or y it as the pc the character will move in that direction until it reaches PC or if not able to tunnel it gets stuck on a wall. If on different X's and Y's than PC the monster will travel diagonally until it reaches the same x and y as PC. If the monster isn't telepathic and hasn't seen the PC meaning it doesn't know where to move the monster stays still and doesn't move this applies until the monster see the PC. Tunneling monsters can use dijkstra's through rock for intelligent and telapathic monsters. Eratic tunneling monsters can eraticly start tunneling at the random neighboring location. PC never moves.

Movement Queue:
The priority queue operates by storing nodes representing each character contain an x and y position and priority (the next turn of the node). WHen starting the game the all nodes have priority of 0, the PC is alway first in the initial queue so PC will always have the first turn. When a node gets exracted the character location represented by the node is moved based on its properties. Once the move is over the node is added back with the new location and updated priority of the next turn. When a monster dies a flag gets updated so that the next time that node gets extracted the node doesn't get readded to the queue. Since PC never move the node will always get added back with the same location until PC movement is supported.

Main:
Now a switch --nummon is used to indicate how many monster are added to the game. The range is 1-20. More than 20 will cause an error of too many monsters. If no switch is used the defualt is 10 monsters. The "playing loop" will run until the PC dies or the number of total turns reaches 25 * the number of monsters this is temperary since PC doesn't move this avoid a senario of an infinite loop. Depending on the loop termination a message will print if the PC died or survirved. Dijkrstras tunnel and non-tunnel are updated after every character has had a chance to move. The loop will continue to process events from the queue until the end of game and a map is printed each iteration.

How to run:
    *Note to load or save a file
    file must be named "dungeon" with no dot extension. Also must be in /.rlg327 folder in the users home directory.
    If either isn't followed an error message will be printed (.ie /.rlg327/dungeon.rlg327 will not be read and will
    print and error because of the .rlg327 added to the dungeon file)
1) run "make" 
2a) run "./dungeon --save" to save a newly generated to /.rlg327/dungeon
2b) run "./dungeon --load" to load dungeon saved in /.rlg327/dungeon
2c) Can also run "./dungeon --save --load" or "./dungeon --load --save" to save a loaded dungeon
3) For monsters use the switch "--nummon %d" where %d is some integer between 1 and 20. If this switch isn't added 
    the default of 10 monsters will be added to the dungeon 

