#include "display.h"
#include <ncurses.h>
#include <cstring>
#include "characters.h"
#include "dungeon_generation.h"
#include <limits.h>
#include <map>
#include <sstream>
#include "items.h"

#define ATTRIBUTE_INTELLIGENCE 0x00000001
#define ATTRIBUTE_TELEPATHIC 0x00000002
#define ATTRIBUTE_TUNNELING 0x00000004
#define ATTRIBUTE_ERATIC 0x00000008

#define COLOR_ERROR 8

void display_message(char *message)
{
    move(0, 0);
    clrtoeol();
    mvprintw(0, 0, "Message: %s", message);
}

void display_message_str(const char *message)
{
    move(0, 0);
    clrtoeol();
    mvprintw(0, 0, "Message: %s", message);
}

void init_terminal(void)
{
    initscr();
    raw();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    start_color();
    init_pair(COLOR_ERROR, COLOR_YELLOW, COLOR_CYAN);
    init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK); // Invis
    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
    init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
}

std::map<std::string, int> color_lookup = {
    {"COLOR_BLACK", COLOR_BLACK},
    {"COLOR_RED", COLOR_RED},
    {"COLOR_GREEN", COLOR_GREEN},
    {"COLOR_YELLOW", COLOR_YELLOW},
    {"COLOR_BLUE", COLOR_BLUE},
    {"COLOR_MAGENTA", COLOR_MAGENTA},
    {"COLOR_CYAN", COLOR_CYAN},
    {"COLOR_WHITE", COLOR_WHITE}};

int get_first_color(const std::string &color_string)
{
    std::istringstream iss(color_string);
    std::string first_color;
    iss >> first_color; // Extract only the first word

    auto it = color_lookup.find(first_color);
    if (it != color_lookup.end())
    {
        return it->second;
    }

    return COLOR_ERROR; // Default if the color is invalid
}

void update_window(Character_Map *character_map, item_map *item_map, Dungeon *dungeon, char *input)
{
    // display_message_str("Displaying Dungeon without fog press 'esc' to exit view");
    for (int y = 1; y < HEIGHT + 1; y++) // Start from line 1
    {
        move(y, 0);
        clrtoeol();
    }

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            chtype ch;

            if (character_map->grid[y][x] == ' ')
            {
                int color = COLOR_WHITE;
                bool item_found = false;

                for (int i = 0; i < MAX_ITEMS && item_map->items_list[i] != nullptr; i++)
                {
                    if (item_map->is_picked_up[i])
                    {
                        continue; // Skip picked up items
                    }
                    if (item_map->coordinates[i].x == x && item_map->coordinates[i].y == y)
                    {
                        item_found = true;

                        if (!item_map->items_list[i]->COLOR.empty())
                        {
                            std::string color_str = item_map->items_list[i]->COLOR;
                            color = get_first_color(color_str);
                        }

                        attron(COLOR_PAIR(color));
                        ch = (chtype)item_map->items_list[i]->character;
                        mvaddch(y + 1, x, ch);
                        attroff(COLOR_PAIR(color));
                        break;
                    }
                }

                if (!item_found)
                {
                    ch = (chtype)dungeon->grid[y][x].type;
                    mvaddch(y + 1, x, ch);
                }
            }
            else
            {
                int color = COLOR_WHITE;
                //
                for (int i = 0; i < MAX_MONSTERS && character_map->monsters[i] != nullptr; i++)
                {
                    if (character_map->monsters[i]->alive == 1 && x == character_map->monsters[i]->location_x && y == character_map->monsters[i]->location_y)
                    {
                        std::string &color_str = character_map->monsters[i]->COLOR;
                        color = get_first_color(color_str);
                        break;
                    }
                }

                attron(COLOR_PAIR(color));
                ch = (chtype)character_map->grid[y][x];
                mvaddch(y + 1, x, ch); // Print character
                attroff(COLOR_PAIR(color));
            }
        }
    }
    if(character_map->pc->HP <= 0 || character_map->pc->HP > 25){
        return;
    }
    mvprintw(23, 0, "HP: %3d", character_map->pc->HP);
    refresh();

    while (1)
    {
        *input = pc_input(input);

        if (*input == '%')
        {
            display_window_FOG(character_map, item_map, dungeon);
            break;
        }
    }
}

