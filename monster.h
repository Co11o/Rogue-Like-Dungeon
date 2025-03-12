#ifndef MONSTER_H
#define MONSTER_H

#include <stdint.h>
#include "dungeon_generation.h"
#include "priority_queue.h"


#define MAX_MONSTERS 20
#define WIDTH 80
#define HEIGHT 21

typedef struct {
    int characteristics;
    uint8_t location_x;
    uint8_t location_y;
    uint8_t character;
    int last_seen_pc_x;
    int last_seen_pc_y;
    int speed;
    int alive;
} Character;

typedef struct {
    char grid[HEIGHT][WIDTH];
    Character characters[MAX_MONSTERS + 1];
} Character_Map;

int init_character_map(Dungeon *dungeon, Character_Map *map, int number_of_monsters);

int init_monster(Character_Map *map, Character *monster, Room *room);

int assign_monster_char(Character *monster);

int init_movement_queue(PriorityQueue *Queue, Character_Map *map);

int add_movement_queue(PriorityQueue *Queue, Character *character, int current_turn);

void process_next_event(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, int distMapTunnel[HEIGHT][WIDTH], int distMapNonTunnel[HEIGHT][WIDTH]);

int character_move(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Character *character, int distMapTunnel[HEIGHT][WIDTH], int distMapNonTunnel[HEIGHT][WIDTH]);

/* TODO BOUNDS CHECK FOR IMMUNTABLE*/
int tunnel(Dungeon *dungeon, int x, int y);

/* Working */
void change_position(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Character *character, int move_to);

void change_position_non_intelligent_tunneling(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Character *character);

void change_position_non_intelligent_non_tunneling(PriorityQueue *Queue, Dungeon *dungeon, Character_Map *map, Character *character);

int monster_line_of_sight(Dungeon *dungeon, Character *character, Character *PC);
void printQueue(PriorityQueue *pq, Character_Map *map);
void printCharacterMap(Character_Map *map);

#endif