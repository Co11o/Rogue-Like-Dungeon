#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dungeon_generation.h"
#include "dungeon_save_and_load.h"
#include "dijkstra.h"
#include "monster.h"
#include <string.h>
#include <limits.h>
#include <unistd.h>

#define WIDTH 80
#define HEIGHT 21

// Main function
int main(int argc, char *argv[])
{
    srand(time(NULL));
    Dungeon *dungeon = malloc(sizeof(Dungeon));
    memset(dungeon, 0, sizeof(Dungeon));
    Character_Map *character_map = malloc(sizeof(Character_Map));
    memset(character_map, 0, sizeof(Character_Map));
    int save = 0, load = 0, num_monster = 0;
    char *home;
    char *dungeon_file;
    int dungeon_file_length;

    home = getenv("HOME");
    dungeon_file_length = strlen(home) + strlen("/.rlg327/dungeon") + 1; // +1 for the null byte
    dungeon_file = malloc(dungeon_file_length * sizeof(*dungeon_file));
    strcpy(dungeon_file, home);
    strcat(dungeon_file, "/.rlg327/dungeon");

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--save") == 0)
        {
            save = 1;
        }
        else if (strcmp(argv[i], "--load") == 0)
        {
            load = 1;
        }
        else if (strcmp(argv[i], "--nummon") == 0 && i + 1 < argc)
        {
            num_monster = atoi(argv[i + 1]);
            if (num_monster < 1 || num_monster > 20)
            {
                printf("Invalid number of monsters. Please enter a number between 1 and 20.\n");
                return -1;
            }
        }
    }
    // printf("Number of monsters: %d\n", num_monster);

    init_Dungeon(dungeon);
    if (load)
    { //   ./dungeon --load
        if (!load_dungeon(dungeon_file, dungeon))
        {
            // printf("Dungeon loaded from file.\n");
            print_Dungeon(dungeon);
        }
        else
        {
            printf("Failed to load please check dungeon file meets the format below\n/.rlg327/dungeon\n");
            return -1;
        }
    }
    else if (!save)
    { //    ./dungeon
        init_Rooms(dungeon);
        // printf("Dungeon Generated\n");
        print_Dungeon(dungeon);
    }
    else
    { //    ./dungeon --save
        init_Rooms(dungeon);
    }

    if (save)
    { //   --save
        if (!save_dungeon(dungeon_file, dungeon) && !load)
        {
            // print_Dungeon(&dungeon);
            printf("Dungeon saved to file.\n");
        }
        else if (!save_dungeon(dungeon_file, dungeon))
        { // print nothing
        }
        else
        {
            printf("Dungeon failed to save\n");
        }
    }

    int distMapTunnel[HEIGHT][WIDTH];    // Distance map for tunneling monsters
    int distMapNonTunnel[HEIGHT][WIDTH]; // Distance map for non-tunneling monsters

    if (Dijkstra_NonTunneling(dungeon, distMapNonTunnel))
    {
        printf("Failed Non-Tunneling Dijkstra\n");
        return -1;
    }

    if (Dijkstras_Tunneling(dungeon, distMapTunnel))
    {
        printf("Failed Tunneling Dijkstra\n");
        return -1;
    }

    printf("Character Map\n");
    init_character_map(dungeon, character_map, num_monster);
    // Print Character Map
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            if (character_map->grid[y][x] == ' ')
            {
                printf("%c ", dungeon->grid[y][x].type);
            }
            else
            {
                printf("%c ", character_map->grid[y][x]);
            }
        }
        printf("\n");
    }

    int i = 0;
    int Victory = 0;
    /*
    #define ATTRIBUTE_INTELLIGENCE 0x00000001
    #define ATTRIBUTE_TELEPATHIC 0x00000002
    #define ATTRIBUTE_TUNNELING 0x00000004
    #define ATTRIBUTE_ERATIC 0x00000008
    */
    PriorityQueue *movementQueue = malloc(sizeof(PriorityQueue));
    memset(movementQueue, 0, sizeof(PriorityQueue));
    init_movement_queue(movementQueue, character_map);

    // printf("Character is: %c\n", character_map.characters[1].character);
    // for (int i = 0; i < 21; i++)
    // {
    //     if (character_map.characters[i].location_x != 0)
    //     {
    //         printf("Character %c | ERATIC %d | TUNNEL %d | TELEPATHIC %d | INTELLIGENT %d\n", character_map.characters[i].character, (character_map.characters[i].characteristics & 0x00000008),
    //                (character_map.characters[i].characteristics & 0x00000004), (character_map.characters[i].characteristics & 0x00000002), (character_map.characters[i].characteristics & 0x00000001));
    //     }
    // }

    i = 0;
    // int j = 0;
    // int k, found = 0;
    while (!Victory)
    {
        // printf("turn #%d\n", i + 1);
        //  printCharacterMap(&character_map);
        //  printQueue(&movementQueue);
        //   printf("Queue:\n");
        //   j=0;
        //   while(character_map.characters[j].alive == 1){
        //       queue_node character_node;
        //       k=0;
        //       found = 0;
        //       while(k < movementQueue.size){
        //           if(movementQueue.queue[k].x_location == character_map.characters[j].location_x && movementQueue.queue[k].y_location == character_map.characters[j].location_y){
        //               //printf("Found in queue \t");
        //               //found = 1;
        //               character_node = movementQueue.queue[k];
        //           }
        //           k++;
        //       }
        //       //printf("Character %c : Speed %d | X %d | Y %d | Queue: priority %d | X %d | Y %d | Found %d\n",character_map.characters[j].character, character_map.characters[j].speed, character_map.characters[j].location_x, character_map.characters[j].location_y, character_node.priority, character_node.x_location, character_node.y_location, found );
        //       j++;
        //   }
        //   Process the next movement event from the queue
        process_next_event(movementQueue, dungeon, character_map, distMapTunnel, distMapNonTunnel);

        // printf("%c MOVED TO: ( %d, %d )\n", character_map.characters[1].location_x, character_map.characters[1].location_y);
        // printf("-------------------------------\n\n");

        // Print the updated dungeon and character map

        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                if (character_map->grid[y][x] == ' ')
                {
                    printf("%c ", dungeon->grid[y][x].type);
                }
                else
                {
                    printf("%c ", character_map->grid[y][x]);
                }
            }
            printf("\n");
        }
        if (character_map->characters[0].alive == 0)
        {
            printf("@ has been SLAIN GAME OVER\n");
            Victory = 1;
        }
        else if (character_map->grid[character_map->characters[0].location_y][character_map->characters[0].location_x] != '@')
        {
            printf("@ has been SLAIN GAME OVER\n");
            Victory = 1;
        }
        usleep(250000);
        i++;
        if (i % (num_monster + 1) == 0)
        {
            if (Dijkstra_NonTunneling(dungeon, distMapNonTunnel))
            {
                printf("Failed Non-Tunneling Dijkstra\n");
                return -1;
            }
            if (Dijkstras_Tunneling(dungeon, distMapTunnel))
            {
                printf("Failed Tunneling Dijkstra\n");
                return -1;
            }
        }
        if (i > (25 * num_monster))
        {
            printf("@ has survived %d turns and claims Victory\n", i);
            Victory = 1;
        }
    }
    // printCharacterMap(&character_map);
    // printQueue(&movementQueue,&character_map);
    //  printf("\nCharacter %c | ERATIC %d | TUNNEL %d | TELEPATHIC %d | INTELLIGENT %d\n",character_map.characters[1].character,(character_map.characters[1].characteristics & 0x00000008),
    //      (character_map.characters[1].characteristics & 0x00000004),(character_map.characters[1].characteristics & 0x00000002), (character_map.characters[1].characteristics & 0x00000001));
    //  printf("Character %c: ERATIC %d: TUNNEL %d\n",character_map.characters[1].character,(character_map.characters[1].characteristics & 0x00000008), (character_map.characters[1].characteristics & 0x00000004));

    free(dungeon_file);
    free(dungeon);
    free(character_map);
    free(movementQueue);

    return 0;
}