void update_win(void)
{
    clear();

    attron(COLOR_PAIR(COLOR_YELLOW));
    int color = COLOR_YELLOW;
    attron(A_BOLD);

    mvprintw(11, 25, "Victory is claimed: Press Q to quit");

    attroff(A_BOLD);
    attroff(COLOR_PAIR(color));

    refresh();
}

void update_lose(void)
{
    clear();

    attron(COLOR_PAIR(COLOR_RED));
    int color = COLOR_RED;
    attron(A_BOLD);

    mvprintw(11, 25,
             "PC has been slain"
             " GAME OVER Press Q to quit:");

    attroff(A_BOLD);
    attroff(COLOR_PAIR(color));

    refresh();
}

void update_quit(void)
{
    clear();

    attron(COLOR_PAIR(COLOR_CYAN));
    int color = COLOR_CYAN;
    attron(A_BOLD);

    mvprintw(11, 25, "Quit");

    attroff(A_BOLD);
    attroff(COLOR_PAIR(color));

    refresh();
}

void display_monster_list(Dungeon *dungeon, item_map *item_map, Character_Map *map, int num_Monster, char *input)
{
    display_message_str("Turn used to view map");
    int pc_x = map->pc->location_x;
    int pc_y = map->pc->location_y;

    char list[num_Monster][50];
    char vertical[12], horizontal[10];
    int i = 0;
    int dx, dy;
    while (map->monsters[i] != nullptr && i < num_Monster)
    {
        if (map->monsters[i]->alive == 0)
        {

            sprintf(list[i], "%c, has died           ", map->monsters[i]->get_Character());
        }
        else
        {
            dx = pc_x - map->monsters[i]->location_x;
            dy = pc_y - map->monsters[i]->location_y;
            if (dx < 0)
            {
                sprintf(horizontal, "East %2d", -dx);
            }
            else
            {
                sprintf(horizontal, "West %2d", dx);
            }
            if (dy < 0)
            {
                sprintf(vertical, "South %2d", -dy);
            }
            else
            {
                sprintf(vertical, "North %2d", dy);
            }
            sprintf(list[i], "%c, %s and %s", map->monsters[i]->get_Character(), vertical, horizontal);
        }
        i++;
    }

    int j, start;
    mvprintw(4, 25, "                                  ");
    mvprintw(3, 25, "                                  ");
    mvprintw(16, 25, "                                  ");
    mvprintw(17, 25, "                                  ");
    mvprintw(5, 25, " Monster List press 'esc' to exit ");
    for (start = 0; start < 10 && start < num_Monster; start++)
    {
        if (strstr(list[start], "has died") != NULL)
        {

            attron(COLOR_PAIR(COLOR_RED));
            int color = COLOR_RED;
            attron(A_BOLD);
            mvprintw(6 + start, 25, "       %s       ", list[start]);

            attroff(A_BOLD);
            attroff(COLOR_PAIR(color));
        }
        else
        {
            mvprintw(6 + start, 25, "       %s       ", list[start]);
        }
    }
    start = 0;

    while (1)
    {
        *input = pc_input(input);

        if (*input == '%')
        {
            display_window_FOG(map, item_map, dungeon);
            break;
        }
        if (num_Monster > 10)
        {
            if (*input == '^')
            {
                start--;
                if (start < 0)
                {
                    start = 0;
                }
            }
            if (*input == 'v')
            {
                start++;
                if (start > num_Monster - 10)
                {
                    start = num_Monster - 10;
                }
            }
            for (j = start; j < start + 10; j++)
            {

                if (strstr(list[j], "has died") != NULL)
                {

                    attron(COLOR_PAIR(COLOR_RED));
                    int color = COLOR_RED;
                    attron(A_BOLD);

                    mvprintw(6 + j - start, 25, "       %s       ", list[j]);

                    attroff(A_BOLD);
                    attroff(COLOR_PAIR(color));
                }
                else
                {
                    mvprintw(6 + j - start, 25, "       %s       ", list[j]);
                }
            }
        }
    }
}

