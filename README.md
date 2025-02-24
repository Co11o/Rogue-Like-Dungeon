Jackson Collalti

Getting file path
In main the program gets the users home Environment and adds /.rlg327/dungeon to the home search path.
After we have the home path and dungeon path malloc a string of the full path then feed the full path
into the save and load functions. At end of main free the memory used for the full path

Load
The load function takes in a file path and a pointer to a blank dungeon. Using the file path, load attempts
to read the file and update the dungeon to what is in the file. If the file doesn't exist the function 
returns an error that prints a about no file. Also if the file is in an incorrect format another error will be
printed. Assuming the file is in the correct format then after getting version and file size. The PC location is
updated in the dungeon struct. Next step is reading hardness if hardness is 0 then set the Cell type to '#' otherwise 
Cell type is ' '. At this point rooms are read using the x and y coordinates along with width and height we 
replace the '#'s with '.'s for each room. Then stairs are read stairs up first then stairs down are added at the
given x and y location. Finally we add the PC to the location gotten earlier to ensure it was overwritten by a '#'
'.' or staircase. At this point the dungeon now contains all the load data from file.

Save
The save function takes in a file path and a pointer to dungeon. The dungeon will either be loaded or newly generated.
First check is if the file path is valid if not an error is printed that no valid file to save to. To begin writing 
the file marker and version are written to the file (all data is written in big endian). Using the dungeon struct at 
the pointer the file size is calculated using the number of rooms, number of stairs up and number of stairs down along
with 1708 bytes of necessary data. After we have the file size it gets written. The order dungeon data is written is PC 
location x then y from the dungeon struct. Then writting the hardness map followed by dungeon room count and the room 
locations x,y,width, and height. Similiar thing is done with stairs doing stairs up first getting the total number of
stairs up getting their location. Same thing is done for stairs down. At this point all data has been written so the file
is closed.

Main
The first part of main is getting the file path then reading if a save or load was called. If neither save or load was
called just generate and print the new dungeon. If only save is called generate a new dungeon and attempt to write the 
data to the save file and print a message that the dungeon has been saved (Won't print the saved dungeon). If only load 
is called, attempt to read the file and print the dungeon from the file. If both save and load are called (order doesn't
matter in call "./dungeon --save --load" is same as "./dungeon --load --save") the load will always run before the save 
so a loaded dungeon will be saved. The loaded dungeon will be displayed and saved after being displayed. 

How to run:
    *Note to load or save a file
    file must be named "dungeon" with no dot extension. Also must be in /.rlg327 folder in the users home directory.
    If either isn't followed an error message will be printed (.ie /.rlg327/dungeon.rlg327 will not be read and will
    print and error because of the .rlg327 added to the dungeon)
1) run "make" 
2a) run "./dungeon --save" to save a newly generated to /.rlg327/dungeon
2b) run "./dungeon --load" to load dungeon saved in /.rlg327/dungeon
2c) Can also run "./dungeon --save --load" or "./dungeon --load --save" to save a loaded dungeon

