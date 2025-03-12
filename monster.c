#include "monster.h"
#include "dungeon_generation.h"
#include "priority_queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>

#define MAX_MONSTERS 20
#define WIDTH 80
#define HEIGHT 21
#define ATTRIBUTE_INTELLIGENCE 0x00000001
#define ATTRIBUTE_TELEPATHIC 0x00000002
#define ATTRIBUTE_TUNNELING 0x00000004
#define ATTRIBUTE_ERATIC 0x00000008

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
    for (int i = 0; i < number_of_monsters + 1; i++)
    {
        map->characters[i].alive = -1;
    }

    // Add PC to character list
    Character tmpPC;
    tmpPC.character = '@';
    tmpPC.speed = 10;
    tmpPC.location_x = dungeon->player_x;
    tmpPC.location_y = dungeon->player_y;
    tmpPC.characteristics = 0;
    tmpPC.last_seen_pc_x = 0;
    tmpPC.last_seen_pc_y = 0;
    tmpPC.alive = 1;
    map->characters[0] = tmpPC;
    map->grid[map->characters[0].location_y][map->characters[0].location_x] = '@';

    // Add Monsters to Character List
    for (int i = 1; i < number_of_monsters + 1; i++)
    {
        init_monster(map, &map->characters[i], &dungeon->rooms[rand() % dungeon->room_count]);
    }
    return 0;
}

int init_monster(Character_Map *map, Character *monster, Room *room)
{
    monster->location_x = room->x + (rand() % room->width);
    monster->location_y = room->y + (rand() % room->height);
    monster->characteristics = monster->characteristics & 0x00000000;
    // Intelligence
    if (rand() % 2 == 0)
    {
        monster->characteristics = monster->characteristics | ATTRIBUTE_INTELLIGENCE;
    }
    // Telepathy
    if (rand() % 2 == 0)
    {
        monster->characteristics = monster->characteristics | ATTRIBUTE_TELEPATHIC;
    }
    // Tunneling Ability
    if (rand() % 2 == 0)
    {
        monster->characteristics = monster->characteristics | ATTRIBUTE_TUNNELING;
    }
    // Erratic Behavior
    if (rand() % 2 == 0)
    {
        monster->characteristics = monster->characteristics | ATTRIBUTE_ERATIC;
    }
    if (assign_monster_char(monster))
    {
        return -1;
    }
    monster->speed = 5 + (rand() % 16);
    monster->last_seen_pc_x = -1;
    monster->last_seen_pc_y = -1;
    monster->alive = 1;
    map->grid[monster->location_y][monster->location_x] = monster->character;
    return 0;
}

int assign_monster_char(Character *monster)
{
    if (monster->characteristics < 10)
    {
        monster->character = 48 + monster->characteristics;
    }
    else
    {
        monster->character = 97 + (monster->characteristics - 10);
    }
    return 0;
}

int init_movement_queue(PriorityQueue *Queue, Character_Map *map)
{
    int i = 0;
    Queue->size = 0;
    //printf("Init Queue:\n");
    while (map->characters[i].alive == 1)
    {
        //printf("Node %c added\n", map->characters[i].character);
        addNode(Queue, map->characters[i].location_x, map->characters[i].location_y, 0);
        i++;
    }
    //printQueue(Queue,map);
    return 0;
}

int add_movement_queue(PriorityQueue *Queue, Character *character, int current_turn)
{
    current_turn += (1000 / character->speed);
    //printf("Queue | Adding: ( %d, %d ) priority %d\n", character->location_x, character->location_y, current_turn);
    addNode(Queue, character->location_x, character->location_y, current_turn);
    return 0;
}