void display_tunneling_distances(int distMap[HEIGHT][WIDTH], Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input)
{
    display_message_str("Turn used to view distance map. press 'esc' to exit");
    for (int y = 1; y < HEIGHT + 1; y++)
    {
        move(y, 0);
        clrtoeol();
    }

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            if (y == map->pc->location_y && x == map->pc->location_x)
            {
                attron(COLOR_PAIR(COLOR_GREEN));
                int color = COLOR_GREEN;
                attron(A_BOLD);
                mvprintw(y + 1, x, "@");
                attroff(A_BOLD);
                attroff(COLOR_PAIR(color));
            }
            else
            {
                mvprintw(y + 1, x, "%d", (distMap[y][x] % 10));
            }
        }
    }
    while (1)
    {
        *input = pc_input(input);

        if (*input == '%')
        {
            display_window_FOG(map, item_map, dungeon);
            break;
        }
    }
}

void display_non_tunneling_distances(int distMap[HEIGHT][WIDTH], Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input)
{
    display_message_str("Turn used to view tunneling distance map. press 'esc' to exit");
    for (int y = 1; y < HEIGHT + 1; y++)
    {
        move(y, 0);
        clrtoeol();
    }

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            if (distMap[y][x] == INT_MAX)
            {
                mvprintw(y + 1, x, " ");
            }
            else if (y == map->pc->location_y && x == map->pc->location_x)
            {
                attron(COLOR_PAIR(COLOR_GREEN));
                int color = COLOR_GREEN;
                attron(A_BOLD);
                mvprintw(y + 1, x, "@");
                attroff(A_BOLD);
                attroff(COLOR_PAIR(color));
            }
            else
            {
                mvprintw(y + 1, x, "%d", (distMap[y][x] % 10));
            }
        }
    }
    while (1)
    {
        *input = pc_input(input);

        if (*input == '%')
        {
            display_window_FOG(map, item_map, dungeon);
            break;
        }
    }
}

void display_hardness(Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input)
{
    display_message_str("Turn used to view hardness map. press 'esc' to exit");
    for (int y = 1; y < HEIGHT + 1; y++)
    {
        move(y, 0);
        clrtoeol();
    }

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            mvprintw(y + 1, x, "%d", (dungeon->grid[y][x].hardness % 10));
        }
    }
    while (1)
    {
        *input = pc_input(input);

        if (*input == '%')
        {
            display_window_FOG(map, item_map, dungeon);
            break;
        }
    }
}

