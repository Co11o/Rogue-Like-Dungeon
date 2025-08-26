#ifndef DUNGEONGENERATION_H
#define DUNGEONGENERATION_H

#include <stdint.h>
#define WIDTH 80
#define HEIGHT 21
#define MAX_ROOMS 1000
#define MAX_STAIRS 1000

typedef struct {
    uint8_t x, y, width, height;
} Room;  // Define Room before Dungeon

typedef struct {
    char type;    // Dungeon type: ' ', '.', '#', '<', '>'
    uint8_t hardness; // Hardness: 0-255: 0 = Room/Corridor, 255 = edges, spaces = 1-254
} Cell;

typedef struct {
    uint8_t location_x;
    uint8_t location_y;
    uint8_t  direction; // -1 = down, 1 = up, other value = error
} Stair;

typedef struct {
    Cell grid[HEIGHT][WIDTH];
    Cell PCCell;
    Room rooms[MAX_ROOMS];  // Now Room is properly defined
    Stair stairs[MAX_STAIRS];
    uint16_t room_count;
    uint8_t player_x, player_y;
    uint16_t stair_up_count, stair_down_count;
} Dungeon;

int init_Dungeon(Dungeon *dungeon);
int init_Rooms(Dungeon *dungeon);
int init_Corridors(Dungeon *dungeon, int x1, int y1, int x2, int y2);
int print_Dungeon(Dungeon *dungeon);

#endif
