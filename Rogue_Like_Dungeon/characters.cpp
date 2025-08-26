#include "characters.h"
#include "dungeon_generation.h"
#include "priority_queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <ncurses.h>
#include "display.h"
#include <unistd.h>
#include "description_parsing.h"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include "items.h"
#include <map>

#define MAX_MONSTERS 20
#define WIDTH 80
#define HEIGHT 21
#define ATTRIBUTE_INTELLIGENCE 0x00000001
#define ATTRIBUTE_TELEPATHIC 0x00000002
#define ATTRIBUTE_TUNNELING 0x00000004
#define ATTRIBUTE_ERATIC 0x00000008
#define esc 27

int init_character_map(Dungeon *dungeon, Character_Map *map, int number_of_monsters)
{
    // initialize empty map
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            map->grid[y][x] = ' ';
        }
    }

    // Add PC to character list

    map->pc = new PC(dungeon, map);
    map->grid[map->pc->location_y][map->pc->location_x] = map->pc->get_Character();

    // Add Monsters to Character List
    for (int i = 0; i < MAX_MONSTERS; i++)
    {
        if (i < number_of_monsters)
        {
            map->monsters[i] = new Monster(map, &(dungeon->rooms[rand() % dungeon->room_count]));
        }
        else
        {
            map->monsters[i] = nullptr; // Ensure unallocated slots are set to null
        }
    }
    return 0;
}

int load_monster_parsed(std::vector<monster_parse> &monsters)
{
    const int max_attempts = 5000; // safety limit
    int attempts = 0;

    while (attempts++ < max_attempts)
    {
        usleep(1000);
        int index = rand() % monsters.size(); // Step 1

        if (!monsters[index].is_valid()) // Step 2
            continue;

        if ((rand() % 100) >= monsters[index].RRTY) // Step 3
            continue;

        // Step 4 - Check Uniqueness
        if (std::find(monsters[index].ABIL.begin(), monsters[index].ABIL.end(), "UNIQ") != monsters[index].ABIL.end())
        {
            monsters[index].has_abil = false;
        }

        return index; // Step 5
    }

    // If we reach here, something went wrong
    std::cerr << "Fatal Error: Unable to load a valid monster after " << max_attempts << " attempts." << std::endl;
    std::exit(EXIT_FAILURE);
}

int init_character_map_parsed(Dungeon *dungeon, Character_Map *map, std::vector<monster_parse> &parsed_monsters, int number_of_monsters)
{
    // Initialize empty map
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            map->grid[y][x] = ' ';
        }
    }

    // Add PC to character list
    map->pc = new PC(dungeon, map);
    map->grid[map->pc->location_y][map->pc->location_x] = map->pc->get_Character();

    // Add parsed monsters to map
    for (int i = 0; i < MAX_MONSTERS; i++)
    {
        if (i < number_of_monsters)
        {
            Room *spawn_room = &(dungeon->rooms[rand() % dungeon->room_count]);
            int monster_index = load_monster_parsed(parsed_monsters);
            map->monsters[i] = new Monster(parsed_monsters[monster_index], spawn_room, map);
        }
        else
        {
            map->monsters[i] = nullptr;
        }
    }

    return 0;
}

Monster::Monster(const monster_parse &mp, Room *room, Character_Map *map)
{
    // Positioning
    location_x = room->x + (rand() % room->width);
    location_y = room->y + (rand() % room->height);

    // Prevent spawning on the PC
    if (location_x == map->pc->location_x && location_y == map->pc->location_y)
    {
        location_x = room->x + (rand() % room->width);
        location_y = room->y + (rand() % room->height);
    }

    // Data from parse
    NAME = mp.Name;
    character = mp.SYMB;
    COLOR = prefix_color_tokens(mp.COLOR);
    DESC = mp.DESC;
    DAM = mp.DAM;
    HP = parse_dice(mp.HP);
    speed = parse_dice(mp.SPEED);
    characteristics = parse_abilities(mp.ABIL);

    // Other
    last_seen_pc_x = -1;
    last_seen_pc_y = -1;
    alive = 1;

    map->grid[location_y][location_x] = character;

    // std::cout << "NAME: " << NAME << std::endl;
    // std::cout << "SYMB: " << character << std::endl;
    // std::cout << "DAM: " << DAM << std::endl;
    // std::cout << "HP: " << HP << std::endl;
    // std::cout << "SPEED: " << speed << std::endl;
    // std::cout << "Abilities: " << characteristics << std::endl;
    // std::cout << "Color: " << COLOR << std::endl << std::endl  << std::endl;
}

int init_monster(Character_Map *map, Monster *monster, Room *room)
{
    monster->location_x = room->x + (rand() % room->width);
    monster->location_y = room->y + (rand() % room->height);
    //
    monster->COLOR = "COLOR_WHITE";

    monster->DESC = "Unknown Origin";
    monster->HP = rand() % 1000;
    monster->DAM = "0+2d40";
    //
    if (monster->location_x == map->pc->location_x && monster->location_y == map->pc->location_y)
    {
        monster->location_x = room->x + (rand() % room->width);
        monster->location_y = room->y + (rand() % room->height);
    }
    int tmp = 0x00000000;
    // Intelligence
    if (rand() % 2 == 0)
    {
        tmp = tmp | ATTRIBUTE_INTELLIGENCE;
    }
    // Telepathy
    if (rand() % 2 == 0)
    {
        tmp = tmp | ATTRIBUTE_TELEPATHIC;
    }
    // Tunneling Ability
    if (rand() % 2 == 0)
    {
        tmp = tmp | ATTRIBUTE_TUNNELING;
    }
    // Erratic Behavior
    if (rand() % 2 == 0)
    {
        tmp = tmp | ATTRIBUTE_ERATIC;
    }
    monster->set_Characteristics(tmp);
    if (assign_monster_char(monster))
    {
        return -1;
    }
    monster->NAME = monster->get_Character();
    monster->set_Speed(5 + (rand() % 16));
    monster->last_seen_pc_x = -1;
    monster->last_seen_pc_y = -1;
    monster->alive = 1;
    map->grid[monster->location_y][monster->location_x] = monster->get_Character();

    // std::cout << "NAME: " << monster->NAME << std::endl;
    // std::cout << "SYMB: " << monster->get_Character() << std::endl;
    // std::cout << "DAM: " << monster->DAM << std::endl;
    // std::cout << "HP: " << monster->HP << std::endl;
    // std::cout << "SPEED: " << monster->get_Speed() << std::endl;
    // std::cout << "Abilities: " << monster->get_Characteristics() << std::endl << std::endl;
    // std::cout << "Color: " << monster->COLOR << std::endl << std::endl;

    return 0;
}

int assign_monster_char(Monster *monster)
{
    if (monster->get_Characteristics() < 10)
    {
        monster->set_Character(48 + monster->get_Characteristics());
    }
    else
    {
        monster->set_Character(97 + (monster->get_Characteristics() - 10));
    }
    return 0;
}

int init_movement_queue(PriorityQueue *Queue, Character_Map *map)
{
    int i = 0;
    Queue->size = 0;
    // printf("Init Queue:\n");
    addNode(Queue, map->pc->location_x, map->pc->location_y, 0);
    while (map->monsters[i] != nullptr && i < MAX_MONSTERS)
    {
        // printf("Node %c added\n", map->monsters[i]->character);
        addNode(Queue, map->monsters[i]->location_x, map->monsters[i]->location_y, 0);
        i++;
    }
    // printQueue(Queue,map);
    return 0;
}

int add_movement_queue_pc(PriorityQueue *Queue, Character_Map *map, int current_turn)
{
    current_turn += (1000 / map->pc->get_Speed());
    // printf("Queue | Adding: ( %d, %d ) priority %d\n", character->location_x, character->location_y, current_turn);
    addNode(Queue, map->pc->location_x, map->pc->location_y, current_turn);
    return 0;
}

int add_movement_queue_monster(PriorityQueue *Queue, Monster *character, int current_turn)
{
    current_turn += (1000 / character->get_Speed());
    // printf("Queue | Adding: ( %d, %d ) priority %d\n", character->location_x, character->location_y, current_turn);
    addNode(Queue, character->location_x, character->location_y, current_turn);
    return 0;
}