void display_character_info(Dungeon *dungeon, item_map *item_map, Character_Map *map, int num_Monster, char *input)
{
    display_message_str("Turn used to view character info");
    char list[num_Monster + 1][50];
    char intelligent[2], telepathic[2], tunneling[2], eratic[2], speed[3];
    int i = 0;

    sprintf(list[0], "   %c     X   X   X   X   %2d", map->pc->get_Character(), map->pc->get_Speed());
    while (map->monsters[i] != nullptr && i < num_Monster)
    {
        if (map->monsters[i]->alive == 0)
        {
            sprintf(list[i + 1], "   %c, has died            ", map->monsters[i]->get_Character());
        }
        else
        {
            /*
            #define ATTRIBUTE_INTELLIGENCE 0x00000001
            #define ATTRIBUTE_TELEPATHIC 0x00000002
            #define ATTRIBUTE_TUNNELING 0x00000004
            #define ATTRIBUTE_ERATIC 0x00000008
            */

            if ((map->monsters[i]->get_Characteristics() & ATTRIBUTE_INTELLIGENCE) == ATTRIBUTE_INTELLIGENCE)
            {
                sprintf(intelligent, "T");
            }
            else
            {
                sprintf(intelligent, "F");
            }

            if ((map->monsters[i]->get_Characteristics() & ATTRIBUTE_TELEPATHIC) == ATTRIBUTE_TELEPATHIC)
            {
                sprintf(telepathic, "T");
            }
            else
            {
                sprintf(telepathic, "F");
            }

            if ((map->monsters[i]->get_Characteristics() & ATTRIBUTE_TUNNELING) == ATTRIBUTE_TUNNELING)
            {
                sprintf(tunneling, "T");
            }
            else
            {
                sprintf(tunneling, "F");
            }

            if ((map->monsters[i]->get_Characteristics() & ATTRIBUTE_ERATIC) == ATTRIBUTE_ERATIC)
            {
                sprintf(eratic, "T");
            }
            else
            {
                sprintf(eratic, "F");
            }

            sprintf(speed, "%2d", map->monsters[i]->get_Speed());
            //                MON INT TLP TUN ERT SPD
            sprintf(list[i + 1], "   %c     %s   %s   %s   %s   %s", map->monsters[i]->get_Character(), intelligent, telepathic, tunneling, eratic, speed);
        }
        i++;
    }

    int min = 0;
    if (num_Monster < 10)
    {
        min = num_Monster;
    }
    else
    {
        min = 10;
    }

    int j, start;
    mvprintw(4, 25, "                                    ");
    mvprintw(3, 25, "                                    ");
    mvprintw(7 + min, 25, "                                    ");
    mvprintw(7 + min + 1, 25, "                                    ");
    mvprintw(5, 25, " Character List press 'esc' to exit ");
    mvprintw(6, 25, " %s        ", list[0]);
    mvprintw(7, 25, " MONSTER INT TLP TUN ERT SPD ");
    for (start = 0; start < 10 && start < num_Monster; start++)
    {
        if (strstr(list[start + 1], "has died") != NULL)
        {

            attron(COLOR_PAIR(COLOR_RED));
            int color = COLOR_RED;
            attron(A_BOLD);
            mvprintw(8 + start, 25, " %s        ", list[start + 1]);

            attroff(A_BOLD);
            attroff(COLOR_PAIR(color));
        }
        else
        {
            mvprintw(8 + start, 25, " %s        ", list[start + 1]);
        }
    }
    start = 0;

    while (1)
    {
        *input = pc_input(input);

        if (*input == '%')
        {
            display_window_FOG(map, item_map, dungeon);
            break;
        }
        if (num_Monster > 10)
        {
            if (*input == '^')
            {
                start--;
                if (start < 0)
                {
                    start = 0;
                }
            }
            if (*input == 'v')
            {
                start++;
                if (start > num_Monster - 10)
                {
                    start = num_Monster - 10;
                }
            }
            for (j = start; j < start + 10; j++)
            {

                if (strstr(list[j + 1], "has died") != NULL)
                {

                    attron(COLOR_PAIR(COLOR_RED));
                    int color = COLOR_RED;
                    attron(A_BOLD);

                    mvprintw(8 + j - start, 25, " %s        ", list[j + 1]);

                    attroff(A_BOLD);
                    attroff(COLOR_PAIR(color));
                }
                else
                {
                    mvprintw(8 + j - start, 25, " %s        ", list[j + 1]);
                }
            }
        }
    }
}

