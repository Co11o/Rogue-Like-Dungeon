#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <stdint.h>
#include <limits.h>
#include "dungeon_generation.h"
#include "priority_queue.h"
#include "dijkstra.h"
#include "characters.h"
#define WIDTH 80
#define HEIGHT 21

int Dijkstras_Tunneling(Dungeon *dungeon, Character_Map *map, int distMap[HEIGHT][WIDTH]) {
    PriorityQueue priority_queue;
    priority_queue.size = 0;

    // Movement directions (8-way)
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    // Initialize Distance Map
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            distMap[y][x] = INT_MAX;
        }
    }

    // Set Player Position
    int px = map->pc->location_x;
    int py = map->pc->location_y;
    distMap[py][px] = 0;
    if(addNode(&priority_queue, px, py, 0)){
        printf("Failed to add source node to queue");
        return -1;
    }

    // While queue isn't empty
    while (priority_queue.size > 0) {
        // Extract Min
        queue_node current;
        extractNode(&priority_queue, &current);
        // Get min properties
        int ux = current.x_location;
        int uy = current.y_location;
        int currentDist = current.priority;
        // Check Neighbors
        for (int i = 0; i < 8; i++) {
            int vx = ux + dx[i];
            int vy = uy + dy[i];
            // Cost to get to neighbors
            if (vx >= 1 && vx < WIDTH-1 && vy >= 1 && vy < HEIGHT-1) {
                int hardness = dungeon->grid[uy][ux].hardness;
                if (hardness < 255) {  // If not an impassable wall
                    int cost = (hardness / 85) + 1;
                    int newDist = currentDist + cost;
                    // New min distance for node
                    if (newDist < distMap[vy][vx]) {
                        // New node
                        if(distMap[vy][vx] == INT_MAX){
                            if(addNode(&priority_queue, vx, vy, newDist)){
                                printf("Failed to add a node to queue");
                                return -1;
                            }
                        }
                        // Existing node with new min distance
                        else{ 
                            decrease_priority(&priority_queue, vx, vy, newDist);
                        }
                        //Update distance map
                        distMap[vy][vx] = newDist;
                    }
                }
            }
        }
    }
    return 0;
}

int Dijkstra_NonTunneling(Dungeon *dungeon, Character_Map *map, int distMap[HEIGHT][WIDTH]) {
    PriorityQueue priority_queue;
    priority_queue.size = 0;

    // Movement directions (8-way)
    int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};

    // Initialize Distance Map
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            distMap[y][x] = INT_MAX;
        }
    }

    // Set Player Position
    int px = map->pc->location_x;
    int py = map->pc->location_y;
    distMap[py][px] = 0;
    if(addNode(&priority_queue, px, py, 0)){
        printf("Failed to add source node to queue");
        return -1;
    }

    while (priority_queue.size > 0) {
        queue_node current;
        extractNode(&priority_queue, &current);
        int ux = current.x_location;
        int uy = current.y_location;
        int currentDist = current.priority;

        for (int i = 0; i < 8; i++) {
            int vx = ux + dx[i];
            int vy = uy + dy[i];

            if (vx >= 1 && vx < WIDTH-1 && vy >= 1 && vy < HEIGHT-1) {
                if (dungeon->grid[vy][vx].hardness == 0) {//Must be a floor
                    int cost = 1;
                    int newDist = currentDist + cost;

                    if (newDist < distMap[vy][vx]) {
                        if(distMap[vy][vx] == INT_MAX){
                            if(addNode(&priority_queue, vx, vy, newDist)){
                                printf("Failed to add a node to queue");
                                return -1;
                            }
                        }else{
                            decrease_priority(&priority_queue, vx, vy, newDist);
                        }
                        distMap[vy][vx] = newDist;
                    }
                }
            }
        }
    }
    return 0;
}