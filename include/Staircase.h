#ifndef STAIRCASE_H
#define STAIRCASE_H

#include "DungeonDefs.h"
#include "Room.h"

// Place staircases in the starting and boss rooms
void PlaceStaircases(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int roomCount, int currentFloor);

#endif // STAIRCASE_H