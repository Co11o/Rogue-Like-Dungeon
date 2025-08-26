Same file found in Rouge_Like_Dungeon Folder

This game is meant to run on *Linux* running it on other OS's may cause issues with libraries 

Made by Jackson Collalti

How to run:
1. Compile
    run "make" to compile the project

2. Run
    (a) run "./dungeon" with default settings

    (b) run "./dungeon --save" to save a newly generated to /.rlg327/dungeon

    (c) run "./dungeon --load" to load dungeon saved in /.rlg327/dungeon
        *Note to save a file
        A "/.rlg327" folder must be in the users home directory. Otherwise the save will fail
        *see sample "/.rlg327" folder provided

        *Note to load  a file
        The file must be named "dungeon" with no dot extension. "dungeon" must also be in /.rlg327 folder in the users home directory.
        If either isn't followed an error message will be printed (.ie /.rlg327/dungeon.rlg327 will not be read and will
        print an error because of the .rlg327 added to the dungeon file)

    (d) run "./dungeon --save --load" or "./dungeon --load --save" to save a loaded dungeon

3. Customize
    (a)	For adding/lowering the number of monsters add the switch "--nummon %d" to "./dungeon" and any additional switches where %d is some integer between 1 and 20. If this switch isn't added
        the default of 10 monsters will be added to the dungeon

    (b) To run with custom monsters/objects be sure "monster_desc.txt" is in "./rlg327" for monster descriptions and "object_desc.txt" is in "./rlg327" for object descriptions
        *see sample "/.rlg327" folder provided

--------------------------------------GAME DETAILS----------------------------------------------------