void process_next_event(PriorityQueue *Queue, Dungeon *dungeon, item_map *item_map, std::vector<item_parse> &items, std::vector<monster_parse> &monsters, Character_Map *map, int distMapTunnel[HEIGHT][WIDTH], int distMapNonTunnel[HEIGHT][WIDTH], char *input, int number_of_monsters, bool *BOSS_KILL)
{
    if (Queue->size == 0)
        return; // No events to process

    if (map->pc->HP <= 0 || map->pc->alive == 0)
    {
        std::cout << "Game Over You've been killed" << std::endl;
        exit(0); // End the game with a loss
    }
    // display_message_str("Extracting event");

    queue_node next_event;
    extractNode(Queue, &next_event); // Get the next event
    int i;
    Monster *character = nullptr;

for (i = 0; i < MAX_MONSTERS && map->monsters[i] != nullptr; i++)
{
    if (map->monsters[i]->alive == 0 &&
        next_event.x_location == map->monsters[i]->location_x &&
        next_event.y_location == map->monsters[i]->location_y)
    {
        return; // Skip dead monster
    }
    else if (next_event.x_location == map->monsters[i]->location_x &&
             next_event.y_location == map->monsters[i]->location_y)
    {
        character = map->monsters[i]; // Valid monster found
        break;
    }
}

// PC turn
if (next_event.x_location == map->pc->location_x &&
    next_event.y_location == map->pc->location_y)
{
    display_message_str("Current PC's turn");
    *input = pc_input(input);
    if (((*input == '>') && (dungeon->grid[map->pc->location_y][map->pc->location_x].type == '>')) ||
        ((*input == '<') && (dungeon->grid[map->pc->location_y][map->pc->location_x].type == '<')))
    {
        pc_stair(dungeon, Queue, item_map, items, monsters, map, number_of_monsters);
        display_message_str("You take the stairs and a new dungeon appears");
    }

    map->pc->move(Queue, dungeon, item_map, map, input, number_of_monsters, BOSS_KILL);
    add_movement_queue_pc(Queue, map, next_event.priority);
}
else if (character)
{
    character->move(Queue, dungeon, map, character, distMapTunnel, distMapNonTunnel);
    add_movement_queue_monster(Queue, character, next_event.priority);
}
else
{
    std::cerr << "Warning: No valid character found at event location (" << next_event.x_location
              << ", " << next_event.y_location << ")" << std::endl;
}

}

int calculate_monster_damage(Monster *monster)
{
    return abs(parse_dice(monster->DAM));
}

int Monster::move(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Monster *monster, int distMapTunnel[HEIGHT][WIDTH], int distMapNonTunnel[HEIGHT][WIDTH])
{
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    // Update Line of Sight
    if (monster_line_of_sight(dungeon, monster, map->pc))
    {
        monster->last_seen_pc_x = map->pc->location_x;
        monster->last_seen_pc_y = map->pc->location_y;
        // printf("@ in view\n");
    }

    // If Erratic
    if (((monster->get_Characteristics() & ATTRIBUTE_ERATIC) == ATTRIBUTE_ERATIC) && ((rand() % 2) == 0))
    {
        // printf("IT MOVED ERATICLY!\n");
        int move = rand() % 8;
        // Erratic and Tunneling
        if ((monster->get_Characteristics() & ATTRIBUTE_TUNNELING) == ATTRIBUTE_TUNNELING)
        {
            // printf("IT CAN TUNNEL!!!\n");
            if (dungeon->grid[monster->location_y + dy[move]][monster->location_x + dx[move]].hardness != 0)
            {
                // printf("IT TUNNELED\n");
                if (tunnel(dungeon, monster->location_x + dx[move], monster->location_y + dy[move]))
                {
                    change_position(Queue, dungeon, map, monster, move);
                }
            }
            else
            {
                change_position(Queue, dungeon, map, monster, move);
            }
        }
        // Erratic and Non Tunneling
        else
        {
            if (dungeon->grid[monster->location_y + dy[move]][monster->location_x + dx[move]].hardness == 0)
            {
                change_position(Queue, dungeon, map, monster, move);
            }
            else
            {
                int j = 0;
                while (j < 8)
                {
                    move = (move + 1) % 8;
                    if (dungeon->grid[monster->location_y + dy[move]][monster->location_x + dx[move]].hardness == 0)
                    {
                        change_position(Queue, dungeon, map, monster, move);
                        break;
                    }
                    j++;
                }
            }
        }
        return 0;
    }

    // If Intelligent can use shortest path remebers last seen PC location
    // If Not Intelligent uses straight line

    // If Telepathic
    // If Not Telepathic
    // run --load --nummon 5
    if ((monster->characteristics & ATTRIBUTE_INTELLIGENCE) == ATTRIBUTE_INTELLIGENCE)
    {
        int min_dist = INT_MAX;
        int direction = -1;
        int i;
        {
            // INTELLIGENT and TELEPATHIC
            if ((monster->get_Characteristics() & ATTRIBUTE_TELEPATHIC) == ATTRIBUTE_TELEPATHIC)
            {
                // INTELLIGENT and TELEPATHIC and TUNNELING
                if ((monster->get_Characteristics() & ATTRIBUTE_TUNNELING) == ATTRIBUTE_TUNNELING)
                {
                    // Get min distance move
                    for (i = 0; i < 8; i++)
                    {
                        if (min_dist > distMapTunnel[monster->location_y + dy[i]][monster->location_x + dx[i]])
                        {
                            min_dist = distMapTunnel[monster->location_y + dy[i]][monster->location_x + dx[i]];
                            direction = i;
                        }
                    }
                    // Tunnel through next location
                    if (dungeon->grid[monster->location_y + dy[direction]][monster->location_x + dx[direction]].hardness != 0)
                    {
                        if (tunnel(dungeon, monster->location_x + dx[direction], monster->location_y + dy[direction]))
                        {
                            // Made corridor
                            change_position(Queue, dungeon, map, monster, direction);
                        }
                    }
                    // Move to next location
                    else
                    {
                        change_position(Queue, dungeon, map, monster, direction);
                    }
                }

                // INTELLIGENT and TELEPATHIC and NON-TUNNELING
                else
                {
                    // Get min distance move
                    for (i = 0; i < 8; i++)
                    {
                        if (min_dist >= distMapNonTunnel[monster->location_y + dy[i]][monster->location_x + dx[i]])
                        {
                            min_dist = distMapNonTunnel[monster->location_y + dy[i]][monster->location_x + dx[i]];
                            direction = i;
                        }
                    }
                    // Move to min distance
                    change_position(Queue, dungeon, map, monster, direction);
                }
            }

            // INTELLIGENT and NON-TELEPATHIC
            else
            {
                // Hasn't seen PC
                if (monster->last_seen_pc_x == -1 && monster->last_seen_pc_y == -1)
                {
                    //  Do nothing
                }
                // Has seen PC and is at last seen location
                else if (monster->location_x == monster->last_seen_pc_x && monster->location_y == monster->last_seen_pc_y)
                {
                    //  Do Nothing
                }
                // Has seen PC not at last seen location
                else
                {
                    // INTELLIGENT and NON-TELEPATHIC and TUNNELING
                    if ((monster->get_Characteristics() & ATTRIBUTE_TUNNELING) == ATTRIBUTE_TUNNELING)
                    {
                        // Min distance to current PC
                        for (i = 0; i < 8; i++)
                        {
                            if (min_dist > distMapTunnel[monster->location_y + dy[i]][monster->location_x + dx[i]])
                            {
                                min_dist = distMapTunnel[monster->location_y + dy[i]][monster->location_x + dx[i]];
                                direction = i;
                            }
                        }
                        if (dungeon->grid[monster->location_y + dy[direction]][monster->location_y + dx[direction]].hardness != 0)
                        {
                            if (tunnel(dungeon, monster->location_x + dx[direction], monster->location_y + dy[direction]))
                            {
                                change_position(Queue, dungeon, map, monster, direction);
                            }
                        }
                        else
                        {
                            change_position(Queue, dungeon, map, monster, direction);
                        }
                    }
                    // INTELLIGENT and NON-TELEPATHIC and NON-TUNNELING
                    else
                    {
                        for (i = 0; i < 8; i++)
                        {
                            if (min_dist > distMapNonTunnel[monster->location_y + dy[i]][monster->location_x + dx[i]])
                            {
                                min_dist = distMapNonTunnel[monster->location_y + dy[i]][monster->location_x + dx[i]];
                                direction = i;
                            }
                        }
                        change_position(Queue, dungeon, map, monster, direction);
                    }
                }
            }
        }
    }
    // Non-Inteligent
    else
    {
        // NON-INTELLIGENT and TELEPATHIC
        if ((monster->get_Characteristics() & ATTRIBUTE_TELEPATHIC) == ATTRIBUTE_TELEPATHIC)
        {
            //  NON-INTELLIGENT and TELEPATHIC and TUNNELING
            if ((monster->get_Characteristics() & ATTRIBUTE_TUNNELING) == ATTRIBUTE_TUNNELING)
            {
                change_position_non_intelligent_tunneling(Queue, dungeon, map, monster);
            }
            // NON-INTELLIGENT and TELEPATHIC and NON-TUNNELING
            else
            {
                change_position_non_intelligent_non_tunneling(Queue, dungeon, map, monster);
            }
        }

        // NON-INTELLIGENT and NON-TELEPATHIC
        else
        {
            // Hasn't seen PC
            if (!(monster_line_of_sight(dungeon, monster, map->pc)))
            {
                //  Do nothing
            }
            else
            {
                // NON-INTELLIGENT and NON-TELEPATHIC and (NON-TUNNELING or TUNNELING)
                change_position_non_intelligent_non_tunneling(Queue, dungeon, map, monster);
            }
        }
    }
    return 0;
}

