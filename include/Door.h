#ifndef DOOR_H
#define DOOR_H

#include <stdbool.h>
#include "DungeonDefs.h"
#include "Room.h"

// Door Constants
#define DOOR_NEXT_CHANCE_INITIAL 100
#define DOOR_CHANCE_DECREASE 15

bool ConnectRoomsViaDoors(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int roomCount);
bool FindDoorPosition(int grid[GRID_HEIGHT][GRID_WIDTH], Room room, int* doorX, int* doorY);

#endif // DOOR_H