void process_next_event(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, int distMapTunnel[HEIGHT][WIDTH], int distMapNonTunnel[HEIGHT][WIDTH])
{
    //printCharacterMap(map);
    //printQueue(Queue,map);
    if (Queue->size == 0)
        return; // No events to process

    queue_node next_event;
    extractNode(Queue, &next_event); // Get the next event
    //printf("Queue | Extracted : ( %d, %d ) priority %d\n", next_event.x_location, next_event.y_location, next_event.priority);
    int i;
    for (i = 0; i < MAX_MONSTERS + 1; i++)
    {
        if (map->characters[i].alive == 0 && next_event.x_location == map->characters[i].location_x && next_event.y_location == map->characters[i].location_y)
            return; // Skip if dead
        else if (next_event.x_location == map->characters[i].location_x && next_event.y_location == map->characters[i].location_y)
        {
            break;// i is the index of character 
        }
    }

    Character character = map->characters[i];
    //printf("\nIts %c's turn\n", map->characters[i].character);

    // Character moves based on its AI behavior
    if (character.character == map->characters[0].character)
    {
        // move_PC(dungeon, map, character);
        add_movement_queue(Queue, &character, next_event.priority);
    }else if(character.alive == 0){
        //don't add back to queue
    }
    else
    {
        //living monster move
        character_move(Queue, dungeon, map, &character, distMapTunnel, distMapNonTunnel);
        add_movement_queue(Queue, &character, next_event.priority);
    }

    // Schedule the next event for this character
    // int next_turn = next_event.priority + (1000 / character.speed);
    
    //printCharacterMap(map);
    //printQueue(Queue);
}