int tunnel(Dungeon *dungeon, int x, int y)
{
    if (dungeon->grid[y][x].hardness == 1)
    {
        return 1;
    }
    if (dungeon->grid[y][x].hardness < 85 && dungeon->grid[y][x].hardness >= 1)
    {
        // printf("NOTHING LEFT TO TUNNEL\n");
        dungeon->grid[y][x].hardness = 0;
        dungeon->grid[y][x].type = '#';
        return 1;
        // print_Dungeon(dungeon);
    }
    else if (dungeon->grid[y][x].hardness == 255)
    {
    }
    else
    {
        // printf("OLD HARDNESS: %d\t", dungeon->grid[y][x].hardness);
        dungeon->grid[y][x].hardness -= 85;
        // printf("NEW HARDNESS: %d\n", dungeon->grid[y][x].hardness);
    }
    return 0;
}
void change_position(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Monster *monster, int move_to)
{
    // printf("INITIAL: ( %d, %d )\n", character->location_x, character->location_y);
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int new_X = monster->location_x + dx[move_to];
    int new_Y = monster->location_y + dy[move_to];
    // printf("NEW: ( %d, %d )\n", new_X, new_Y);

    if (map->grid[new_Y][new_X] != ' ')
    { // Another Character already in location
      // PC
        if (map->grid[new_Y][new_X] == '@')
        {
            // display_message_str("Change Position PC DAM");
            int damage = calculate_monster_damage(monster);
            if (map->pc->HP > 0)
            {
                map->pc->HP -= damage;
                //std::cout << "Damage: " << damage << " HP Remaining: " << map->pc->HP << std::endl;
                char combat_msg[80];
                sprintf(combat_msg, "%c attacked you for %d damage !", monster->get_Character(), damage);
                display_message(combat_msg);
                mvprintw(22, 0, "Message: %s", combat_msg);
            }
            else
            {
                map->pc->alive = 0;
                mvprintw(22, 0, "You have been killed! Game over.");
                std::cout << "Game Over You've been killed" << std::endl;
                exit(0); // End the game with a loss
            }
            return;
        }
        // Another monster
        int i = 0;
        while (map->monsters[i] != nullptr && i < (MAX_MONSTERS))
        {
            if (map->monsters[i]->alive == 1 && map->monsters[i]->location_x == new_X && map->monsters[i]->location_y == new_Y)
            {
                // Displace the current occupant
                // char debug_msg[80];
                // sprintf(debug_msg, "Displace 1: %c, %c", map->monsters[i]->get_Character(), monster->get_Character());
                // mvprintw(23, 40, "%s", debug_msg);

                bool displaced = false;
                for (int j = 0; j < 8; j++)
                {
                    int neighbor_X = new_X + dx[j];
                    int neighbor_Y = new_Y + dy[j];

                    // Valid move
                    if (map->grid[neighbor_Y][neighbor_X] == ' ' && dungeon->grid[neighbor_Y][neighbor_X].hardness == 0)
                    {
                        map->monsters[i]->location_x = neighbor_X;
                        map->monsters[i]->location_y = neighbor_Y;
                        map->grid[neighbor_Y][neighbor_X] = map->monsters[i]->get_Character();
                        map->grid[new_Y][new_X] = ' ';

                        // Update the queue for the displaced monster
                        // display_message_str("Change Position Queue");
                        int k = 0;
                        while (k < Queue->size)
                        {
                            if (Queue->queue[k].x_location == new_X && Queue->queue[k].y_location == new_Y)
                            {
                                Queue->queue[k].x_location = neighbor_X;
                                Queue->queue[k].y_location = neighbor_Y;
                                break;
                            }
                            k++;
                        }
                        // display_message_str("Change Position dis");
                        displaced = true;
                        break;
                    }
                }
                if (!displaced)
                {
                    // Swap positions if displacement fails
                    // display_message_str("Change Position !dis");
                    // Swap locations
                    int temp_X = monster->location_x;
                    int temp_Y = monster->location_y;
                    monster->location_x = new_X;
                    monster->location_y = new_Y;
                    map->monsters[i]->location_x = temp_X;
                    map->monsters[i]->location_y = temp_Y;
                    map->grid[temp_Y][temp_X] = map->monsters[i]->get_Character();
                    map->grid[new_Y][new_X] = monster->get_Character();

                    // display_message_str("Change Position !dis queue");
                    //  Update the queue for both monsters
                    int index_temp = -1;
                    int index_new = -1;

                    for (int k = 0; k < Queue->size; ++k)
                    {
                        if (Queue->queue[k].x_location == temp_X && Queue->queue[k].y_location == temp_Y)
                            index_temp = k;
                        else if (Queue->queue[k].x_location == new_X && Queue->queue[k].y_location == new_Y)
                            index_new = k;
                    }

                    // Perform swaps safely after lookup
                    if (index_temp != -1)
                    {
                        Queue->queue[index_temp].x_location = new_X;
                        Queue->queue[index_temp].y_location = new_Y;
                    }

                    if (index_new != -1)
                    {
                        Queue->queue[index_new].x_location = temp_X;
                        Queue->queue[index_new].y_location = temp_Y;
                    }
                }
                return;
            }
            i++;
        }
    }
    else if (new_X == 80 || new_X == 0 || new_Y == 21 || new_Y == 0)
    {
        return;
    }

    // Update Character
    int k = 0;
    while (k < Queue->size)
    {
        if (Queue->queue[k].x_location == monster->location_x && Queue->queue[k].y_location == monster->location_y)
        {
            // printf("Update Queue Cords\n");
            Queue->queue[k].x_location = new_X;
            Queue->queue[k].y_location = new_Y;
            break;
        }
        k++;
    }
    map->grid[monster->location_y][monster->location_x] = ' ';
    map->grid[new_Y][new_X] = monster->get_Character();
    monster->location_x = new_X;
    monster->location_y = new_Y;
}

