#ifndef DUNGEONGENERATION_H
#define DUNGEONGENERATION_H
#define WIDTH 80
#define HEIGHT 21

typedef struct
{
    char type;    // Dungeon type: ' ', '.', '#', '<', '>'
    int hardness; // Hardness: 0-255: 0 = Room/Corridor, 255 = edges, spaces = 1-254
} Cell;

int init_Dungeon(Cell grid[HEIGHT][WIDTH]);
int init_Rooms(Cell grid[HEIGHT][WIDTH]);
int init_Corridors(Cell grid[HEIGHT][WIDTH], int Room_A_X, int Room_A_Y, int Room_B_X, int Room_B_Y);

#endif