void display_window_FOG(Character_Map *map, item_map *item_map, Dungeon *dungeon)
{
    // Clear display
    for (int y = 1; y < HEIGHT + 1; y++)
    {
        move(y, 0);
        clrtoeol();
    }

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            chtype ch;

            if (map->pc->discoverd[y][x] != ' ')
            {
                if (abs(map->pc->location_x - x) < 3 && abs(map->pc->location_y - y) < 3 && map->grid[y][x] != ' ')
                {
                    // PC's current field of view
                    int color = COLOR_WHITE;

                    for (int i = 0; i < MAX_MONSTERS && map->monsters[i] != nullptr; i++)
                    {
                        if (map->monsters[i]->alive == 1 &&
                            x == map->monsters[i]->location_x &&
                            y == map->monsters[i]->location_y)
                        {

                            if (!map->monsters[i]->COLOR.empty())
                            {
                                std::string color_str = map->monsters[i]->COLOR;
                                color = get_first_color(color_str);
                            }
                            break;
                        }
                    }

                    attron(COLOR_PAIR(color));
                    ch = (chtype)map->grid[y][x];
                    mvaddch(y + 1, x, ch);
                    attroff(COLOR_PAIR(color));
                }
                else
                {
                    // Outside FOV but discovered before
                    int color = COLOR_WHITE;
                    bool item_found = false;

                    for (int i = 0; i < MAX_ITEMS && item_map->items_list[i] != nullptr; i++)
                    {
                        if (item_map->coordinates[i].x == x && item_map->coordinates[i].y == y)
                        {
                            item_found = true;
                            if (item_map->is_picked_up[i])
                            {
                                ch = (chtype)map->pc->discoverd[y][x];
                                mvaddch(y + 1, x, ch);
                                continue; // Skip picked up items
                            }
                            if (!item_map->items_list[i]->COLOR.empty())
                            {
                                std::string color_str = item_map->items_list[i]->COLOR;
                                color = get_first_color(color_str);
                            }

                            attron(COLOR_PAIR(color));
                            ch = (chtype)item_map->items_list[i]->character;
                            mvaddch(y + 1, x, ch);
                            attroff(COLOR_PAIR(color));
                            break;
                        }
                    }

                    if (!item_found)
                    {
                        ch = (chtype)map->pc->discoverd[y][x];
                        mvaddch(y + 1, x, ch);
                    }
                }
            }
            else
            {
                // Undiscovered tile
                ch = ' ';
                mvaddch(y + 1, x, ch);
            }
        }
    }
    mvprintw(23, 0, "HP: %d", map->pc->HP);
    refresh();
}

void display_queue(PriorityQueue *Queue, Dungeon *dungeon, item_map *item_map, Character_Map *map, int num_Monster, char *input)
{
    display_message_str("Turn used to view queue");
    char list[num_Monster + 1][50];
    int i = 0;
    PriorityQueue *tmp = (PriorityQueue *)malloc(sizeof(PriorityQueue));
    memset(tmp, 0, sizeof(PriorityQueue));
    *tmp = *Queue;
    bool found = false;

    while (i < tmp->size)
    {
        queue_node node = tmp->queue[i];
        found = false;

        if (map->pc->location_x == node.x_location && map->pc->location_y == node.y_location)
        {
            sprintf(list[i], "  %c | x: %2d | y: %2d | priority:%4d  ", map->pc->get_Character(), node.x_location, node.y_location, node.priority);
            found = true;
            break;
        }
        else
        {
            int k = 0;
            while (map->monsters[k] != nullptr && k < (MAX_MONSTERS))
            {
                if (map->monsters[k]->location_x == node.x_location && map->monsters[k]->location_y == node.y_location)
                {
                    sprintf(list[i], "  %c | x: %2d | y: %2d | priority:%4d  ", map->monsters[k]->get_Character(), node.x_location, node.y_location, node.priority);
                    found = true;
                    break;
                }
                k++;
            }

            // If no monster was found, add "I" entry
            if (!found)
            {
                sprintf(list[i], "  I | x: %2d | y: %2d | priority:%4d  ", node.x_location, node.y_location, node.priority);
            }
        }
        i++;
    }

    int min = 0;
    if (tmp->size < 10)
    {
        min = tmp->size;
    }
    else
    {
        min = 10;
    }

    int j, start;
    mvprintw(4, 25, "                                    ");
    mvprintw(3, 25, "                                    ");
    mvprintw(7 + min, 25, "                                    ");
    mvprintw(7 + min + 1, 25, "                                    ");
    mvprintw(5, 25, " Queue List press 'esc' to exit ");
    mvprintw(6, 25, "                                    ");
    mvprintw(7, 25, "                                    ");
    for (start = 0; start < 10 && start < tmp->size; start++)
    {
        mvprintw(8 + start, 25, " %s        ", list[start]);
    }
    start = 0;

    while (1)
    {
        *input = pc_input(input);

        if (*input == '%')
        {
            free(tmp);
            display_window_FOG(map, item_map, dungeon);
            break;
        }
        if (num_Monster > 10)
        {
            if (*input == '^')
            {
                start--;
                if (start < 0)
                {
                    start = 0;
                }
            }
            if (*input == 'v')
            {
                start++;
                if (start > num_Monster - 10)
                {
                    start = num_Monster - 10;
                }
            }
            for (j = start; j < start + 10; j++)
            {
                mvprintw(8 + j - start, 25, " %s        ", list[j]);
            }
        }
    }
}