void change_position_non_intelligent_tunneling(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Monster *monster)
{
    int dx = 0, dy = 0;
    int adx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int ady[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    // Move horizontally toward last known x
    if (monster->location_x < map->pc->location_x)
        dx = 1;
    else if (monster->location_x > map->pc->location_x)
        dx = -1;
    else
        dx = 0;

    // Move vertically toward last known y
    if (monster->location_y < map->pc->location_y)
        dy = 1;
    else if (monster->location_y > map->pc->location_y)
        dy = -1;
    else
        dy = 0;

    // Compute new position
    int new_X = monster->location_x + dx;
    int new_Y = monster->location_y + dy;

    // Ensure the move is valid (not into a wall and within bounds)
    if (new_X > 0 && new_X < WIDTH &&
        new_Y > 0 && new_Y < HEIGHT &&
        dungeon->grid[new_Y][new_X].hardness == 0)
    {
        if (map->grid[new_Y][new_X] != ' ')
        {
            if (map->grid[new_Y][new_X] == '@')
            {
                // display_message_str("Change Position PC DAM");
                int damage = calculate_monster_damage(monster);
                if (map->pc->HP > 0)
                {
                    map->pc->HP -= damage;
                    char combat_msg[80];
                    sprintf(combat_msg, "%c attacked you for %d damage!", monster->get_Character(), damage);
                    display_message(combat_msg);
                    mvprintw(22, 0, "Message: %s", combat_msg);
                }
                else
                {
                    map->pc->alive = 0;
                    mvprintw(22, 0, "You have been killed! Game over.");
                    std::cout << "Game Over You've been killed" << std::endl;
                    exit(0); // End the game with a loss
                }
                return;
            }
            // Another monster
            int i = 0;
            while (map->monsters[i] != nullptr && i < (MAX_MONSTERS))
            {
                if (map->monsters[i]->alive == 1 && map->monsters[i]->location_x == new_X && map->monsters[i]->location_y == new_Y)
                {
                    // Displace the current occupant
                    char debug_msg[80];
                    // sprintf(debug_msg, "Displace 1: %c, %c", map->monsters[i]->get_Character(), monster->get_Character());
                    mvprintw(23, 40, "%s", debug_msg);

                    bool displaced = false;
                    for (int j = 0; j < 8; j++)
                    {
                        int neighbor_X = new_X + adx[j];
                        int neighbor_Y = new_Y + ady[j];

                        // Valid move
                        if (map->grid[neighbor_Y][neighbor_X] == ' ' && dungeon->grid[neighbor_Y][neighbor_X].hardness == 0)
                        {
                            map->monsters[i]->location_x = neighbor_X;
                            map->monsters[i]->location_y = neighbor_Y;
                            map->grid[neighbor_Y][neighbor_X] = map->monsters[i]->get_Character();
                            map->grid[new_Y][new_X] = ' ';

                            // Update the queue for the displaced monster
                            // display_message_str("Change Position Queue");
                            int k = 0;
                            while (k < Queue->size)
                            {
                                if (Queue->queue[k].x_location == new_X && Queue->queue[k].y_location == new_Y)
                                {
                                    Queue->queue[k].x_location = neighbor_X;
                                    Queue->queue[k].y_location = neighbor_Y;
                                    break;
                                }
                                k++;
                            }
                            // display_message_str("Change Position dis");
                            displaced = true;
                            break;
                        }
                    }
                    if (!displaced)
                    {
                        // Swap positions if displacement fails
                        // display_message_str("Change Position !dis");
                        // Swap locations
                        int temp_X = monster->location_x;
                        int temp_Y = monster->location_y;
                        monster->location_x = new_X;
                        monster->location_y = new_Y;
                        map->monsters[i]->location_x = temp_X;
                        map->monsters[i]->location_y = temp_Y;
                        map->grid[temp_Y][temp_X] = map->monsters[i]->get_Character();
                        map->grid[new_Y][new_X] = monster->get_Character();

                        // display_message_str("Change Position !dis queue");
                        //  Update the queue for both monsters
                        int index_temp = -1;
                        int index_new = -1;

                        for (int k = 0; k < Queue->size; ++k)
                        {
                            if (Queue->queue[k].x_location == temp_X && Queue->queue[k].y_location == temp_Y)
                                index_temp = k;
                            else if (Queue->queue[k].x_location == new_X && Queue->queue[k].y_location == new_Y)
                                index_new = k;
                        }

                        // Perform swaps safely after lookup
                        if (index_temp != -1)
                        {
                            Queue->queue[index_temp].x_location = new_X;
                            Queue->queue[index_temp].y_location = new_Y;
                        }

                        if (index_new != -1)
                        {
                            Queue->queue[index_new].x_location = temp_X;
                            Queue->queue[index_new].y_location = temp_Y;
                        }
                    }
                    return;
                }
                i++;
            }
        }

        // Move the monster
        map->grid[monster->location_y][monster->location_x] = ' '; // Clear old position
        map->grid[new_Y][new_X] = monster->get_Character();        // Place monster in new position
        int k = 0;
        while (k < Queue->size)
        {
            if (Queue->queue[k].x_location == monster->location_x && Queue->queue[k].y_location == monster->location_y)
            {
                Queue->queue[k].x_location = new_X;
                Queue->queue[k].y_location = new_Y;
            }
            k++;
        }
        monster->location_x = new_X;
        monster->location_y = new_Y;
    }
    else if (new_X > 0 && new_X < WIDTH &&
             new_Y > 0 && new_Y < HEIGHT &&
             dungeon->grid[new_Y][new_X].hardness != 0)
    {
        if (tunnel(dungeon, new_X, new_Y))
        {
            map->grid[monster->location_y][monster->location_x] = ' ';
            map->grid[new_Y][new_X] = monster->get_Character();
            for (int j = 0; j < MAX_MONSTERS; j++)
            {
                if (map->monsters[j]->location_x == monster->location_x && map->monsters[j]->location_y == monster->location_y)
                {
                    map->monsters[j]->location_x = new_X;
                    map->monsters[j]->location_y = new_Y;
                    break;
                }
            }
            int k = 0;
            while (k < Queue->size)
            {
                if (Queue->queue[k].x_location == monster->location_x && Queue->queue[k].y_location == monster->location_y)
                {
                    Queue->queue[k].x_location = new_X;
                    Queue->queue[k].y_location = new_Y;
                    break;
                }
                k++;
            }
            monster->location_x = new_X;
            monster->location_y = new_Y;
        }
    }
}

void change_position_non_intelligent_non_tunneling(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Monster *monster)
{
    int dx = 0, dy = 0;
    int adx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int ady[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    // Move horizontally toward last known x
    if (monster->location_x < map->pc->location_x)
        dx = 1;
    else if (monster->location_x > map->pc->location_x)
        dx = -1;
    else
        dx = 0;

    // Move vertically toward last known y
    if (monster->location_y < map->pc->location_y)
        dy = 1;
    else if (monster->location_y > map->pc->location_y)
        dy = -1;
    else
        dy = 0;

    // Compute new position
    int new_X = monster->location_x + dx;
    int new_Y = monster->location_y + dy;

    // Ensure the move is valid (not into a wall and within bounds)
    if (new_X > 0 && new_X < WIDTH &&
        new_Y > 0 && new_Y < HEIGHT &&
        dungeon->grid[new_Y][new_X].hardness == 0)
    {
        if (map->grid[new_Y][new_X] != ' ')
        {
            if (map->grid[new_Y][new_X] == '@')
            {
                display_message_str("Change Position PC DAM");
                int damage = calculate_monster_damage(monster);
                if (map->pc->HP > 0)
                {
                    map->pc->HP -= damage;
                    char combat_msg[80];
                    sprintf(combat_msg, "%c attacked you for %d damage!", monster->get_Character(), damage);
                    display_message(combat_msg);
                    mvprintw(22, 0, "Message: %s", combat_msg);
                }
                else
                {
                    map->pc->alive = 0;
                    mvprintw(22, 0, "You have been killed! Game over.");
                    std::cout << "Game Over You've been killed" << std::endl;
                    exit(0); // End the game with a loss
                }
                return;
            }
            // Another monster
            int i = 0;
            while (map->monsters[i] != nullptr && i < (MAX_MONSTERS))
            {
                if (map->monsters[i]->alive == 1 && map->monsters[i]->location_x == new_X && map->monsters[i]->location_y == new_Y)
                {
                    // Displace the current occupant
                    // char debug_msg[80];
                    // sprintf(debug_msg, "Displace 1: %c, %c", map->monsters[i]->get_Character(), monster->get_Character());
                    // mvprintw(23, 40, "%s", debug_msg);

                    bool displaced = false;
                    for (int j = 0; j < 8; j++)
                    {
                        int neighbor_X = new_X + adx[j];
                        int neighbor_Y = new_Y + ady[j];

                        // Valid move
                        if (map->grid[neighbor_Y][neighbor_X] == ' ' && dungeon->grid[neighbor_Y][neighbor_X].hardness == 0)
                        {
                            map->monsters[i]->location_x = neighbor_X;
                            map->monsters[i]->location_y = neighbor_Y;
                            map->grid[neighbor_Y][neighbor_X] = map->monsters[i]->get_Character();
                            map->grid[new_Y][new_X] = ' ';

                            // Update the queue for the displaced monster
                            // display_message_str("Change Position Queue");
                            int k = 0;
                            while (k < Queue->size)
                            {
                                if (Queue->queue[k].x_location == new_X && Queue->queue[k].y_location == new_Y)
                                {
                                    Queue->queue[k].x_location = neighbor_X;
                                    Queue->queue[k].y_location = neighbor_Y;
                                    break;
                                }
                                k++;
                            }
                            // display_message_str("Change Position dis");
                            displaced = true;
                            break;
                        }
                    }
                    if (!displaced)
                    {
                        // Swap positions if displacement fails
                        // display_message_str("Change Position !dis");
                        // Swap locations
                        int temp_X = monster->location_x;
                        int temp_Y = monster->location_y;
                        monster->location_x = new_X;
                        monster->location_y = new_Y;
                        map->monsters[i]->location_x = temp_X;
                        map->monsters[i]->location_y = temp_Y;
                        map->grid[temp_Y][temp_X] = map->monsters[i]->get_Character();
                        map->grid[new_Y][new_X] = monster->get_Character();

                        // display_message_str("Change Position !dis queue");
                        //  Update the queue for both monsters
                        int index_temp = -1;
                        int index_new = -1;

                        for (int k = 0; k < Queue->size; ++k)
                        {
                            if (Queue->queue[k].x_location == temp_X && Queue->queue[k].y_location == temp_Y)
                                index_temp = k;
                            else if (Queue->queue[k].x_location == new_X && Queue->queue[k].y_location == new_Y)
                                index_new = k;
                        }

                        // Perform swaps safely after lookup
                        if (index_temp != -1)
                        {
                            Queue->queue[index_temp].x_location = new_X;
                            Queue->queue[index_temp].y_location = new_Y;
                        }

                        if (index_new != -1)
                        {
                            Queue->queue[index_new].x_location = temp_X;
                            Queue->queue[index_new].y_location = temp_Y;
                        }
                    }
                    return;
                }
                i++;
            }
        }
        // Move the monster
        map->grid[monster->location_y][monster->location_x] = ' '; // Clear old position
        map->grid[new_Y][new_X] = monster->get_Character();        // Place monster in new position

        int k = 0;
        while (k < Queue->size)
        {
            if (Queue->queue[k].x_location == monster->location_x && Queue->queue[k].y_location == monster->location_y)
            {
                Queue->queue[k].x_location = new_X;
                Queue->queue[k].y_location = new_Y;
                break;
            }
            k++;
        }
        monster->location_x = new_X;
        monster->location_y = new_Y;
    }
}

int monster_line_of_sight(Dungeon *dungeon, Monster *monster, PC *PC)
{
    // Monster see whole room for LOS
    for (int i = 0; i < dungeon->room_count; i++)
    {
        // Find room that monster is in.
        if ((dungeon->rooms[i].x < monster->location_x) && (monster->location_x < dungeon->rooms[i].x + dungeon->rooms[i].width) &&
            (dungeon->rooms[i].y < monster->location_y) && (monster->location_y < dungeon->rooms[i].y + dungeon->rooms[i].height))
        {
            // PC is in that same room
            if ((dungeon->rooms[i].x < PC->location_x) && (PC->location_x < dungeon->rooms[i].x + dungeon->rooms[i].width) &&
                (dungeon->rooms[i].y < PC->location_y) && (PC->location_y < dungeon->rooms[i].y + dungeon->rooms[i].height))
            {
                // printf("LOS: Same room as PC\n");
                return 1;
            }
            break;
        }
    }
    // Monsters have line of sight 10 spaces in all 4 major direction (left,right,up,down)
    //
    int x_dir = (monster->location_x > PC->location_x) ? 1 : -1;
    int y_dir = (monster->location_y > PC->location_y) ? 1 : -1;
    // Same X
    if (PC->location_x == monster->location_x && (abs(PC->location_y - monster->location_y) < 10))
    {
        for (int i = 0; i < abs(PC->location_y - monster->location_y); i++)
        {
            if (dungeon->grid[monster->location_y + i * y_dir][monster->location_x].hardness != 0)
            {
                return 0;
            }
        }
        return 1;
    }
    // Same Y
    else if (PC->location_y == monster->location_y && (abs(PC->location_x - monster->location_x) < 10))
    {
        for (int i = 0; i < abs(PC->location_x - monster->location_x); i++)
        {
            if (dungeon->grid[monster->location_y][monster->location_x + i * x_dir].hardness != 0)
            {
                return 0;
            }
        }
        return 1;
    }
    return 0;
}

void printQueue(PriorityQueue *pq, Character_Map *map)
{
    printf("\nCurrent Queue State: size %d\n", pq->size);
    for (int i = 0; i < pq->size; i++)
    {
        int j, is_pc = 0;
        for (j = 0; j < 21; j++)
        {
            if (map->monsters[j]->alive == 1 && (map->monsters[j]->location_x == pq->queue[i].x_location && map->monsters[j]->location_y == pq->queue[i].y_location))
            {
                break;
            }
            else if (map->pc->alive == 1 && (map->pc->location_x == pq->queue[i].x_location && map->pc->location_y == pq->queue[i].y_location))
            {
                is_pc = 1;
                break;
            }
        }
        if (is_pc)
        {
            printf("%d | Char %c: X=%d, Y=%d, Priority=%d\n", i,
                   map->pc->get_Character(), pq->queue[i].x_location, pq->queue[i].y_location, pq->queue[i].priority);
            is_pc = 0;
        }
        else
        {
            printf("%d | Char %c: X=%d, Y=%d, Priority=%d\n", i,
                   map->monsters[j]->get_Character(), pq->queue[i].x_location, pq->queue[i].y_location, pq->queue[i].priority);
        }
    }
}

void printCharacterMap(Character_Map *map)
{
    printf("\nCurrent Character State:\n");
    for (int i = 0; i < MAX_MONSTERS && map->monsters[i] != nullptr; i++)
    {
        if (map->monsters[i]->location_x != 0 || map->monsters[i]->location_y != 0)
        {
            printf("%d | Character %c: X = %d, Y = %d, Alive = %d\n", i,
                   map->monsters[i]->get_Character(), map->monsters[i]->location_x, map->monsters[i]->location_y, map->monsters[i]->alive);
        }
    }
}

char pc_input(char *prev_input)
{
    int key = getch();
    *prev_input = key;

    switch (key)
    {
    case '7':
        return '7';
    case 'y':
        return 'y';
    case '8':
        return '8';
    case 'k':
        return 'k';
    case '9':
        return '9';
    case 'u':
        return 'u';
    case '6':
        return '6';
    case 'l':
        return 'l';
    case '3':
        return '3';
    case 'n':
        return 'n';
    case '2':
        return '2';
    case 'j':
        return 'j';
    case '1':
        return '1';
    case 'b':
        return 'b';
    case '4':
        return '4';
    case 'h':
        return 'h';
    case '>':
        return '>';
    case '<':
        return '<';
    case '5':
        return '5';
    case ' ':
        return ' ';
    case '.':
        return '.';
    case 'm':
        return 'm';
    case 'a':
        return 'a';
    case 'c':
        return 'c';
    case 'd':
        return 'd';
    case 'e':
        return 'e';
    case 'f':
        return 'f';
    case 'g':
        return 'g';
    case 'i':
        return 'i';
    case KEY_UP:
        return '^';
    case KEY_DOWN:
        return 'v';
    case 27:
        return '%'; // Escape key
    case 'Q':
        return 'Q';
    case 'T':
        return 'T';
    case 't':
        return 't';
    case 'H':
        return 'H';
    case 'D':
        return 'D';
    case 'C':
        return 'C';
    case '|':
        return '|';
    case 'r':
        return 'r';
    case 'I':
        return 'I';
    case 'N':
        return 'N';
    case 'L':
        return 'L';
    case 'w':
        return 'w';
    case 'x':
        return 'x';
    }
    return '-'; // Default return
}

int calculate_pc_damage(PC *pc)
{
    int damage = 0;

    // Bare-handed damage if no weapon (Weapon, Offhand, Ranged) is equipped
    if (pc->inventory.items_list[0] == nullptr && pc->inventory.items_list[1] == nullptr &&
        pc->inventory.items_list[2] == nullptr)
    {
        damage += parse_dice(pc->DAM); // Example bare-handed damage
    }

    // Add damage from equipped weapon
    if (pc->inventory.items_list[0] != nullptr || pc->inventory.items_list[1] != nullptr ||
        pc->inventory.items_list[2] != nullptr)
    {

        // Add damage from equipped items
        for (int i = 0; i < 12; i++)
        {
            if (pc->inventory.items_list[i] != nullptr)
            {
                damage += parse_dice(pc->inventory.items_list[i]->DAM);
            }
        }
    }

    return abs(damage);
}

/*
7 8 9
4 5 6
1 2 3
*/
int PC::move(PriorityQueue *Queue, Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input, int number_of_monsters, bool *BOSS_KILL)
{
    if(map->pc->alive == 0){
        return 0;
    }
    char tmp;
    if (*input == '7' || *input == 'y')
    {
        tmp = '7';
    }
    else if (*input == '8' || *input == 'k')
    {
        tmp = '8';
    }
    else if (*input == '9' || *input == 'u')
    {
        tmp = '9';
    }
    else if (*input == '6' || *input == 'l')
    {
        tmp = '6';
    }
    else if (*input == '3' || *input == 'n')
    {
        tmp = '3';
    }
    else if (*input == '2' || *input == 'j')
    {
        tmp = '2';
    }
    else if (*input == '1' || *input == 'b')
    {
        tmp = '1';
    }
    else if (*input == '4' || *input == 'h')
    {
        tmp = '4';
    }
    else if (*input == '5' || *input == ' ' || *input == '.')
    {
        tmp = '5';
        display_message_str("Character is now resting");
        return 0;
    }
    else
    {
        display_message_str("Invalid Input");
        return 0; // Invalid input, do nothing
    }

    int move_to = tmp - '1';

    int dx[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
    int dy[] = {1, 1, 1, 0, 0, 0, -1, -1, -1};
    int new_X = map->pc->location_x + dx[move_to];
    int new_Y = map->pc->location_y + dy[move_to];
    char str_tmp[25];
    sprintf(str_tmp, "Moved to X: %d, Y: %d", new_X, new_Y);
    display_message(str_tmp);

    // Prevent PC from moving to non floor type
    if (dungeon->grid[new_Y][new_X].hardness != 0)
    {
        display_message_str("A wall is in the way try a new position next turn");
        return 0;
    }

    if (map->grid[new_Y][new_X] != ' ')
    {
        int i = 0;
        while (map->monsters[i] != nullptr && i < MAX_MONSTERS)
        {
            if (map->monsters[i]->location_x == new_X && map->monsters[i]->location_y == new_Y)
            {
                // Combat logic
                int damage = calculate_pc_damage(map->pc);
                map->monsters[i]->HP -= damage;
                char combat_msg[80];
                sprintf(combat_msg, "You attacked %c for %d damage!", map->monsters[i]->get_Character(), damage);
                display_message(combat_msg);
                mvprintw(22, 40, "Message: %s", combat_msg);

                if (map->monsters[i]->HP <= 0 || map->monsters[i]->alive == 0)
                {
                    map->monsters[i]->alive = 0;
                    map->grid[new_Y][new_X] = ' ';
                    removeNode(Queue, new_X, new_Y);
                    sprintf(combat_msg, "You killed %c!", map->monsters[i]->get_Character());
                    display_message(combat_msg);

                    // Check for BOSS flag
                    if ((map->monsters[i]->get_Characteristics() & 0x16) == 0x16)
                    {
                        display_message_str("You defeated a BOSS! You win!");
                        *BOSS_KILL = true;
                    }
                }

                return 0; // Attack uses the turn, no movement
            }
            i++;
        }
    }
    else if (new_X == 80 || new_X == 0 || new_Y == 21 || new_Y == 0)
    {
        display_message_str("Trying to move to invalid location try a new position next turn");
        return -1;
    }

    // Update Character
    int k = 0;
    while (k < Queue->size)
    {
        if (Queue->queue[k].x_location == map->pc->location_x && Queue->queue[k].y_location == map->pc->location_y)
        {
            // printf("Update Queue Cords\n");
            Queue->queue[k].x_location = new_X;
            Queue->queue[k].y_location = new_Y;
            break;
        }
        k++;
    }
    map->grid[map->pc->location_y][map->pc->location_x] = ' ';
    map->grid[new_Y][new_X] = map->pc->get_Character();
    map->pc->location_x = new_X;
    map->pc->location_y = new_Y;
    dungeon->player_x = new_X;
    dungeon->player_y = new_Y;

    // Automatic Pickup item
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        if (item_map->items_list[i] == nullptr)
        {
            continue;
        }
        if (item_map->coordinates[i].y == new_Y && item_map->coordinates[i].x == new_X)
        {
            pc_pickup(map, item_map->items_list[i]);
            item_map->is_picked_up[i] = true;
        }
    }
    // Visibility Map
    int ddx[] = {-2, -1, 0, 1, 2};
    int ddy[] = {-2, -1, 0, 1, 2};
    for (int i = 0; i < 25; i++)
    {
        // Corners
        if ((i == 0) && dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
        {
        }
        else if ((i == 4) && dungeon->grid[new_Y - 1][new_X + 1].hardness != 0)
        {
        }
        else if ((i == 20) && dungeon->grid[new_Y + 1][new_X - 1].hardness != 0)
        {
        }
        else if ((i == 24) && dungeon->grid[new_Y + 1][new_X + 1].hardness != 0)
        {
        }
        // Top and sides
        else if ((i == 2) && dungeon->grid[new_Y - 1][new_X].hardness != 0)
        {
        }
        else if ((i == 14) && dungeon->grid[new_Y][new_X + 1].hardness != 0)
        {
        }
        else if ((i == 10) && dungeon->grid[new_Y][new_X - 1].hardness != 0)
        {
        }
        else if ((i == 22) && dungeon->grid[new_Y + 1][new_X].hardness != 0)
        {
        }
        else if ((i == 3) && dungeon->grid[new_Y - 1][new_X].hardness != 0 &&
                 dungeon->grid[new_Y - 1][new_X + 1].hardness != 0)
        {
        }
        else if ((i == 1) && dungeon->grid[new_Y - 1][new_X].hardness != 0 &&
                 dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
        {
        }
        else if ((i == 5) && dungeon->grid[new_Y][new_X - 1].hardness != 0 &&
                 dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
        {
        }
        else if ((i == 9) && dungeon->grid[new_Y - 1][new_X - 1].hardness != 0 &&
                 dungeon->grid[new_Y][new_X - 1].hardness != 0)
        {
        }
        else if ((i == 15) && dungeon->grid[new_Y + 1][new_X - 1].hardness != 0 &&
                 dungeon->grid[new_Y][new_X - 1].hardness != 0)
        {
        }
        else if ((i == 21) && dungeon->grid[new_Y + 1][new_X].hardness != 0 &&
                 dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
        {
        }
        else if ((i == 19) && dungeon->grid[new_Y + 1][new_X + 1].hardness != 0 &&
                 dungeon->grid[new_Y][new_X - 1].hardness != 0)
        {
        }
        else if ((i == 23) && dungeon->grid[new_Y + 1][new_X - 1].hardness != 0 &&
                 dungeon->grid[new_Y + 1][new_X].hardness != 0)
        {
        }
        else
        {
            map->pc->discoverd[new_Y + ddy[i / 5]][new_X + ddx[i % 5]] = dungeon->grid[new_Y + ddy[i / 5]][new_X + ddx[i % 5]].type;
        }
    }

    return 0;
}

int PC::move()
{
    return 0;
}

int Monster::move()
{
    return 0;
}

void pc_stair(Dungeon *dungeon, PriorityQueue *Queue, item_map *item_map, std::vector<item_parse> &items, std::vector<monster_parse> &monsters, Character_Map *map, int number_of_monsters)
{
    // Preserved data
    int tmp_PC_HP = map->pc->HP;
    pc_inventory tmp_inventory = map->pc->inventory;

    for (int i = 0; i < 22; i++)
    {
        if (map->pc->inventory.items_list[i] != nullptr)
        {
            tmp_inventory.items_list[i] = new item(*map->pc->inventory.items_list[i]); // Deep copy
        }
        else
        {
            tmp_inventory.items_list[i] = nullptr;
        }
    }
    clean_character_map(map);
    clean_item_map(item_map);

    init_Dungeon(dungeon);
    init_Rooms(dungeon);

    if (monsters.empty())
    {
        init_character_map(dungeon, map, number_of_monsters);
    }
    else
    {
        init_character_map_parsed(dungeon, map, monsters, number_of_monsters);
    }
    init_item_map(dungeon, item_map, items);
    init_movement_queue(Queue, map);

    // Apply preserved data
    map->pc->HP = tmp_PC_HP;
    for (int i = 0; i < 22; i++)
    {
        if (tmp_inventory.items_list[i] != nullptr)
        {
            map->pc->inventory.items_list[i] = tmp_inventory.items_list[i];
        }
    }
}

void teleport(Dungeon *dungeon, PriorityQueue *Queue, item_map *item_map, Character_Map *map, char *input)
{
    display_message_str("Entered teleportion mode");
    int new_X = map->pc->location_x;
    int new_Y = map->pc->location_y;
    *input = ' ';
    while (1)
    {
        display_teleport(map, dungeon, new_X, new_Y);
        *input = pc_input(input);

        if (*input == 'e')
        {
            display_window_FOG(map, item_map, dungeon);
            break;
        }

        if (*input == 'g')
        {
            // teleport to location
            if (map->grid[new_Y][new_X] != ' ')
            { // Another Character already in location
                // Kill character in location
                int i = 0;
                while (map->monsters[i] != nullptr && i < (MAX_MONSTERS))
                {
                    if (map->monsters[i]->location_x == new_X && map->monsters[i]->location_y == new_Y)
                    {
                        map->monsters[i]->alive = 0;
                        map->grid[new_Y][new_X] = ' ';
                        removeNode(Queue, new_X, new_Y);
                        break;
                    }
                    i++;
                }
            }
            int k = 0;
            while (k < Queue->size)
            {
                if (Queue->queue[k].x_location == map->pc->location_x && Queue->queue[k].y_location == map->pc->location_y)
                {
                    Queue->queue[k].x_location = new_X;
                    Queue->queue[k].y_location = new_Y;
                    break;
                }
                k++;
            }
            int ddx[] = {-2, -1, 0, 1, 2};
            int ddy[] = {-2, -1, 0, 1, 2};
            for (int i = 0; i < 25; i++)
            {
                // Corners
                if ((i == 0) && dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 4) && dungeon->grid[new_Y - 1][new_X + 1].hardness != 0)
                {
                }
                else if ((i == 20) && dungeon->grid[new_Y + 1][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 24) && dungeon->grid[new_Y + 1][new_X + 1].hardness != 0)
                {
                }
                // Top and sides
                else if ((i == 2) && dungeon->grid[new_Y - 1][new_X].hardness != 0)
                {
                }
                else if ((i == 14) && dungeon->grid[new_Y][new_X + 1].hardness != 0)
                {
                }
                else if ((i == 10) && dungeon->grid[new_Y][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 22) && dungeon->grid[new_Y + 1][new_X].hardness != 0)
                {
                }
                else if ((i == 3) && dungeon->grid[new_Y - 1][new_X].hardness != 0 &&
                         dungeon->grid[new_Y - 1][new_X + 1].hardness != 0)
                {
                }
                else if ((i == 1) && dungeon->grid[new_Y - 1][new_X].hardness != 0 &&
                         dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 5) && dungeon->grid[new_Y][new_X - 1].hardness != 0 &&
                         dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 9) && dungeon->grid[new_Y - 1][new_X - 1].hardness != 0 &&
                         dungeon->grid[new_Y][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 15) && dungeon->grid[new_Y + 1][new_X - 1].hardness != 0 &&
                         dungeon->grid[new_Y][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 21) && dungeon->grid[new_Y + 1][new_X].hardness != 0 &&
                         dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 19) && dungeon->grid[new_Y + 1][new_X + 1].hardness != 0 &&
                         dungeon->grid[new_Y][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 23) && dungeon->grid[new_Y + 1][new_X - 1].hardness != 0 &&
                         dungeon->grid[new_Y + 1][new_X].hardness != 0)
                {
                }
                else
                {
                    map->pc->discoverd[new_Y + ddy[i / 5]][new_X + ddx[i % 5]] = dungeon->grid[new_Y + ddy[i / 5]][new_X + ddx[i % 5]].type;
                }
            }
            map->grid[map->pc->location_y][map->pc->location_x] = ' ';
            map->grid[new_Y][new_X] = map->pc->get_Character();
            map->pc->location_x = new_X;
            map->pc->location_y = new_Y;
            dungeon->player_x = new_X;
            dungeon->player_y = new_Y;
            char tmp[25];
            sprintf(tmp, "Teleported to X: %d, Y: %d", new_X, new_Y);
            display_message(tmp);
            break;
        }
        else if (*input == 'r')
        {
            // teleport random
            new_Y = 1 + (rand() % (HEIGHT - 2));
            new_X = 1 + (rand() % (WIDTH - 2));

            if (map->grid[new_Y][new_X] != ' ')
            { // Another Character already in location
                // Kill character in location
                int i = 0;
                while (map->monsters[i] != nullptr && i < (MAX_MONSTERS))
                {
                    if (map->monsters[i]->location_x == new_X && map->monsters[i]->location_y == new_Y)
                    {
                        map->monsters[i]->alive = 0;
                        map->grid[new_Y][new_X] = ' ';
                        removeNode(Queue, new_X, new_Y);
                        break;
                    }
                    i++;
                }
            }
            int k = 0;
            while (k < Queue->size)
            {
                if (Queue->queue[k].x_location == map->pc->location_x && Queue->queue[k].y_location == map->pc->location_y)
                {
                    Queue->queue[k].x_location = new_X;
                    Queue->queue[k].y_location = new_Y;
                    break;
                }
                k++;
            }
            int ddx[] = {-2, -1, 0, 1, 2};
            int ddy[] = {-2, -1, 0, 1, 2};
            for (int i = 0; i < 25; i++)
            {
                // Corners
                if ((i == 0) && dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 4) && dungeon->grid[new_Y - 1][new_X + 1].hardness != 0)
                {
                }
                else if ((i == 20) && dungeon->grid[new_Y + 1][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 24) && dungeon->grid[new_Y + 1][new_X + 1].hardness != 0)
                {
                }
                // Top and sides
                else if ((i == 2) && dungeon->grid[new_Y - 1][new_X].hardness != 0)
                {
                }
                else if ((i == 14) && dungeon->grid[new_Y][new_X + 1].hardness != 0)
                {
                }
                else if ((i == 10) && dungeon->grid[new_Y][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 22) && dungeon->grid[new_Y + 1][new_X].hardness != 0)
                {
                }
                else if ((i == 3) && dungeon->grid[new_Y - 1][new_X].hardness != 0 &&
                         dungeon->grid[new_Y - 1][new_X + 1].hardness != 0)
                {
                }
                else if ((i == 1) && dungeon->grid[new_Y - 1][new_X].hardness != 0 &&
                         dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 5) && dungeon->grid[new_Y][new_X - 1].hardness != 0 &&
                         dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 9) && dungeon->grid[new_Y - 1][new_X - 1].hardness != 0 &&
                         dungeon->grid[new_Y][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 15) && dungeon->grid[new_Y + 1][new_X - 1].hardness != 0 &&
                         dungeon->grid[new_Y][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 21) && dungeon->grid[new_Y + 1][new_X].hardness != 0 &&
                         dungeon->grid[new_Y - 1][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 19) && dungeon->grid[new_Y + 1][new_X + 1].hardness != 0 &&
                         dungeon->grid[new_Y][new_X - 1].hardness != 0)
                {
                }
                else if ((i == 23) && dungeon->grid[new_Y + 1][new_X - 1].hardness != 0 &&
                         dungeon->grid[new_Y + 1][new_X].hardness != 0)
                {
                }
                else
                {
                    map->pc->discoverd[new_Y + ddy[i / 5]][new_X + ddx[i % 5]] = dungeon->grid[new_Y + ddy[i / 5]][new_X + ddx[i % 5]].type;
                }
            }
            map->grid[map->pc->location_y][map->pc->location_x] = ' ';
            map->grid[new_Y][new_X] = map->pc->get_Character();
            map->pc->location_x = new_X;
            map->pc->location_y = new_Y;
            dungeon->player_x = new_X;
            dungeon->player_y = new_Y;
            display_message_str("Teleported to random location");
            break;
        }

        char temp;
        if (*input == '7' || *input == 'y')
        {
            temp = '7';
        }
        else if (*input == '8' || *input == 'k')
        {
            temp = '8';
        }
        else if (*input == '9' || *input == 'u')
        {
            temp = '9';
        }
        else if (*input == '6' || *input == 'l')
        {
            temp = '6';
        }
        else if (*input == '3' || *input == 'n')
        {
            temp = '3';
        }
        else if (*input == '2' || *input == 'j')
        {
            temp = '2';
        }
        else if (*input == '1' || *input == 'b')
        {
            temp = '1';
        }
        else if (*input == '4' || *input == 'h')
        {
            temp = '4';
        }
        else if (*input == '5' || *input == ' ' || *input == '.')
        {
            temp = '5';
        }

        int move_to = temp - '1';

        int dx[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
        int dy[] = {1, 1, 1, 0, 0, 0, -1, -1, -1};

        if (temp == '5')
        {
            // No move
        }
        else if (temp > '1' && temp < '9')
        {
            new_X = new_X + dx[move_to];
            new_Y = new_Y + dy[move_to];
            char tmp[80];
            sprintf(tmp, "Current Teleport Location X: %2d, Y: %2d press 'g' to move here", new_X, new_Y);
            display_message(tmp);
        }
        else
        {
            display_message_str("Invalid Input");
        }
    }
    *input = ' ';
}

