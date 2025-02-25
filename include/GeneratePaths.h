#ifndef GENERATEPATHS_H
#define GENERATEPATHS_H

#include <stdbool.h>

#include "Dungeon.h"

#define CELL_SCOUT_AMOUNT 2    // Cell to look ahead while pathfinding
#define PATH_LENGTH_THRESHOLD 1.5f // Only create new path if it reduces length by 33%
#define GRID_SIZE (GRID_WIDTH * GRID_HEIGHT)

// Logic preprocessor macros
#define GET_GRID_INDEX(x, y) ((y) * GRID_WIDTH + (x))
#define IS_VALID_CELL(x, y) ((x) >= 1 && (x) < GRID_WIDTH - 1 && (y) >= 1 && (y) < GRID_HEIGHT - 1)
#define CAN_BE_PATH(cell) ((cell) == CELL_CORRIDOR || (cell) == CELL_EMPTY_1 || (cell) == CELL_EMPTY_2)

// Path
static bool InitializePathfinding(bool** connected, Corridor** queue, bool** visited,
    Corridor** previous, int roomCount, int* startDoorX, int* startDoorY,
    int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int startRoomIndex);

static bool FindPathBetweenDoors(int grid[GRID_HEIGHT][GRID_WIDTH],
    Corridor currentDoor, int nextDoorX, int nextDoorY,
    Corridor* queue, bool* visited, Corridor* previous);

void GeneratePaths(int grid[GRID_HEIGHT][GRID_WIDTH],
    Room rooms[], int roomCount, int startRoomIndex, int bossRoomIndex);

// Points
Room FindStartingRoom(Room rooms[], int roomCount);
Room FindBossRoom(Room rooms[], int roomCount);
bool FindDoorPosition(int grid[GRID_HEIGHT][GRID_WIDTH], Room room, int* doorX, int* doorY);

#endif //GENERATEPATHS_H
