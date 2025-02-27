#ifndef CORRIDOR_H
#define CORRIDOR_H

#include <stdbool.h>
#include "DungeonDefs.h"

// Complete Corridor struct definition
typedef struct Corridor {
    int x;
    int y;
} Corridor;

typedef enum {
    DIR_NORTH = 0,
    DIR_EAST = 1,
    DIR_SOUTH = 2,
    DIR_WEST = 3
} Direction;

bool IsValidCorridorCell(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y);
void RandomizedFloodFill(int grid[GRID_HEIGHT][GRID_WIDTH], int startX, int startY);
void GenerateMazes(int grid[GRID_HEIGHT][GRID_WIDTH]);

#endif // CORRIDOR_H