void clean_character_map(Character_Map *map)
{
    if (!map)
        return;

    // Clean up monsters
    for (int i = 0; i < MAX_MONSTERS; i++)
    {
        if (map->monsters[i])
        {
            delete map->monsters[i];
            map->monsters[i] = nullptr;
        }
    }

    // Clean up PC
    if (map->pc)
    {
        delete map->pc;
        map->pc = nullptr;
    }
}

std::map<std::string, int> item_look_up = {
    {"WEAPON", 0},
    {"OFFHAND", 1},
    {"RANGED", 2},
    {"ARMOR", 3},
    {"HELMET", 4},
    {"CLOAK", 5},
    {"GLOVES", 6},
    {"BOOTS", 7},
    {"AMULET", 8},
    {"LIGHT", 9},
    {"RING", 10}};

// Slots: WEAPON, OFFHAND, RANGED, ARMOR, HELMET, CLOAK, GLOVES, BOOTS, AMULET, LIGHT, and two for RING
void pc_pickup(Character_Map *map, item *item)
{
    auto find_item = item_look_up.find(item->TYPE);
    int item_type = -1;

    if (find_item != item_look_up.end())
    {
        item_type = find_item->second;
    }

    int equip_slot = -1;

    if (item_type == item_look_up["RING"])
    {
        // RING logic: try left then right
        if (!map->pc->inventory.items_list[10])
        {
            equip_slot = 10;
        }
        else if (!map->pc->inventory.items_list[11])
        {
            equip_slot = 11;
        }
    }
    else if (item_type >= 0 && item_type < 10)
    {
        // Other equipment types directly map to their slot
        if (!map->pc->inventory.items_list[item_type])
        {
            equip_slot = item_type;
        }
    }

    if (equip_slot != -1)
    {
        map->pc->inventory.items_list[equip_slot] = item;
        return;
    }

    // If item can't be equipped (slot full), try carry slots (12–21)
    for (int i = 12; i < 22; ++i)
    {
        if (!map->pc->inventory.items_list[i])
        {
            map->pc->inventory.items_list[i] = item;
            return;
        }
    }

    // Inventory full
    display_message_str("Inventory Full — Can't pick up item");
}

