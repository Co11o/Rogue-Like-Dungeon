#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dungeon_generation.h"
#include "dungeon_save_and_load.h"
#include "dijkstra.h"
#include "characters.h"
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "display.h"
#include <ncurses.h>
#include <iostream>
#include <vector>
#include "description_parsing.h"
#include "display.h"
#include "items.h"

#define WIDTH 80
#define HEIGHT 21

// Main function
int main(int argc, char *argv[])
{
    srand(time(NULL));
    // Monster Parsing
    // std::vector<monster_parse> monsters;
    // int parse_result = parse_monster_description(monsters);
    Dungeon *dungeon = (Dungeon *)malloc(sizeof(Dungeon));
    memset(dungeon, 0, sizeof(Dungeon));
    Character_Map *character_map = new Character_Map();
    int save = 0, load = 0, num_monster = 10;
    char *home;
    char *dungeon_file;
    int dungeon_file_length;

    home = getenv("HOME");
    dungeon_file_length = strlen(home) + strlen("/.rlg327/dungeon") + 1; // +1 for the null byte
    dungeon_file = (char *)malloc(dungeon_file_length * sizeof(*dungeon_file));
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

    std::vector<monster_parse> monsters;

    if (!parse_monster_description(monsters))
    {
        // std::cout << "Parsed Monsters" << std::endl;
        init_character_map_parsed(dungeon, character_map, monsters, num_monster);
    }
    else
    {
        // std::cout << "Random Monsters" << std::endl;
        init_character_map(dungeon, character_map, num_monster);
    }

    //print damage strings
    for(int l; l < num_monster; l++){
        std::cout << "Monster " << character_map->monsters[l]->NAME << " DAM String: " << character_map->monsters[l]->DAM << std::endl;
    }
    
    std::vector<item_parse> items;

    item_map *map_items = new item_map();
    if (!parse_item_description(items))
    {
        init_item_map(dungeon, map_items, items);
    }
    else
    {
        std::cout << "No items list provided" << std::endl;
    }

    init_terminal();
    // for(int k = 0; k < 15; k++){
    //     if(map_items->items_list[k] == nullptr){
    //         break;
    //     }
    //     pc_pickup(character_map, map_items->items_list[k]);
    // }
    // character_map->pc->print_inventory();

    int distMapTunnel[HEIGHT][WIDTH];    // Distance map for tunneling monsters
    int distMapNonTunnel[HEIGHT][WIDTH]; // Distance map for non-tunneling monsters

    if (Dijkstra_NonTunneling(dungeon, character_map, distMapNonTunnel))
    {
        printf("Failed Non-Tunneling Dijkstra\n");
        return -1;
    }

    if (Dijkstras_Tunneling(dungeon, character_map, distMapTunnel))
    {
        printf("Failed Tunneling Dijkstra\n");
        return -1;
    }

    int i = 0;
    int Victory = 0;

    PriorityQueue *movementQueue = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    memset(movementQueue, 0, sizeof(PriorityQueue));
    init_movement_queue(movementQueue, character_map);

    int quit = 0;
    bool BOSS_KILL = false;

    display_window_FOG(character_map, map_items, dungeon);

    // usleep(3000000);
    char input = ' ';

    while (!quit)
    {
        // input = pc_input(&input);

        if (input == 'Q')
        {
            quit = 1;
            update_quit();
            usleep(500000);
        }
        else if (input == 'm')
        {
            display_monster_list(dungeon, map_items, character_map, num_monster, &input);
        }
        else if (input == 'T')
        {
            display_tunneling_distances(distMapTunnel, dungeon, map_items, character_map, &input);
        }
        else if (input == 'D')
        {
            display_non_tunneling_distances(distMapNonTunnel, dungeon, map_items, character_map, &input);
        }
        else if (input == 'H')
        {
            display_hardness(dungeon, map_items, character_map, &input);
        }
        else if (input == 'C')
        {
            display_character_info(dungeon, map_items, character_map, num_monster, &input);
        }
        else if (input == 'f')
        {
            update_window(character_map, map_items, dungeon, &input);
        }
        else if (input == 'g')
        {
            teleport(dungeon, movementQueue, map_items, character_map, &input);
        }
        else if (input == '|')
        {
            display_queue(movementQueue, dungeon, map_items, character_map, num_monster, &input);
        }
        else if (input == 'i')
        {
            display_inventory(dungeon, map_items, character_map, &input);
        }
        else if (input == 'e')
        {
            display_equipment(dungeon, map_items, character_map, &input);
        }
        else if (input == 'I')
        {
            inspect_item(dungeon, map_items, character_map, &input);
        }
        else if (input == 'L')
        {
            inspect_monster(dungeon, map_items, character_map, &input);
        }else if(input == 'x'){
            expunge_item(character_map, items, &input);
        }else if(input == 't'){
            take_off_item(character_map, map_items, &input);
        }else if(input == 'w'){
            wear(character_map, &input);
        }else if(input == 'd'){
            drop_item(dungeon, character_map, map_items, &input);
        }

        if (!Victory)
        {
            //update_window(character_map, map_items, dungeon, &input);
            display_window_FOG(character_map, map_items, dungeon);
            process_next_event(movementQueue, dungeon, map_items, items, monsters, character_map, distMapTunnel, distMapNonTunnel, &input, num_monster, &BOSS_KILL);

            if (character_map->pc->HP <= 0 || character_map->pc->alive == 0 || character_map->pc->HP > 25)
            {
                update_lose();
                Victory = 1;
            }

            usleep(50000);
            i++;

            if (i % (num_monster + 1) == 0)
            {
                if (Dijkstra_NonTunneling(dungeon, character_map, distMapNonTunnel))
                {
                    printf("Failed Non-Tunneling Dijkstra\n");
                    return -1;
                }
                if (Dijkstras_Tunneling(dungeon, character_map, distMapTunnel))
                {
                    printf("Failed Tunneling Dijkstra\n");
                    return -1;
                }
            }

            if (BOSS_KILL)
            {
                update_win();
                Victory = 1;
            }
        }
        else
        {
            input = pc_input(&input);

            if (input == 'Q')
            {
                quit = 1;
                update_quit();
                usleep(500000);
            }
        }
    }

    //     // printCharacterMap(&character_map);
    //     // printQueue(&movementQueue,&character_map);
    //     //  printf("\nCharacter %c | ERATIC %d | TUNNEL %d | TELEPATHIC %d | INTELLIGENT %d\n",character_map.characters[1].character,(character_map.characters[1].characteristics & 0x00000008),
    //     //      (character_map.characters[1].characteristics & 0x00000004),(character_map.characters[1].characteristics & 0x00000002), (character_map.characters[1].characteristics & 0x00000001));
    //     //  printf("Character %c: ERATIC %d: TUNNEL %d\n",character_map.characters[1].character,(character_map.characters[1].characteristics & 0x00000008), (character_map.characters[1].characteristics & 0x00000004));
    endwin();
    // character_map->pc->print_inventory();

    clean_character_map(character_map);
    delete character_map;
    clean_item_map(map_items);
    delete map_items;
    free(dungeon_file);
    free(dungeon);
    free(movementQueue);

    return 0;
}