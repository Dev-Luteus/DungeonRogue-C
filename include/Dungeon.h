#ifndef DUNGEON_H
#define DUNGEON_H

// Cell Types
#define CELL_EMPTY_1 0 // Empty
#define CELL_EMPTY_2 1 // Empty 2 (currently just for pattern)
#define CELL_ROOM 2
#define CELL_CORRIDOR 3

// General
// The Height and Width should be an ODD number
#define GRID_HEIGHT 99
#define GRID_WIDTH 99
#define CELL_SIZE 10
#define GRID_TOTAL_HEIGHT GRID_HEIGHT * CELL_SIZE
#define GRID_TOTAL_WIDTH GRID_WIDTH * CELL_SIZE

void GenerateGrid(int grid[GRID_HEIGHT][GRID_WIDTH]);
void GenerateDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);
void PrintDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);

// Rooms
#define ROOM_MAX_SIZE 26
#define ROOM_MIN_SIZE 10
#define ROOM_MIN_WIDTH 10
#define ROOM_MIN_HEIGHT 10
#define ROOM_AMOUNT 24

typedef struct
{
    int x;
    int y;
    int width;
    int height;
} Room;

Room CreateRoom(int x, int y, int width, int height);
bool GenerateRoom(int grid[GRID_HEIGHT][GRID_WIDTH]);
bool IsRoomValid(int grid[GRID_HEIGHT][GRID_WIDTH], Room room);
void PlaceRoom(int grid[GRID_HEIGHT][GRID_WIDTH], Room room);
void GenerateRooms(int grid[GRID_HEIGHT][GRID_WIDTH]);

// Corridors
typedef struct
{
    int x;
    int y;
} Corridor;

typedef enum
{
    DIR_NORTH = 0,
    DIR_EAST = 1,
    DIR_SOUTH = 2,
    DIR_WEST = 3
} Direction;

bool IsValidCorridorCell(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y);
void RandomizedFloodFill(int grid[GRID_HEIGHT][GRID_WIDTH], int startX, int startY);
void GenerateMazes(int grid[GRID_HEIGHT][GRID_WIDTH]);

#endif //DUNGEON_H