void display_teleport(Character_Map *character_map, Dungeon *dungeon, int teleport_x, int teleport_y)
{

    for (int y = 1; y < HEIGHT + 1; y++) // Start from line 1
    {
        move(y, 0);
        clrtoeol();
    }

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            chtype ch;
            if (teleport_x == x && teleport_y == y)
            {
                ch = (chtype)'*';
                mvaddch(y + 1, x, ch); // Print character
            }
            else if (character_map->grid[y][x] == ' ')
            {
                ch = (chtype)dungeon->grid[y][x].type;
                if (dungeon->grid[y][x].hardness != 0)
                {
                    mvaddch(y + 1, x, ch);
                }
                else
                {
                    mvaddch(y + 1, x, ch); // Print character
                }
            }
            else
            {
                ch = (chtype)character_map->grid[y][x];
                mvaddch(y + 1, x, ch); // Print character
            }
        }
    }
    refresh();
}

void display_inventory(Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input)
{
    display_message_str("Turn used to view inventory");

    char list[10][77]; // 76 chars + null terminator

    for (int i = 0; i < 10; i++)
    {
        int slot_index = 12 + i;
        char slot_char = '0' + i;

        if (map->pc->inventory.items_list[slot_index] == nullptr)
        {
            snprintf(list[i], sizeof(list[i]),
                     " Slot %c | %-66s", slot_char, "EMPTY");
        }
        else
        {
            std::string name = map->pc->inventory.items_list[slot_index]->Name;
            std::string type = map->pc->inventory.items_list[slot_index]->TYPE;

            // Truncate to safe lengths
            if (name.length() > 40) name = name.substr(0, 40);
            if (type.length() > 10) type = type.substr(0, 10);

            snprintf(list[i], sizeof(list[i]),
                     " Slot %c | NAME: %-40s | TYPE: %-10s", slot_char, name.c_str(), type.c_str());
        }
    }

    mvprintw(5, 25, " Inventory press 'esc' to exit ");
    for (int i = 0; i < 10; i++)
    {
        mvprintw(6 + i, 1, "%s  ", list[i]);
    }

    while (1)
    {
        *input = pc_input(input);
        if (*input == '%')
        {
            display_window_FOG(map, item_map, dungeon);
            break;
        }
    }
}


