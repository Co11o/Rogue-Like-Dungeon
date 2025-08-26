#include "description_parsing.h"
#include "items.h"
#include "dungeon_generation.h"
#include <algorithm>
#include <vector>
#include <ncurses.h>
#include <regex>
#include <unistd.h>

#define MAX_ITEMS 20
#define MIN_ITEMS 10
#define WIDTH 80
#define HEIGHT 21

using namespace std;

char assign_char(std::string TYPE)
{
    char character;
    if (TYPE == "WEAPON")
    {
        character = '|';
    }
    else if (TYPE == "OFFHAND")
    {
        character = ')';
    }
    else if (TYPE == "RANGED")
    {
        character = '}';
    }
    else if (TYPE == "ARMOR")
    {
        character = '[';
    }
    else if (TYPE == "HELMET")
    {
        character = ']';
    }
    else if (TYPE == "CLOAK")
    {
        character = '(';
    }
    else if (TYPE == "GLOVES")
    {
        character = '{';
    }
    else if (TYPE == "BOOTS")
    {
        character = '\\';
    }
    else if (TYPE == "RING")
    {
        character = '=';
    }
    else if (TYPE == "AMULET")
    {
        character = '"';
    }
    else if (TYPE == "LIGHT")
    {
        character = '_';
    }
    else if (TYPE == "SCROLL")
    {
        character = '~';
    }
    else if (TYPE == "BOOK")
    {
        character = '?';
    }
    else if (TYPE == "FLASK")
    {
        character = '!';
    }
    else if (TYPE == "GOLD")
    {
        character = '$';
    }
    else if (TYPE == "AMMUNITION")
    {
        character = '/';
    }
    else if (TYPE == "FOOD")
    {
        character = ',';
    }
    else if (TYPE == "WAND")
    {
        character = '-';
    }
    else if (TYPE == "CONTAINER")
    {
        character = '%';
    }
    else
    {
        character = '*';
    }
    return character;
}

int load_item(std::vector<item_parse> &items)
{
    const int max_attempts = 5000; // safety limit
    int attempts = 0;

    while (attempts++ < max_attempts)
    {
        usleep(1000);
        int index = rand() % items.size(); // Step 1
        // std::cout << "Test " << index <<std::endl;
        if (!items[index].is_valid())
        { // Step 2
            // std::cout << "  Invalid " <<std::endl;
            continue;
        }
        if ((rand() % 100) >= items[index].RRTY)
        { // Step 3
            // std::cout << "  RRTY: " << items[index].RRTY <<std::endl;
            continue;
        }
        // Step 4 - Check if artifact
        if (items[index].ART == "TRUE")
        {
            // std::cout << "  Unique " <<std::endl;
            items[index].has_art = false;
        }
        // std::cout << "Valid \n" << index <<std::endl;
        return index; // Step 5
    }

    // If we reach here, something went wrong
    std::cerr << "Fatal Error: Unable to load a valid item after " << max_attempts << " attempts." << std::endl;
    std::exit(EXIT_FAILURE);
}

int init_item_map(Dungeon *dungeon, item_map *map, std::vector<item_parse> &items)
{
    if (!map)
        return -1;
    int i = 0;
    while (i < MAX_ITEMS)
    {
        // Get item
        usleep(100);

        // if(map->is_picked_up[i] == true)
        // {
        //     i++;
        //     continue;
        // }

        if (map->items_list[i])
        {
            delete map->items_list[i];
            map->items_list[i] = nullptr;
        }
        map->items_list[i] = new item(items[load_item(items)]);

        // Set location of item
        Room room = dungeon->rooms[rand() % dungeon->room_count];
        int location_x = room.x + (rand() % room.width);
        int location_y = room.y + (rand() % room.height);
        map->coordinates[i] = {location_x, location_y};
        map->is_picked_up[i] = false;
        if (i > MIN_ITEMS && (rand() % 4 == 0))
        {
            break;
        }
        i++;
    }

    // Fill remaining slots with default/empty items
    for (int j = i; j < MAX_ITEMS; j++)
    {
        if (map->items_list[j])
        {
            delete map->items_list[j];
        }
        map->items_list[j] = nullptr;
        map->coordinates[j] = {-1, -1};
        map->is_picked_up[i] = false;
    }

    return 0;
}

void clean_item_map(item_map *map)
{
    if (!map)
        return;

    for (int i = 0; i < MAX_ITEMS; i++)
    {
        if (map->items_list[i])
        {
            delete map->items_list[i];
            map->items_list[i] = nullptr;
        }
    }
}
