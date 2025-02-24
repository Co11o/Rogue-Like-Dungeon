#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dungeon_generation.h"
#include "dungeon_save_and_load.h"
#include "dijkstra.h"
#include <string.h>
#include <limits.h>

#define WIDTH 80
#define HEIGHT 21

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

    if(!save || load){
    int distMapTunnel[HEIGHT][WIDTH];//Distance map for tunneling monsters
    int distMapNonTunnel[HEIGHT][WIDTH];//Distance map for non-tunneling monsters

    if(Dijkstra_NonTunneling(&dungeon, distMapNonTunnel)){
        printf("Failed Non-Tunneling Dijkstra\n");
        return -1;
    }
    printf("\n");
    printf("Non Tunneling Distance Map");
    printf("\n");

    //Print Dikjstras Non-Tunneling
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if(x == dungeon.player_x && y == dungeon.player_y){
                printf("@ ");
            }else if(distMapNonTunnel[y][x] == INT_MAX){
                // Unreachable floor
                if(dungeon.grid[y][x].hardness == 0){
                    printf("X ");
                }else{
                    printf("  ");
                }
            }else{
                printf("%d ", distMapNonTunnel[y][x] % 10);
            }
        }
        printf("\n");
    }


    if(Dijkstras_Tunneling(&dungeon, distMapTunnel)){
        printf("Failed Tunneling Dijkstra\n");
        return -1;
    }

    printf("\n");
    printf("Tunneling Distance Map");
    printf("\n");

    //Print Dikjstras tunneling
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if(x == dungeon.player_x && y == dungeon.player_y){
                printf("@ ");
            }else if(distMapTunnel[y][x] == INT_MAX){
                printf("  ");
            }else{
                printf("%d ", distMapTunnel[y][x] % 10);
            }
        }
        printf("\n");
    }
}


    // printf("\n");
    // printf("Hardness Map");
    // printf("\n");

    // for (int y = 0; y < HEIGHT; y++) {
    //     for (int x = 0; x < WIDTH; x++) {
    //         printf("%02x",dungeon.grid[y][x].hardness);
    //     }
    //     printf("\n");
    // }

    free(dungeon_file);

    return 0;
}