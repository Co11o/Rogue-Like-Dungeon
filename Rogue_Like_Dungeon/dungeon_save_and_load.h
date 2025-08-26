#ifndef DUNGEONSAVEANDLOAD_H
#define DUNGEONSAVEANDLOAD_H

#include "dungeon_generation.h"

int save_dungeon(const char *path, Dungeon *dungeon);
int load_dungeon(const char *path, Dungeon *dungeon);

#endif