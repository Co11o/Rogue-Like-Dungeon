#ifndef DIJKSTRA_H
#define DIJKSTRA_H
#include "characters.h"

#define WIDTH 80
#define HEIGHT 21

int Dijkstras_Tunneling(Dungeon *dungeon, Character_Map *map,int distMap[HEIGHT][WIDTH]);
int Dijkstra_NonTunneling(Dungeon *dungeon, Character_Map *map, int distMap[HEIGHT][WIDTH]);

#endif