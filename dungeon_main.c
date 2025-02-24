#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "dungeon_generation.h"

int main()
{
    Cell grid[HEIGHT][WIDTH]; // Grid of objects
    srand(time(NULL));

    if (init_Dungeon(grid))
    {
        printf("Failed to generate blank dungeon");
        return -1;
    }
    if (init_Rooms(grid))
    {
        printf("Failed to generate rooms try again");
        return -1;
    }

    // Print Dungeon Layout
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            printf("%c ", grid[i][j].type);
        }
        printf("\n");
    }

    // Print Hardness Map
    //  printf("\nHardness Map:\n");
    //  for (int i = 0; i < HEIGHT; i++)
    //  {
    //      for (int j = 0; j < WIDTH; j++)
    //      {
    //          printf("%d ", grid[i][j].hardness % 10);
    //      }
    //      printf("\n");
    //  }

    return 0;
}