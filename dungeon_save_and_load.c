#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <stdint.h>
#include "dungeon_generation.h"

#define FILE_MARKER "RLG327-S2025"
#define VERSION 0

int save_dungeon(const char *path, Dungeon *dungeon)
{
    FILE *f = fopen(path, "wb");
    if (!f)
    {
        perror("Failed to open file for saving");
        return -1;
    }

    // Write file marker
    fwrite(FILE_MARKER, 1, 12, f);

    // Write version (big-endian)
    uint32_t version = htobe32(VERSION);
    fwrite(&version, sizeof(version), 1, f);

    // Compute and write file size (big-endian)
    uint32_t file_size = htobe32(1708 + dungeon->room_count * 4 +
                                 dungeon->stair_up_count * 2 +
                                 dungeon->stair_down_count * 2);
    fwrite(&file_size, sizeof(file_size), 1, f);

    // Write player position (big-endian)
    //printf("Save: PC: x %d, y %d\n",dungeon->player_x,dungeon->player_y);
    fwrite(&dungeon->player_x, sizeof(dungeon->player_x), 1, f);
    fwrite(&dungeon->player_y, sizeof(dungeon->player_y), 1, f);


    // Write hardness values
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            fwrite(&dungeon->grid[y][x].hardness, 1, 1, f);
        }
    }

    // Write room count (big-endian)
    uint16_t room_count = htobe16(dungeon->room_count);
    //printf("Save: Room Count: %d\n",dungeon->room_count);
    fwrite(&room_count, sizeof(room_count), 1, f);

    // Write rooms
    for (int i = 0; i < dungeon->room_count; i++)
    {
        uint8_t x = dungeon->rooms[i].x;
        uint8_t y = dungeon->rooms[i].y;
        uint8_t w = dungeon->rooms[i].width;
        uint8_t h = dungeon->rooms[i].height;
        //printf("Save Room %d | X %u : Y %u : Width %u : Height %u\n",i,x,y,w,h);
        fwrite(&x, 1, 1, f);
        fwrite(&y, 1, 1, f);
        fwrite(&w, 1, 1, f);
        fwrite(&h, 1, 1, f);
    }

    // Write stair counts (big-endian)
    uint16_t up_count = htobe16(dungeon->stair_up_count);
    //printf("Save: Stair Count UP: x %d\n",dungeon->stair_up_count);
    fwrite(&up_count, sizeof(up_count), 1, f);

    // Write stair positions
    for (int i = 0; i < dungeon->stair_up_count + dungeon->stair_down_count; i++)
    {
        //printf("Save | Stair %d | X %u : Y %u | DIR %u\n",i,dungeon->stairs[i].location_x,dungeon->stairs[i].location_y,dungeon->stairs[i].direction);
        if (dungeon->stairs[i].direction == 1)
        {
            uint8_t sx = dungeon->stairs[i].location_x;
            uint8_t sy = dungeon->stairs[i].location_y;
            //printf("Save | Stair UP %d | X %u : Y %u\n",i,sx,sy);
            fwrite(&sx, 1, 1, f);
            fwrite(&sy, 1, 1, f);
        }
    }

    uint16_t down_count = htobe16(dungeon->stair_down_count);
    //printf("Save: Stair Count DOWN: %d\n",dungeon->stair_down_count);
    fwrite(&down_count, sizeof(down_count), 1, f);

    // Write stair positions
    for (int i = 0; i < dungeon->stair_up_count + dungeon->stair_down_count; i++)
    {
        if (dungeon->stairs[i].direction != 1)
        {
            uint8_t sx = dungeon->stairs[i].location_x;
            uint8_t sy = dungeon->stairs[i].location_y;
            //printf("Save | Stair DOWN %d | X %u : Y %u\n",i,sx,sy);
            fwrite(&sx, 1, 1, f);
            fwrite(&sy, 1, 1, f);
        }
    }
    fclose(f);
    return 0;
}

