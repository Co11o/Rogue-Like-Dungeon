#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dungeon_generation.h"

#define WIDTH 80
#define HEIGHT 21
#define MAX_ROOMS 1000


// Initialize dungeon with walls and random rock hardness
int init_Dungeon(Dungeon *dungeon) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            dungeon->grid[y][x].type = ' ';                    // Default to rock
            dungeon->grid[y][x].hardness = 1 + (rand() % 254); // Set random hardness 1-254
        }
    }

    // Create indestructible boundary walls
    for (int y = 1; y < HEIGHT - 1; y++) {
        dungeon->grid[y][0].type = '|';
        dungeon->grid[y][0].hardness = 255;
        dungeon->grid[y][WIDTH - 1].type = '|';
        dungeon->grid[y][WIDTH - 1].hardness = 255;
    }
    for (int x = 0; x < WIDTH; x++) {
        dungeon->grid[0][x].type = '-';
        dungeon->grid[0][x].hardness = 255;
        dungeon->grid[HEIGHT - 1][x].type = '-';
        dungeon->grid[HEIGHT - 1][x].hardness = 255;
    }

    dungeon->room_count = 0;
    dungeon->player_x = 255;
    dungeon->player_y = 255;
    dungeon->stair_up_count = 0;
    dungeon->stair_down_count = 0;
    return 0;
}

// Generate rooms with corridors
int init_Rooms(Dungeon *dungeon) {
    dungeon->room_count = 0;
    int error_track = 0;

    for (int i = 0; i < 7 + rand() % 4; i++) {
        Room room;
        room.width = 5 + (rand() % 8);
        room.height = 6 + (rand() % 4);
        room.x = 2 + (rand() % (WIDTH - room.width - 4));
        room.y = 2 + (rand() % (HEIGHT - room.height - 4));

        // Check if the room overlaps
        int valid = 1;
        for (int y = room.y - 1; y < room.y + room.height + 1; y++) {
            for (int x = room.x - 1; x < room.x + room.width + 1; x++) {
                if (dungeon->grid[y][x].type == '.') {
                    valid = 0;
                }
            }
        }

        if (valid) {
            // Place the room
            for (int y = 0; y < room.height; y++) {
                for (int x = 0; x < room.width; x++) {
                    dungeon->grid[room.y + y][room.x + x].type = '.';
                    dungeon->grid[room.y + y][room.x + x].hardness = 0;
                }
            }

            // Store the room
            dungeon->rooms[dungeon->room_count++] = room;

            // Place stairs and player character

            Stair stair;
            stair.location_x = room.x + rand() % room.width;
            stair.location_y = room.y + rand() % room.height;

            if (rand() % 4 == 0) {
                dungeon->grid[stair.location_y][stair.location_x].type = '<';
                stair.direction = 1;
                dungeon->stairs[dungeon->stair_up_count + dungeon->stair_down_count] = stair;
                dungeon->stair_up_count++;
            } else if (rand() % 4 == 1) {
                dungeon->grid[stair.location_y][stair.location_x].type = '>';
                stair.direction =255;
                dungeon->stairs[dungeon->stair_up_count + dungeon->stair_down_count] = stair;
                dungeon->stair_down_count++;
            } else if (dungeon->player_x == 255 && dungeon->player_y == 255) {
                dungeon->player_x = stair.location_x;
                dungeon->player_y = stair.location_y;
                if(dungeon->grid[dungeon->player_y][dungeon->player_x].type == '>'){
                    dungeon->PCCell.hardness=0;
                    dungeon->PCCell.type = '>';
                }else if(dungeon->grid[dungeon->player_y][dungeon->player_x].type == '<'){
                    dungeon->PCCell.hardness=0;
                    dungeon->PCCell.type = '<';
                }else{ // == '.'
                    dungeon->PCCell.hardness=0;
                    dungeon->PCCell.type = '.';
                }
                dungeon->grid[dungeon->player_y][dungeon->player_x].type = '@';
            }

            if(i==4 && dungeon->stair_down_count == 0){
                dungeon->grid[stair.location_y][stair.location_x].type = '>';
                stair.direction = 255;
                dungeon->stairs[dungeon->stair_up_count + dungeon->stair_down_count] = stair;
                dungeon->stair_down_count++;
            }
            if(i==3 && dungeon->stair_up_count == 0){
                dungeon->grid[stair.location_y][stair.location_x].type = '<';
                stair.direction = 1;
                dungeon->stairs[dungeon->stair_up_count + dungeon->stair_down_count] = stair;
                dungeon->stair_up_count++;
            }

            // Connect rooms with corridors
            if (dungeon->room_count > 1) {
                Room prev = dungeon->rooms[dungeon->room_count - 2];
                Room curr = dungeon->rooms[dungeon->room_count - 1];
                init_Corridors(dungeon, prev.x + prev.width / 2, prev.y + prev.height / 2,
                               curr.x + curr.width / 2, curr.y + curr.height / 2);
            }
        } else {
            i--; // Retry
            error_track++;
        }

        if (error_track >= 2000) {
            break;
        }
    }
    return 0;
}

// Create corridors between rooms
int init_Corridors(Dungeon *dungeon, int x1, int y1, int x2, int y2) {
    int x_dir = (x2 > x1) ? 1 : -1;
    int y_dir = (y2 > y1) ? 1 : -1;

    // Horizontal corridor
    for (int x = x1; x != x2; x += x_dir) {
        if (dungeon->grid[y1][x].type == ' ') {
            dungeon->grid[y1][x].type = '#';
            dungeon->grid[y1][x].hardness = 0;
        }
    }

    // Vertical corridor
    for (int y = y1; y != y2; y += y_dir) {
        if (dungeon->grid[y][x2].type == ' ') {
            dungeon->grid[y][x2].type = '#';
            dungeon->grid[y][x2].hardness = 0;
        }
    }
    return 0;
}

// Print dungeon for debugging
int print_Dungeon(Dungeon *dungeon) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            printf("%c ", dungeon->grid[y][x].type);
        }
        printf("\n");
    }
    return 1;
}
