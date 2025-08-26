#ifndef DISPLAY_H
#define DISPLAY_H
#include <ncurses.h>
#include "characters.h"
#include "dungeon_generation.h"
#include "items.h"

#define WIDTH 80
#define HEIGHT 21

void init_terminal(void);

void update_window(Character_Map *map, item_map *item_map, Dungeon *dungeon, char *input);

void update_win(void);

void update_lose(void);

void update_quit(void);

void display_monster_list(Dungeon *dungeon, item_map *item_map, Character_Map *map,int num_Monster,char *input);

void display_tunneling_distances(int distMap[HEIGHT][WIDTH],Dungeon *dungeon, item_map *item_map, Character_Map *map,char *input);

void display_non_tunneling_distances(int distMap[HEIGHT][WIDTH],Dungeon *dungeon, item_map *item_map, Character_Map *map,char *input);

void display_hardness(Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input);

void display_character_info(Dungeon *dungeon, item_map *item_map, Character_Map *map, int num_Monster,char *input);

void display_message(char *message);

void display_message_str(const char *message);

void display_window_FOG(Character_Map *map, item_map *item_map, Dungeon *dungeon);

void display_queue(PriorityQueue *Queue, Dungeon *dungeon, item_map *item_map, Character_Map *map, int num_Monster, char *input);

void display_teleport(Character_Map *character_map, Dungeon *dungeon, int teleport_x, int teleport_y);

void display_inventory(Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input);

void display_equipment(Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input);

void inspect_item(Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input);

void inspect_monster(Dungeon *dungeon, item_map *item_map, Character_Map *map, char *input);

#endif