/*
w Wear an item. Prompts the user for a carry slot. If an item of that type is already
equipped, items are swapped.
t Take off an item. Prompts for equipment slot. Item goes to an open carry slot.
d Drop an item. Prompts user for carry slot. Item goes to floor.
x Expunge an item from the game. Prompts the user for a carry slot. Item is permanently removed from the game.
*/

void wear(Character_Map *map, char *input)
{
    display_message_str("Enter Carry Slot to equip");
    *input = ' ';

    while (*input != '%')
    {
        pc_input(input);
        if (*input < '%'){
            break;
        }

        if (*input < '0' || *input > '9')
        {
            display_message_str("Invalid slot number. try again or 'esc' to exit");
            continue;
        }

        int index = 12 + *input - '0';
        item *wear = map->pc->inventory.items_list[index];

        if (!wear)
        {
            display_message_str("Slot empty. Nothing to equip. try again or 'esc' to exit");
            continue;
        }

        // Check if the item type is valid
        auto find_item = item_look_up.find(wear->TYPE);
        if (find_item == item_look_up.end())
        {
            display_message_str("Unknown item type. Can't equip.");
            continue;
        }

        int type = find_item->second;

        // Find item in inventory (carry slots 12-21)
        int item_index = -1;
        for (int i = 12; i < 22; ++i)
        {
            if (map->pc->inventory.items_list[i] == wear)
            {
                item_index = i;
                break;
            }
        }

        if (item_index == -1)
        {
            display_message_str("Item not in inventory. Can't equip. try again or 'esc' to exit");
            continue;
        }

        item *swap_item = new item(); // Now the compiler knows what 'item' is

        // Handle rings (slots 10-11)
        if (wear->TYPE == "RING")
        {
            if (!map->pc->inventory.items_list[10])
            {
                // First ring slot empty
                map->pc->inventory.items_list[10] = wear;
                map->pc->inventory.items_list[item_index] = nullptr;
            }
            else if (!map->pc->inventory.items_list[11])
            {
                // Second ring slot empty
                map->pc->inventory.items_list[11] = wear;
                map->pc->inventory.items_list[item_index] = nullptr;
            }
            else
            {
                // Both ring slots full - swap with first slot
                swap_item = map->pc->inventory.items_list[10];
                map->pc->inventory.items_list[10] = wear;
                map->pc->inventory.items_list[item_index] = nullptr;

                // Find empty inventory slot
                bool placed = false;
                for (int i = 12; i < 22; ++i)
                {
                    if (!map->pc->inventory.items_list[i])
                    {
                        map->pc->inventory.items_list[i] = swap_item;
                        placed = true;
                        break;
                    }
                }

                if (!placed)
                {
                    display_message_str("No space to store unequipped ring. 'esc' to exit");
                    // Could add code to drop on ground here
                }
            }
            continue;
        }

        // Handle all other equipment types
        if (!map->pc->inventory.items_list[type])
        {
            // Empty equipment slot
            display_message_str("Now wearing new item");
            map->pc->inventory.items_list[type] = wear;
            map->pc->inventory.items_list[item_index] = nullptr;
        }
        else
        {
            display_message_str("Replaced item");
            // Equipment slot has an item - swap
            swap_item = map->pc->inventory.items_list[type];
            map->pc->inventory.items_list[type] = wear;
            map->pc->inventory.items_list[item_index] = nullptr;

            // Find empty inventory slot
            bool placed = false;
            for (int i = 12; i < 22; ++i)
            {
                if (!map->pc->inventory.items_list[i])
                {
                    map->pc->inventory.items_list[i] = swap_item;
                    placed = true;
                    break;
                }
            }

            if (!placed)
            {
                display_message_str("No room to unequip current item. 'esc' to exit");
            }
        }
    }
}