int character_move(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Character *character, int distMapTunnel[HEIGHT][WIDTH], int distMapNonTunnel[HEIGHT][WIDTH])
{
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    // Update Line of Sight
    if (monster_line_of_sight(dungeon, character, &map->characters[0]))
    {
        character->last_seen_pc_x = map->characters[0].location_x;
        character->last_seen_pc_y = map->characters[0].location_y;
        // printf("@ in view\n");
    }
    // If Erratic
    if (((character->characteristics & ATTRIBUTE_ERATIC) == ATTRIBUTE_ERATIC) && ((rand() % 2) == 0))
    {
        // printf("IT MOVED ERATICLY!\n");
        int move = rand() % 8;
        // Erratic and Tunneling
        if ((character->characteristics & ATTRIBUTE_TUNNELING) == ATTRIBUTE_TUNNELING)
        {
            // printf("IT CAN TUNNEL!!!\n");
            if (dungeon->grid[character->location_y + dy[move]][character->location_x + dx[move]].hardness != 0)
            {
                // printf("IT TUNNELED\n");
                if (tunnel(dungeon, character->location_x + dx[move], character->location_y + dy[move]))
                {
                    change_position(Queue, dungeon, map, character, move);
                }
            }
            else
            {
                change_position(Queue, dungeon, map, character, move);
            }
        }
        // Erratic and Non Tunneling
        else
        {
            if (dungeon->grid[character->location_y + dy[move]][character->location_x + dx[move]].hardness == 0)
            {
                change_position(Queue, dungeon, map, character, move);
            }
            else
            {
                int j = 0;
                while (j < 8)
                {
                    move = (move + 1) % 8;
                    if (dungeon->grid[character->location_y + dy[move]][character->location_x + dx[move]].hardness == 0)
                    {
                        change_position(Queue, dungeon, map, character, move);
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
    if ((character->characteristics & ATTRIBUTE_INTELLIGENCE) == ATTRIBUTE_INTELLIGENCE)
    {
        int min_dist = INT_MAX;
        int direction = -1;
        int i;
        {
            // INTELLIGENT and TELEPATHIC
            if ((character->characteristics & ATTRIBUTE_TELEPATHIC) == ATTRIBUTE_TELEPATHIC)
            {
                // INTELLIGENT and TELEPATHIC and TUNNELING
                if ((character->characteristics & ATTRIBUTE_TUNNELING) == ATTRIBUTE_TUNNELING)
                {
                    // printf("IT MOVED SMART AND TELEPATHIC!\n");
                    for (i = 0; i < 8; i++)
                    {
                        if (min_dist > distMapTunnel[character->location_y + dy[i]][character->location_x + dx[i]])
                        {
                            min_dist = distMapTunnel[character->location_y + dy[i]][character->location_x + dx[i]];
                            direction = i;
                        }
                    }

                    // printf("(Shortest Path is %d, %d )\n", character->location_x + dx[direction], character->location_y + dy[direction]);

                    if (dungeon->grid[character->location_y + dy[direction]][character->location_x + dx[direction]].hardness != 0)
                    {
                        // printf("TUNNELING\n");
                        if (tunnel(dungeon, character->location_x + dx[direction], character->location_y + dy[direction]))
                        {
                            // printf("MADE COORIDOR\n");
                            change_position(Queue, dungeon, map, character, direction);
                        }
                    }
                    else
                    {
                        change_position(Queue, dungeon, map, character, direction);
                    }
                }
                // INTELLIGENT and TELEPATHIC and NON-TUNNELING
                else
                {
                    // printf("IT MOVED SMART AND TELEPATHIC!\n");

                    for (i = 0; i < 8; i++)
                    {
                        if (min_dist >= distMapNonTunnel[character->location_y + dy[i]][character->location_x + dx[i]])
                        {
                            min_dist = distMapNonTunnel[character->location_y + dy[i]][character->location_x + dx[i]];
                            direction = i;
                        }
                    }
                    /*
                    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
                    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
                    */
                    // printf("(Shortest Path is %d, %d ), %d direction\n", character->location_x + dx[direction], character->location_y + dy[direction], direction);
                    change_position(Queue, dungeon, map, character, direction);
                }
            }

            // INTELLIGENT and NON-TELEPATHIC
            else
            {
                // Hasn't seen PC
                if (character->last_seen_pc_x == -1 && character->last_seen_pc_y == -1)
                {
                    // printf("IT DIDN'T MOVE IT HASN'T SEEN PC!\n");
                    //  Do nothing
                }
                // Has seen PC and is at last seen location
                else if (character->location_x == character->last_seen_pc_x && character->location_y == character->last_seen_pc_y)
                {
                    // printf("IT DIDN'T MOVE IT'S AT SEEN PC!\n");
                    //  Do Nothing
                }
                else
                {
                    // INTELLIGENT and NON-TELEPATHIC and TUNNELING
                    if ((character->characteristics & ATTRIBUTE_TUNNELING) == ATTRIBUTE_TUNNELING)
                    {

                        for (i = 0; i < 8; i++)
                        {
                            if (min_dist > distMapTunnel[character->location_y + dy[i]][character->location_x + dx[i]])
                            {
                                min_dist = distMapTunnel[character->location_y + dy[i]][character->location_x + dx[i]];
                                direction = i;
                            }
                        }
                        if (dungeon->grid[character->location_y + dy[direction]][character->location_y + dx[direction]].hardness != 0)
                        {
                            // printf("IT TUNNELED\n");
                            if (tunnel(dungeon, character->location_x + dx[direction], character->location_y + dy[direction]))
                            {
                                change_position(Queue, dungeon, map, character, direction);
                            }
                        }
                        else
                        {
                            change_position(Queue, dungeon, map, character, direction);
                        }
                    }
                    // INTELLIGENT and NON-TELEPATHIC and NON-TUNNELING
                    else
                    {
                        for (i = 0; i < 8; i++)
                        {
                            if (min_dist > distMapNonTunnel[character->location_y + dy[i]][character->location_x + dx[i]])
                            {
                                min_dist = distMapNonTunnel[character->location_y + dy[i]][character->location_x + dx[i]];
                                direction = i;
                            }
                        }
                        change_position(Queue, dungeon, map, character, direction);
                    }
                }
            }
        }
    }
    // Non-Inteligent
    else
    {

        // NON-INTELLIGENT and TELEPATHIC
        if ((character->characteristics & ATTRIBUTE_TELEPATHIC) == ATTRIBUTE_TELEPATHIC)
        {
            //6 and E
            // NON-INTELLIGENT and TELEPATHIC and TUNNELING
            if ((character->characteristics & ATTRIBUTE_TUNNELING) == ATTRIBUTE_TUNNELING)
            {
                // printf("MOVED NOT SMART TUNNELER\n");
                change_position_non_intelligent_tunneling(Queue, dungeon, map, character);
            }
            // NON-INTELLIGENT and TELEPATHIC and NON-TUNNELING
            else
            {
                // printf("MOVED NOT SMART\n");
                change_position_non_intelligent_non_tunneling(Queue, dungeon, map, character);
            }
        }

        // NON-INTELLIGENT and NON-TELEPATHIC
        else
        {
            // Hasn't seen PC
            if (!(monster_line_of_sight(dungeon, character, &map->characters[0])))
            {
                // printf("NO LINE OF SIGHT NO MOVE\n");
                //  Do nothing
            }
            else
            {
                // NON INTELLIGENT and NON-TELEPATHIC and (NON-TUNNELING or TUNNELING)
                // printf("LINE OF SIGHT MOVE\n");
                change_position_non_intelligent_non_tunneling(Queue, dungeon, map, character);
            }
        }
    }
    return 0;
}

/* TODO BOUNDS CHECK FOR IMMUNTABLE*/
int tunnel(Dungeon *dungeon, int x, int y)
{

    if (dungeon->grid[y][x].hardness < 85 && dungeon->grid[y][x].hardness > 0)
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

void change_position(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Character *character, int move_to)
{
    //printf("INITIAL: ( %d, %d )\n", character->location_x, character->location_y);
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int new_X = character->location_x + dx[move_to];
    int new_Y = character->location_y + dy[move_to];
    //printf("NEW: ( %d, %d )\n", new_X, new_Y);

    if (map->grid[new_Y][new_X] != ' ')
    { // Another Character already in location
        // Kill character in location
        if (map->grid[new_Y][new_X] != ' ')
        { // Another Character already in location
            // Kill character in location
            int i = 0;
            while (map->characters[i].alive != -1 && i < (MAX_MONSTERS))
            {
                if (map->characters[i].location_x == new_X && map->characters[i].location_y == new_Y)
                {
                    map->characters[i].alive = 0;
                    map->grid[new_Y][new_X] = ' ';
                    removeNode(Queue,map->characters[i].location_x,map->characters[i].location_y);
                    // printf("KILLED: %c\n",map->characters[i].character);
                }
                i++;
            }
        }
    }else if(new_X == 80 || new_X == 0 || new_Y == 21 || new_Y == 0){
         return;
    }

    // Update Character
    int k = 0;
    while (k < Queue->size)
    {
        if (Queue->queue[k].x_location == character->location_x && Queue->queue[k].y_location == character->location_y)
        {
            //printf("Update Queue Cords\n");
            Queue->queue[k].x_location = new_X;
            Queue->queue[k].y_location = new_Y;
            break;
        }
        k++;
    }
    map->grid[character->location_y][character->location_x] = ' ';
    map->grid[new_Y][new_X] = character->character;
    for(int j=0;j<21;j++){
        if(map->characters[j].location_x == character->location_x && map->characters[j].location_y == character->location_y){
            map->characters[j].location_x = new_X;
            map->characters[j].location_y = new_Y;
            break;
        }
    }
    character->location_x = new_X;
    character->location_y = new_Y;
}

void change_position_non_intelligent_tunneling(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Character *character)
{
    //printf("INITIAL: ( %d, %d )\n", character->location_x, character->location_y);
    Character *player = &map->characters[0]; // Assume player is at index 0
    int dx = 0, dy = 0;

    // Move horizontally toward last known x
    if (character->location_x < player->location_x)
        dx = 1;
    else if (character->location_x > player->location_x)
        dx = -1;
    else
        dx = 0;

    // Move vertically toward last known y
    if (character->location_y < player->location_y)
        dy = 1;
    else if (character->location_y > player->location_y)
        dy = -1;
    else
        dy = 0;

    // Compute new position
    int new_X = character->location_x + dx;
    int new_Y = character->location_y + dy;
    //printf("NEW: ( %d, %d )\n", new_X, new_Y);

    // Ensure the move is valid (not into a wall and within bounds)
    if (new_X > 0 && new_X < WIDTH &&
        new_Y > 0 && new_Y < HEIGHT &&
        dungeon->grid[new_Y][new_X].hardness == 0)
    {
        if (map->grid[new_Y][new_X] != ' ')
        { // Another Character already in location
            // Kill character in location
            int i = 0;
            while (map->characters[i].alive != -1 && i < (MAX_MONSTERS))
            {
                if (map->characters[i].location_x == new_X && map->characters[i].location_y == new_Y)
                {
                    map->characters[i].alive = 0;
                    map->grid[new_Y][new_X] = ' ';
                    removeNode(Queue,map->characters[i].location_x,map->characters[i].location_y);
                    // printf("KILLED: %c\n",map->characters[i].character);
                }
                i++;
            }
        }
        
        // Move the monster
        map->grid[character->location_y][character->location_x] = ' '; // Clear old position
        map->grid[new_Y][new_X] = character->character;                // Place monster in new position
        int k = 0;
        while (k < Queue->size)
        {
            if (Queue->queue[k].x_location == character->location_x && Queue->queue[k].y_location == character->location_y)
            {
                //printf("Update Queue Cords\n");
                Queue->queue[k].x_location = new_X;
                Queue->queue[k].y_location = new_Y;
            }
            k++;
        }
        for(int j=0;j<21;j++){
            if(map->characters[j].location_x == character->location_x && map->characters[j].location_y == character->location_y){
                map->characters[j].location_x = new_X;
                map->characters[j].location_y = new_Y;
                break;
            }
        }
        character->location_x = new_X;
        character->location_y = new_Y;
    }
    else if (new_X > 0 && new_X < WIDTH &&
             new_Y > 0 && new_Y < HEIGHT &&
             dungeon->grid[new_Y][new_X].hardness != 0)
    {
        if (tunnel(dungeon, new_X, new_Y))
        {
            map->grid[character->location_y][character->location_x] = ' '; // Clear old position
            map->grid[new_Y][new_X] = character->character;                // Place monster in new position
            for(int j=0;j<21;j++){
                if(map->characters[j].location_x == character->location_x && map->characters[j].location_y == character->location_y){
                    map->characters[j].location_x = new_X;
                    map->characters[j].location_y = new_Y;
                    break;
                }
            }
            int k = 0;
            while (k < Queue->size)
            {
                if (Queue->queue[k].x_location == character->location_x && Queue->queue[k].y_location == character->location_y)
                {
                    //printf("Update Queue Cords\n");
                    Queue->queue[k].x_location = new_X;
                    Queue->queue[k].y_location = new_Y;
                    break;
                }
                k++;
            }
            character->location_x = new_X;
            character->location_y = new_Y;
        }
    }
}

void change_position_non_intelligent_non_tunneling(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Character *character)
{
    //printf("INITIAL: ( %d, %d )\n", character->location_x, character->location_y);
    Character *player = &map->characters[0]; // Assume player is at index 0
    int dx = 0, dy = 0;

    // Move horizontally toward last known x
    if (character->location_x < player->location_x)
        dx = 1;
    else if (character->location_x > player->location_x)
        dx = -1;
    else
        dx = 0;

    // Move vertically toward last known y
    if (character->location_y < player->location_y)
        dy = 1;
    else if (character->location_y > player->location_y)
        dy = -1;
    else
        dy = 0;

    // Compute new position
    int new_X = character->location_x + dx;
    int new_Y = character->location_y + dy;
    //printf("NEW: ( %d, %d )\n", new_X, new_Y);

    //Ensure the move is valid (not into a wall and within bounds)
    if (new_X > 0 && new_X < WIDTH &&
        new_Y > 0 && new_Y < HEIGHT &&
        dungeon->grid[new_Y][new_X].hardness == 0)
    {
        if (map->grid[new_Y][new_X] != ' ')
        { // Another Character already in location
            // Kill character in location
            int i = 0;
            while (map->characters[i].alive != -1 && i < (MAX_MONSTERS))
            {
                if (map->characters[i].location_x == new_X && map->characters[i].location_y == new_Y)
                {
                    map->characters[i].alive = 0;
                    map->grid[new_Y][new_X] = ' ';
                    removeNode(Queue,map->characters[i].location_x,map->characters[i].location_y);
                    // printf("KILLED: %c\n",map->characters[i].character);
                }
                i++;
            }
        }
        // Move the monster
        map->grid[character->location_y][character->location_x] = ' '; // Clear old position
        map->grid[new_Y][new_X] = character->character;                // Place monster in new position
        for(int j=0;j<21;j++){
            if(map->characters[j].location_x == character->location_x && map->characters[j].location_y == character->location_y){
                map->characters[j].location_x = new_X;
                map->characters[j].location_y = new_Y;
                break;
            }
        }

        int k = 0;
        while (k < Queue->size)
        {
            if (Queue->queue[k].x_location == character->location_x && Queue->queue[k].y_location == character->location_y)
            {
                //printf("Update Queue Cords\n");
                Queue->queue[k].x_location = new_X;
                Queue->queue[k].y_location = new_Y;
                break;
            }
            k++;
        }
        character->location_x = new_X;
        character->location_y = new_Y;
    }
}

int monster_line_of_sight(Dungeon *dungeon, Character *character, Character *PC)
{
    // Monster see whole room for LOS
    for (int i = 0; i < dungeon->room_count; i++)
    {
        // Find room that monster is in.
        if ((dungeon->rooms[i].x < character->location_x) && (character->location_x < dungeon->rooms[i].x + dungeon->rooms[i].width) &&
            (dungeon->rooms[i].y < character->location_y) && (character->location_y < dungeon->rooms[i].y + dungeon->rooms[i].height))
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
    int x_dir = (character->location_x > PC->location_x) ? 1 : -1;
    int y_dir = (character->location_y > PC->location_y) ? 1 : -1;
    // Same X
    if (PC->location_x == character->location_x && (abs(PC->location_y - character->location_y) < 10))
    {
        for (int i = 0; i < abs(PC->location_y - character->location_y); i++)
        {
            if (dungeon->grid[character->location_y + i * y_dir][character->location_x].hardness != 0)
            {
                return 0;
            }
        }
        // printf("LOS: Same X as PC\n");
        return 1;
    }
    // Same Y
    else if (PC->location_y == character->location_y && (abs(PC->location_x - character->location_x) < 10))
    {
        for (int i = 0; i < abs(PC->location_x - character->location_x); i++)
        {
            if (dungeon->grid[character->location_y][character->location_x + i * x_dir].hardness != 0)
            {
                return 0;
            }
        }
        // printf("LOS: Same Y as PC\n");
        return 1;
    }
    // printf("LOS: NO LINE OF SIGHT\n");
    return 0;
}

void printQueue(PriorityQueue *pq, Character_Map *map)
{
    printf("\nCurrent Queue State: size %d\n",pq->size);
    for (int i = 0; i < pq->size; i++)
    {
        int j = 0;
        for(j=0;j<21;j++){
            if(map->characters[j].alive == 1 && (map->characters[j].location_x == pq->queue[i].x_location && map->characters[j].location_y == pq->queue[i].y_location)){
                break;
            }
        }
        printf("%d | Char %c: X=%d, Y=%d, Priority=%d\n", i,
            map->characters[j].character, pq->queue[i].x_location, pq->queue[i].y_location, pq->queue[i].priority);
    }
}

void printCharacterMap(Character_Map *map)
{
    printf("\nCurrent Character State:\n");
    for (int i = 0; i < 21; i++)
    {
        if (map->characters[i].location_x != 0 || map->characters[i].location_y != 0)
        {
            printf("%d | Character %c: X = %d, Y = %d, Alive = %d\n", i,
                   map->characters[i].character, map->characters[i].location_x, map->characters[i].location_y,map->characters[i].alive);
        }
    }
}
