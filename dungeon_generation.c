#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dungeon_generation.h"

#define WIDTH 80
#define HEIGHT 21

// Initialize dungeon with walls and random rock hardness
int init_Dungeon(Cell grid[HEIGHT][WIDTH])
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            grid[i][j].type = ' ';                    // Default to rock
            grid[i][j].hardness = 1 + (rand() % 254); // Set random hardness 1-254
        }
    }

    // Create indestructible outer walls/Boundry
    for (int i = 1; i < HEIGHT - 1; i++)
    {
        grid[i][0].type = '|';
        grid[i][0].hardness = 255;
        grid[i][WIDTH - 1].type = '|';
        grid[i][WIDTH - 1].hardness = 255;
    }
    for (int i = 0; i < WIDTH; i++)
    {
        grid[0][i].type = '-';
        grid[0][i].hardness = 255;
        grid[HEIGHT - 1][i].type = '-';
        grid[HEIGHT - 1][i].hardness = 255;
    }

    return 0;
}

// Generate rooms with corridors
int init_Rooms(Cell grid[HEIGHT][WIDTH])
{
    int roomCount,error_track;
    int roomEndX, roomEndY;
    int roomStartX, roomStartY;
    int roomSize;
    int validLocation;
    int CorridorAXStart, CorridorAYStart;
    int StairUp, StairDown;
    StairDown = 0;
    StairUp = 0;
    error_track=0;
    //Generate 7 to 10 rooms
    for (roomCount = 0; roomCount < (7 + rand() % 4); roomCount++)
    {
        // X Size and Location
        roomSize = 5 + (rand() % 8);
        roomStartX = 2 + (rand() % 76);
        if ((roomStartX + roomSize > 77))
        {
            roomEndX = roomStartX;
            roomStartX = roomStartX - roomSize;
        }
        else
        {
            roomEndX = roomStartX + roomSize;
        }

        // Y size and Location
        roomSize = 6 + (rand() % 4);
        roomStartY = 2 + (rand() % 17);
        if ((roomStartY + roomSize > 18))
        {
            roomEndY = roomStartY;
            roomStartY = roomStartY - roomSize;
        }
        else
        {
            roomEndY = roomStartY + roomSize;
        }

        //Check if Location is valid ie no overlap and has at least one empty space between rooms
        validLocation = 1;
        for (int i = 0; i < (roomEndY - roomStartY) + 2; i++)
        {
            for (int j = 0; j < (roomEndX - roomStartX) + 2; j++)
            {
                if (grid[roomStartY + i - 1][roomStartX + j - 1].type == '.')
                {
                    validLocation = 0;
                }
            }
        }

        //Insert Valid Room
        if (validLocation == 1)
        {
            for (int i = 0; i < (roomEndY - roomStartY); i++)
            {
                for (int j = 0; j < (roomEndX - roomStartX); j++)
                {
                    for (i = 0; i < (roomEndY - roomStartY); i++)
                    {
                        //Probability of having extra stairs
                        if (rand() % 250 == 0)
                        {
                            grid[roomStartY + i][roomStartX + j].type = '<';
                            StairUp++;
                        }
                        else if (rand() % 250 == 1)
                        {
                            grid[roomStartY + i][roomStartX + j].type = '>';
                            StairDown++;
                        }
                        //Fill with .'s if not stairs
                        else
                        {
                            grid[roomStartY + i][roomStartX + j].type = '.';
                        }
                        //.'s, <'s, and >'s all have hardness = 0
                        grid[roomStartY + i][roomStartX + j].hardness = 0; // Room has hardness 0
                    }
                }
                // Guarente that by the 5th room one stair down exists
                if (roomCount == 5 && StairDown == 0)
                {
                    grid[roomStartY + (rand() % (roomEndY - roomStartY))][roomStartX + (rand() % (roomEndX - roomStartX))].type = '>';
                }
                // Guarente that by the 6th room one stair up exists
                if (roomCount == 6 && StairUp == 0)
                {
                    grid[roomStartY + (rand() % (roomEndY - roomStartY))][roomStartX + (rand() % (roomEndX - roomStartX))].type = '<';
                }

                //Setup and Connect new room to previous room made
                if (roomCount > 0)
                {
                    init_Corridors(grid, CorridorAXStart, CorridorAYStart,
                                   roomStartX + 2 + (rand() % (roomEndX - roomStartX - 4)),
                                   roomStartY + 2 + (rand() % (roomEndY - roomStartY - 4)));
                }
                CorridorAXStart = roomStartX + 2 + (rand() % (roomEndX - roomStartX - 4));
                CorridorAYStart = roomStartY + 2 + (rand() % (roomEndY - roomStartY - 4));
            }
        }
        //Not valid location don't increase roomcount
        else
        {
            roomCount--;
            error_track++;
        }
        if(error_track >= 2000){
            return 1;
        }
    }
    return 0;
}

int init_Corridors(Cell grid[HEIGHT][WIDTH], int Room_A_X, int Room_A_Y, int Room_B_X, int Room_B_Y)
{
    int i, Xdirection, Ydirection;

    // Determine X direction
    if (Room_B_X > Room_A_X)
    {
        Xdirection = 1;
    }
    else
    {
        Xdirection = -1;
    }

    // Determine Y direction
    if (Room_B_Y > Room_A_Y)
    {
        Ydirection = 1;
    }
    else
    {
        Ydirection = -1;
    }

    // Create Horizontal corridor
    for (i = 0; i <= abs(Room_B_X - Room_A_X); i++)
    {
        if (grid[Room_A_Y][Room_A_X + i * Xdirection].type == ' ')
        {
            if(rand() % 400 == 0){
                grid[Room_A_Y][Room_A_X + i * Xdirection].type = '>';
            }else if (rand() % 400 == 0){
                grid[Room_A_Y][Room_A_X + i * Xdirection].type = '<';
            }else{
                grid[Room_A_Y][Room_A_X + i * Xdirection].type = '#';
            }
            grid[Room_A_Y][Room_A_X + i * Xdirection].hardness = 0;
        }
    }

    // Create Vertical corridor from end of created horizontal corridor
    for (i = 0; i <= abs(Room_B_Y - Room_A_Y); i++)
    {
        if (grid[Room_A_Y + i * Ydirection][Room_B_X].type == ' ')
        {
            if(rand() % 400 == 0){
                grid[Room_A_Y + i * Ydirection][Room_B_X].type = '>';
            }else if (rand() % 400 == 0){
                grid[Room_A_Y + i * Ydirection][Room_B_X].type = '<';
            }else{
            grid[Room_A_Y + i * Ydirection][Room_B_X].type = '#';
            }
            grid[Room_A_Y + i * Ydirection][Room_B_X].hardness = 0;
        }
    }

    return 0;
}