GAMEPLAY
--------
    PC Movement:
        Grid movement (5 is where the PC is currently at)

                                7 8 9
                                4 5 6
                                1 2 3

        7 or y Attempt to move PC one cell to the upper left.
        8 or k Attempt to move PC one cell up.
        9 or u Attempt to move PC one cell to the upper right.
        6 or l Attempt to move PC one cell to the right.
        3 or n Attempt to move PC one cell to the lower right.
        2 or j Attempt to move PC one cell down.
        1 or b Attempt to move PC one cell to the lower left.
        4 or h Attempt to move PC one cell to the left.
        5 or space or . Rest for a turn. NPCs still move.
        > Attempt to go down stairs. Works only if standing on down staircase.
        < Attempt to go up stairs. Works only if standing on up staircase

        Stairs:
            Taking a staircase generates a new dungeon with the same number on monster as the original dungeon. Levels of the dungeon do not presist meaning once you leave a level you won't be able to return to it

    PC Non Movement Turns:
        Show Monster List (user input 'm' when PC's turn):  
            Displayed on over the center of the dungeon. Displays the location of all monster relative to the current PC position ("3, 12 North and 2 East"). If the monster is dead the monster list will have text saying the monster is dead in 
            red text. The list implements scrolling such that a max of 10 monsters are shown at a time with scrolling giving access to the rest of the monsters locations. Scrolling is done with arrow keys where up arrow scrolls up and down 
            arrow scrolls down. If the dungeon was generated with less than 10 monsters, all monster will be shown on one page. ESC exits the list.

        Show Monster Info List (user input 'C' when PC's turn):
            Displayed on over the center of the dungeon. Displays all character's characteristics in a true false table showing all attributes of a given monster (Intelligent, Telepathic, Tunneling, Eratic) the table also shows each characters 
            speed. Scrolling is implement similiar to the monster list. The PC characteristics are printed at the top of the Character info list. ESC exits the list.

        Show Tunneling Distance Maps (user input 'T' when PC's turn):
            Displayed on lines 1-21 instead of the dungeon. Prints the last digit of min distance from all positions to the PC. ESC exits the map.

        Show Non-Tunneling Distance Maps (user input 'D' when PC's turn):
            Displayed on lines 1-21 instead of the dungeon. Prints the last digit of min distance from all positions to the PC. ESC exits the map.

        Show Hardness Map (user input 'H' when PC's turn): 
            Displayed on lines 1-21 instead of the dungeon. Prints the last digit of hardness map. ESC exits the map.
        
        Show Dungeon Without Fog (user input 'f' when PC's turn): 
            Displays the full map of the dungeon without fog enabled

        Show Turn Queue (user input '|' when PC's turn):
            Displays the current state of the movement queue. Used mostly for debugging but can be used to view upcoming turn order

        Show Telepoprt (user input 'g' when PC's turn):
            Displays the full dungeon without fog with a * character representing the current teleportation desination. Using same keys as PC movement the destination can be changed and is displayed in this mode. When at the desired location 
            press 'g' again or press 'r' to randomly teleport to a location.

    PC Turns
        -If the PC moves to a valid spot PC's turn is over.
        -If PC moves to invalid location the PC rest/doesn't move and PC's turn is over
        -If a display option such as viewing any of distance map, hardness map, character info list, or monster list counts a turn and when you exit the the map or list the PC's turn is over.
        -If on a stair and the correct stair input is inputed a new dungeon is created and turn it remains PC's turn
        -If not on a stair matching the input of stair the PC's rest and the turn is over

                                        ***  WARNING  ***
        If more than one key is pressed during a turn, one or more key is pressed while not PC's turn, or a key is pressed and held. The input will still be read such that the input will potentially go towards the next turn. 
        This may cause a casading problem. If a key pressed doesn't match the action wait until the next stop to clear out the buffer. Continuing to press keys will expand the buffer worsening the issue.
        
    PC Fog:
        PC has a vision radius of 3 meaning a 5x5 box around the PC current position. Each time the PC is moved discovered array keeping track of what's been seen in the dungeon is updated. Monsters are only displayed when within vision. 
        Anytime terrain changes in a already visited area that the PC isn't currently in discovered isn't updated. Walls block vision in the following way

        Diagonal walls: wall represent with an X below
                        A B C
                        . X D
                        @ . E 
        X blocks @ vision to only C meaning C will remain undiscovered

        Horizontal/Vertical walls: wall represent with an X below
                        A B C
                        . X .
                        . @ .
        X blocks only B meaning B will remain undiscovered

        Combination
                        A B C D
                        . X X .
                        . @ . .
        X's Block both B C and D
    
    Inventory/Equipment Management:
        Automatic Pickup:
            When equipment slot matching the item on the floor or any carry slots are open. The PC will automaticly pickup and equip or store in carry slot depending on if equipment slot is filled
            All the below work assuming it's currently PC's turn
        
        Drop Item ('d'):
            Must be in a carry slot. Once a valid slot is chosen the item is dropped onto a neighboring floor. Since automatic pickup is on items aren't dropped on the same floor location that the PC is currently on.
        
        Wear ('w'):
            Equips a carried item and if a the equipment slot was filled then the items are swapped.
        
        Take off ('t'):
            Moves an equiped item to a carried item. If no more carry slots takeoff will fail and the PC will have to drop a carried item before attempting to take off an item again.
        
        Expunge ('x'):
            The selected carry item is removed from inventory and item_map. Then in the storage of parsed items a flag is changed so that the item won't be able to be generated again simialar to how artifact/uniqueness are handled.
        
        Inspecting items ('I') /monsters ('L'):
            Monster selection use teleportation like movement where movement keys are used to select visiable monsters. Item selection is done by selecting any equipment slot or carry slot. Then the description will be printed over the dungeon 
            and press esc to exit.
        
        List equipment ('e') / carry slot ('i'):
            Displays the slot name, name of item, and type of the item.
            
    End Of Game:
        Victory (PC kills a boss monster): 
            Displays yellow message saying victory
        
        Loss (PC is slain):
            Displays red message saying game over

        Quit (user input 'Q' when PC's turn):
            Displays blue message saying the game has been quit before program termination

DISPLAY
-------
    The display is made out of a 80 x 21 grid of Cells. Borders of the display are marked with '-' and '|' signaling edges of display and game field

    Display Hierarchy (Overlaps):
        Priority 1: Characters (PC/Monsters)
        Priority 2: Objects
        Priority 3: Dungeon Terrain

    Characters:
        PC (Player Character): '@'

        Monsters Default: '0'-'9' and 'a'-'f'
            if using custon monsters check monster descriptions for symb characteristic

    Objects: 
        Items on the floor such as armor and weapons 

    Terrain: 
        Each Cell has a type and a visual when nothing is on the location
            Room floor: '.'
            Hallways: '#' 
            Rock: ' '
            Stair Up: '<'
            Stair Down: '<'
            Display Border: '|' or '-'

    
GENERATION
----------
    Dice Rolls: 
        <base> + <rolls>d<sides>
    
    Dungeon (DEFAULT): 
        Rooms:
            All Rooms are made up of '.' to signify walkable floors
            Number of rooms: 7-10
            Size: Horizontal 5 - 12 Cells by Vertical 6 - 9 Cells
            Each generation is guarenteed at least 1 stair up and 1 stair down in random rooms
        
        Hallways:
            Hallways ('#') connect rooms to each other. All rooms are reachable rooms may have more than one hallway.  
        
        Rock:
            Rocks are non-walkable terrain to the PC is displayed by ' '. 
                Unmutable Rocks/Borders ('-' and '|') are set to hardness 255 and can't be broken.
                
                Mutable rocks are rocks that have a hardness 1-254 that can be broken by tunneling monsters
                    when broken the rock turns to a walkable floor
        
        Monsters: 
            Monsters (Default):
                Works same as previous system's monsters (0-F) all monster have color of white and sybl is their character (0-F). For "rolled" attributes / moves the rolls are
                large dice rolls for extreme ranges in values for something like HP. The game is ideal played using custom monsters

                Monster have four primary/rolled characteristics (Intelligent, Telepathic, Tunneling, Erratic), All characteristics have 50% chance of being applied to a monster.
                    ERATIC:
                        There's a 50% chance that the character goes to to a random valid neighboring location
                    
                    INTELLITGENT:
                        Monster will take the shortest path to the PC if known
                    
                    TELEPATHIC:
                        Monster always knows the PC's current location
                    
                    TUNNELING:
                        Can dig through rocks

            Monsters (Custom):
                Parsed monsters are displayed using their sybl and color. If a monster has more than one color, currently only the first color is used. For some of the new
                attributes like invisiablity and boss are present in the monster objects but not implemented in game yet. If a monster is unique once placed in the dungeon 
                the parsed monster object will no longer become valid to place meaning once gerenate once it won't be placed again. When taking a staircase monsters will be 
                freshly generated with the same rules as the initial dungeon same number of monsters and no duplicates of unique monsters.

    Items:
        Items (Default):
            If no object_desc.txt is provided no objects will be generated. The game will still run, but without any objects.

        Items (Custom):
            Items are displayed using their type and color. At initialization all stats are rolled. Once "discovered" by the PC the item will remain shown/remebered. Items 
            currently don't stack for example only the "last generated item" will show if two items share a location (very unlikely in generation). If a item is an artifact
            once placed in the dungeon the parsed object will no longer become valid to place meaning once gerenate once it won't be placed again. When taking a staircase items
            will be freshly generated with the no duplicates of unique monsters. Each time a a dungeon is generated at least 10 items are genereated and a continuous 75% chance 
            for an additional item up to 20 items.