void display_equipment(Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input)
{
    display_message_str("Turn used to view equipment");

    char list[12][77]; // 76 chars + null terminator

    for (int i = 0; i < 12; i++)
    {
        char slot_char = 'a' + i;

        if (map->pc->inventory.items_list[i] == nullptr)
        {
            snprintf(list[i], sizeof(list[i]),
                     " Slot %c | %-66s", slot_char, "EMPTY");
        }
        else
        {
            std::string name = map->pc->inventory.items_list[i]->Name;
            std::string type = map->pc->inventory.items_list[i]->TYPE;

            // Truncate name and type if they are too long
            if (name.length() > 40)
                name = name.substr(0, 40);
            if (type.length() > 10)
                type = type.substr(0, 10);

            // Construct a clean, fixed-width line
            snprintf(list[i], sizeof(list[i]),
                     " Slot %c | NAME: %-40s | TYPE: %-10s", slot_char, name.c_str(), type.c_str());
        }
    }

    // Display the inventory header
    mvprintw(5, 1, "  Equipment press 'esc' to exit      ");

    // Display the equipment list
    for (int i = 0; i < 12; i++)
    {
        mvprintw(6 + i, 1, "%s  ", list[i]);
    }

    // Wait for user input
    while (1)
    {
        *input = pc_input(input);
        if (*input == '%')
        {
            display_window_FOG(map, item_map, dungeon);
            break;
        }
    }
}

void inspect_item(Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input)
{
    int exit = 0;
    while (!exit)
    {
        display_message_str("Turn used to view items, press esc to exit");

        *input = pc_input(input);

        if (*input == '%')
        {
            display_window_FOG(map, item_map, dungeon);
            exit = 1;
            break;
        }

        int index = *input;
        if (index >= 'a' && index <= 'l')
        {
            index -= 'a';
        }
        else if (index >= '0' && index <= '9')
        {
            index = index - '0' + 12;
        }
        else
        {
            display_message_str("Invalid item slot");
            return;
        }

        int line_index = 0;
        int start = 0;
        char list[10][77];

        if (!map->pc->inventory.items_list[index])
        {
            // Line 0: Slot label and "EMPTY"
            snprintf(list[0], sizeof(list[0]), " Slot %c | %-65s",
                     (index >= 12) ? ('0' + index - 12) : ('a' + index),
                     "EMPTY");

            // Line 1: Placeholder for description
            snprintf(list[1], sizeof(list[1]), " %-*s", (int)(sizeof(list[1]) - 2), "No Description Available");

            // Lines 2-9: Fill with spaces to clear
            for (int i = 2; i < 10; i++)
            {
                snprintf(list[i], sizeof(list[i]), " %-*s", (int)(sizeof(list[i]) - 2), " ");
            }
        }
        else
        {
            std::string desc = map->pc->inventory.items_list[index]->DESC;
            std::istringstream desc_stream(desc);
            std::vector<std::string> lines;
            std::string line;

            // Split description into lines
            while (std::getline(desc_stream, line))
                lines.push_back(line);

            // Fill list with lines or blank if out of bounds
            for (int i = 0; i < 10; i++)
            {
                if (i < (int)lines.size())
                {
                    // Clip the line if it's too long for safety
                    snprintf(list[i], sizeof(list[i]), " %-*.*s",
                             (int)(sizeof(list[i]) - 2),
                             (int)(sizeof(list[i]) - 2),
                             lines[i].c_str());
                }
                else
                {
                    snprintf(list[i], sizeof(list[i]), " %-*s", (int)(sizeof(list[i]) - 2), " ");
                }
            }
        }

        // Display UI section cleanup
        mvprintw(3, 25, "                                  ");
        mvprintw(4, 25, "                                  ");
        mvprintw(16, 25, "                                  ");
        mvprintw(18, 25, "                                  ");
        mvprintw(5, 1, " Item Description: Press 'N' to select new item or Exit            ");

        // Display lines
        for (int i = 0; i < 10; i++)
        {
            mvprintw(6 + i, 1, " %s ", list[i]);
        }

        while (1)
        {
            display_message_str("Press 'N' to select new item or to Exit");
            *input = pc_input(input);

            if (*input == 'N')
            {
                display_window_FOG(map, item_map, dungeon);
                break;
            }

            if (line_index > 10)
            {
                if (*input == '^' && start > 0)
                    start--;
                else if (*input == 'v' && start < line_index - 10)
                    start++;

                for (int j = start; j < start + 10; j++)
                {
                    mvprintw(6 + j - start, 1, " %s        ", (j < line_index) ? list[j] : "                                    ");
                }
            }
        }
    }
}

