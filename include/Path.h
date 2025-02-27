#ifndef PATH_H
#define PATH_H

#include "DungeonDefs.h"
#include "Room.h"
#include "Corridor.h" // Not coloured correctly on my IDE for some reason but very important, include!

// Path generation constants
#define CELL_SCOUT_AMOUNT 2    // Cell to look ahead while pathfinding
#define PATH_LENGTH_THRESHOLD 1.5f // Only create new path if it reduces length by 33%

// Main path generation function
void GeneratePaths(int grid[GRID_HEIGHT][GRID_WIDTH],
                   Room rooms[], int roomCount, int startRoomIndex, int bossRoomIndex);

#endif //PATH_H