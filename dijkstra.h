#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#define WIDTH 80
#define HEIGHT 21

int Dijkstras_Tunneling(Dungeon *dungeon,int distMap[HEIGHT][WIDTH]);
int Dijkstra_NonTunneling(Dungeon *dungeon, int distMap[HEIGHT][WIDTH]);

#endif