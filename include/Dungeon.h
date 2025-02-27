#ifndef DUNGEON_H
#define DUNGEON_H

#include <stdbool.h>
#include "DungeonDefs.h"
#include "Room.h"

// Core dungeon functions
void GenerateGrid(int grid[GRID_HEIGHT][GRID_WIDTH]);
bool GenerateDungeon(int grid[GRID_HEIGHT][GRID_WIDTH], int maxAttempts, int currentFloor,
                     Room rooms[], int* roomCount);
void PrintDungeon(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int roomCount);

#endif //DUNGEON_H