void take_off_item(Character_Map *map, item_map *map_item, char *input)
{
    display_message_str("Enter Equipment Slot to take off");
    *input = ' ';
    while (*input != '%')
    {
        *input = pc_input(input);
        int index = *input;

        if (index >= 'a' && index <= 'l')
        {
            index -= 'a';
        }
        else
        {
            display_message_str("Invalid equipment slot. try again or 'esc' to exit");
            continue;
        }

        if (map->pc->inventory.items_list[index] == nullptr)
        {
            display_message_str("No item in that equipment slot. try again or 'esc' to exit");
            continue;
        }

        // Find first open carry slot
        int carry_index = -1;
        for (int i = 12; i < 22; i++)
        {
            if (map->pc->inventory.items_list[i] == nullptr)
            {
                carry_index = i;
                break;
            }
        }

        if (carry_index == -1)
        {
            display_message_str("No room to unequip — carry slots full. try again or 'esc' to exit");
            continue;
        }

        // Move the item directly from equipment to carry
        map->pc->inventory.items_list[carry_index] = map->pc->inventory.items_list[index];
        map->pc->inventory.items_list[index] = nullptr;

        display_message_str("Item is taken off. try again or 'esc' to exit");
    }
}

void drop_item(Dungeon *dungeon, Character_Map *map, item_map *item_map, char *input)
{
    display_message_str("Enter Carry Slot to drop");
    while (*input != '%')
    {
        pc_input(input);
        if (*input < '%'){
            break;
        }

        // Translate 0–9 input to inventory indices 12–21
        if (*input < '0' || *input > '9')
        {
            display_message_str("Invalid slot number. try again or 'esc' to exit");
            return;
        }

        int index = 12 + *input - '0';
        if (map->pc->inventory.items_list[index])
        {
            //find index in item_map
            int item_map_index = -1;
            for(int j=0;j<20;j++){
                if(item_map->items_list[j]->Name == map->pc->inventory.items_list[index]->Name && item_map->is_picked_up[j] == true){
                    item_map_index = j;
                    break;
                }
            }

            map->pc->inventory.items_list[index] = nullptr;
            // Offsets for the 8 neighboring directions: N, NE, E, SE, S, SW, W, NW
            int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
            int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

            int x = map->pc->location_x;
            int y = map->pc->location_y;

            // Try placing item in one of the 8 neighboring floor tiles
            for (int k = 0; k < 8; k++)
            {
                int nx = x + dx[k];
                int ny = y + dy[k];

                // Ensure within bounds
                if (nx >= 0 && nx < 80 && ny >= 0 && ny < 21)
                {
                    // Check if tile is a floor
                    if (dungeon->grid[ny][nx].hardness == 0)
                    {
                        item_map->coordinates[item_map_index].x = nx;
                        item_map->coordinates[item_map_index].y = ny;
                        item_map->is_picked_up[item_map_index] = false;
                        display_message_str("Dropped Item to Neighboring Floor");
                        return;
                    }
                }
            }

            // Place on current floor tile
            item_map->coordinates[index].x = map->pc->location_x;
            item_map->coordinates[index].y = map->pc->location_y;
            item_map->is_picked_up[index] = false;
        }
        char tmp[80];
        sprintf(tmp, "Item slot %2d is empty can't drop try again or 'esc' to exit", index);
        display_message(tmp);
    }
}

void expunge_item(Character_Map *map, std::vector<item_parse> &items, char *input)
{
    display_message_str("Enter carry slot (0-9) to expunge item:");
    while (*input != '%')
    {
        pc_input(input);
        if (*input < '%'){
            break;
        }

        // Translate 0–9 input to inventory indices 12–21
        if (*input < '0' || *input > '9')
        {
            display_message_str("Invalid slot number. Try again or 'esc' to exit");
            return;
        }

        int index = 12 + *input - '0';

        if (map->pc->inventory.items_list[index] == nullptr)
        {
            display_message_str("No item in that slot to expunge.Try again or 'esc' to exit");
            return;
        }

        std::string target_name = map->pc->inventory.items_list[index]->Name;

        auto it = std::find_if(items.begin(), items.end(), [&](const item_parse &i)
                               { return i.Name == target_name; });

        // Make it an invalid item meaning it won't be placed again
        it->has_name = false;

        std::string item_name = map->pc->inventory.items_list[index]->Name;
        delete map->pc->inventory.items_list[index];
        map->pc->inventory.items_list[index] = nullptr;

        display_message_str(("Item '" + item_name + "' permanently expunged. Try again or 'esc' to exit").c_str());
    }
}