std::vector<std::string> wrap_text(const std::string &text, size_t width)
{
    std::istringstream words(text);
    std::string word;
    std::string line;
    std::vector<std::string> wrapped;

    while (words >> word)
    {
        if (line.length() + word.length() + 1 > width)
        {
            wrapped.push_back(line);
            line = word;
        }
        else
        {
            if (!line.empty())
                line += " ";
            line += word;
        }
    }
    if (!line.empty())
        wrapped.push_back(line);

    return wrapped;
}

void inspect_monster(Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input)
{
    int NEW_X = map->pc->location_x;
    int NEW_Y = map->pc->location_y;

    int dx[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
    int dy[] = {1, 1, 1, 0, 0, 0, -1, -1, -1};

    while (1)
    {
        display_message_str("In monster inspection mode. 't' to inspect, 'esc' to exit.");
        *input = pc_input(input);

        if (*input == '%')
        {
            display_window_FOG(map, item_map, dungeon);
            break;
        }

        char temp = *input;
        int move_to = -1;

        if (*input == '7' || *input == 'y')
            temp = '7';
        else if (*input == '8' || *input == 'k')
            temp = '8';
        else if (*input == '9' || *input == 'u')
            temp = '9';
        else if (*input == '6' || *input == 'l')
            temp = '6';
        else if (*input == '3' || *input == 'n')
            temp = '3';
        else if (*input == '2' || *input == 'j')
            temp = '2';
        else if (*input == '1' || *input == 'b')
            temp = '1';
        else if (*input == '4' || *input == 'h')
            temp = '4';
        else if (*input == '5' || *input == '.' || *input == ' ')
            temp = '5';

        if (temp >= '1' && temp <= '9')
        {
            move_to = temp - '1';
            if (temp != '5')
            {
                int next_x = NEW_X + dx[move_to];
                int next_y = NEW_Y + dy[move_to];
                if (next_x >= 0 && next_x < WIDTH && next_y >= 0 && next_y < HEIGHT)
                {
                    NEW_X = next_x;
                    NEW_Y = next_y;
                }
            }
            char tmp[80];
            sprintf(tmp, "Current Inspection Location: X: %2d, Y: %2d - Press 't' to inspect here", NEW_X, NEW_Y);
            display_message_str(tmp);
        }

        // Show the map with '*' cursor
        display_window_FOG(map, item_map, dungeon);
        chtype ch;
        ch = '*';
        mvaddch(NEW_Y + 1, NEW_X, ch);
        if (*input == 't')
        {
            while (*input != '%')
            {
                bool found = false;
                int i = 0;
                for (i = 0; i < 22; i++)
                {
                    if (map->monsters[i] &&
                        map->monsters[i]->location_x == NEW_X &&
                        map->monsters[i]->location_y == NEW_Y)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    display_message_str("No monster found at this location.");
                }
                else
                {
                    display_message_str("Press any key to exit description");
                    std::string desc = map->monsters[i]->DESC;
                    std::vector<std::string> lines = wrap_text(desc, 75); // leave space for padding
                    char list[10][77];

                    for (int j = 0; j < 10; j++)
                    {
                        if (j < (int)lines.size())
                            snprintf(list[j], sizeof(list[j]), " %-*s", 75, lines[j].c_str());
                        else
                            snprintf(list[j], sizeof(list[j]), " ");
                    }

                    mvprintw(3, 5, "Monster Description: Press any key to continue");

                    // Display formatted lines
                    for (int j = 0; j < 10; j++)
                    {
                        mvprintw(4 + j, 1, "%s", list[j]);
                    }

                    refresh();
                    *input = pc_input(input); // Wait for user to press a key to continue
                }
            }
        }
    }
}
