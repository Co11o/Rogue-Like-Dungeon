#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dungeon_generation.h"
#include "dungeon_save_and_load.h"
#include <string.h>

// Main function
int main(int argc, char *argv[]) {
    srand(time(NULL));
    Dungeon dungeon;
    int save = 0, load = 0;
    char *home;
    char *dungeon_file;
    int dungeon_file_length;

    home = getenv("HOME");
    dungeon_file_length = strlen(home) + strlen("/.rlg327/dungeon") + 1; // +1 for the null byte
    dungeon_file = malloc(dungeon_file_length * sizeof (*dungeon_file));
    strcpy(dungeon_file, home);
    strcat(dungeon_file, "/.rlg327/dungeon");

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--save") == 0) {
            save = 1;
        } else if (strcmp(argv[i], "--load") == 0) {
            load = 1;
        }
    }
    init_Dungeon(&dungeon);
    if (load) {//   ./dungeon --load
        if(!load_dungeon(dungeon_file,&dungeon)){
            //printf("Dungeon loaded from file.\n");
            print_Dungeon(&dungeon);
        }else{
            printf("Failed to load please check dungeon file meets the format below\n/.rlg327/dungeon\n");
            return -1;
        }
    } else if(!save){ //    ./dungeon
        init_Rooms(&dungeon);
        //printf("Dungeon Generated\n");
        print_Dungeon(&dungeon);
    }else{//    ./dungeon --save
        init_Rooms(&dungeon);
    }
    
    
    if (save) {//   --save
        if(!save_dungeon(dungeon_file,&dungeon) && !load){
            //print_Dungeon(&dungeon);
            printf("Dungeon saved to file.\n");
        }else if(!save_dungeon(dungeon_file,&dungeon)){ //print nothing
        }else{
            printf("Dungeon failed to save\n");   
        }
    }
    //Output Dungeon
    //printf("Final Dungeon\n\n");
    //print_Dungeon(&dungeon);
    free(dungeon_file);

    return 0;
}