int load_dungeon(const char *path, Dungeon *dungeon)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        perror("Failed to open file for loading");
        return -1;
    }

    char marker[12];
    fread(marker, 1, 12, f);
    if (memcmp(marker, FILE_MARKER, 12) != 0)
    {
        fprintf(stderr, "Invalid file format\n");
        fclose(f);
        return -1;
    }

    uint32_t version;
    fread(&version, sizeof(version), 1, f);
    version = be32toh(version);

    uint32_t file_size;
    fread(&file_size, sizeof(file_size), 1, f);
    file_size = be32toh(file_size);

    fread(&dungeon->player_x, sizeof(dungeon->player_x), 1, f);
    fread(&dungeon->player_y, sizeof(dungeon->player_y), 1, f);
    //printf("Load: PC: x %d,y %d\n",dungeon->player_x, dungeon->player_y);

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            fread(&dungeon->grid[y][x].hardness, 1, 1, f);
            if (dungeon->grid[y][x].hardness == 0)
            {
                dungeon->grid[y][x].type = '#'; //
            }
        }
    }

    uint16_t room_count;
    fread(&room_count, sizeof(room_count), 1, f);
    dungeon->room_count = be16toh(room_count);
    //printf("Load: Room Count: %d\n",dungeon->room_count);

    for (int i = 0; i < dungeon->room_count; i++)
    {
        Room r;
        fread(&r.x, 1, 1, f);
        fread(&r.y, 1, 1, f);
        fread(&r.width, 1, 1, f);
        fread(&r.height, 1, 1, f);
        dungeon->rooms[i].x = r.x;
        dungeon->rooms[i].y = r.y;
        dungeon->rooms[i].height = r.height;
        dungeon->rooms[i].width = r.width;

        //printf("Load Room %d | X %u : Y %u : Width %u : Height %u\n",i,dungeon->rooms[i].x,dungeon->rooms[i].y
        //        ,dungeon->rooms[i].width,dungeon->rooms[i].height);
        
        for (int y = 0; y < dungeon->rooms[i].height; y++)
        {
            for (int x = 0; x < dungeon->rooms[i].width; x++)
            {
                dungeon->grid[dungeon->rooms[i].y + y][dungeon->rooms[i].x + x].type = '.'; //
            }
        }
        dungeon->rooms[i] = r;
    }

    uint16_t up_count, down_count;
    fread(&up_count, sizeof(up_count), 1, f);
    dungeon->stair_up_count = be16toh(up_count);
    //printf("Load: Stair Count UP: %d\n",dungeon->stair_up_count);
    
    Stair stair;
    for (int i = 0; i < dungeon->stair_up_count; i++)
    {
        fread(&dungeon->stairs[i].location_x, 1, 1, f);
        fread(&dungeon->stairs[i].location_y, 1, 1, f);
        stair.location_x = dungeon->stairs[i].location_x;
        stair.location_y = dungeon->stairs[i].location_y;
        stair.direction = 1;
        //printf("Load Stair UP %d | X %u : Y %u\n",i,stair.location_x,stair.location_y);
        dungeon->stairs[i] = stair;
        dungeon->grid[dungeon->stairs[i].location_y][dungeon->stairs[i].location_x].type = '<';
    }

    
    fread(&down_count, sizeof(down_count), 1, f);
    dungeon->stair_down_count = be16toh(down_count);
    //printf("Load: Stair Count DOWN: %d\n",dungeon->stair_down_count);

    for (int i = dungeon->stair_up_count; i < dungeon->stair_up_count + dungeon->stair_down_count; i++)
    {
        fread(&dungeon->stairs[i].location_x, 1, 1, f);
        fread(&dungeon->stairs[i].location_y, 1, 1, f);
        stair.location_x = dungeon->stairs[i].location_x;
        stair.location_y = dungeon->stairs[i].location_y;
        stair.direction = 255;
        //printf("Load Stair DOWN %d | X %u : Y %u\n",i,stair.location_x,stair.location_y);
        dungeon->stairs[i] = stair;
        dungeon->grid[dungeon->stairs[i].location_y][dungeon->stairs[i].location_x].type = '>';
    }

    dungeon->PCCell.hardness=0;
    dungeon->PCCell.type = dungeon->grid[dungeon->player_y][dungeon->player_x].type;
    dungeon->grid[dungeon->player_y][dungeon->player_x].type = '.';

    fclose(f);
    return 0;
}