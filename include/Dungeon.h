#ifndef DUNGEON_H
#define DUNGEON_H

// Cell Types
#define CELL_EMPTY_1 0 // Empty
#define CELL_EMPTY_2 1 // Empty 2 (currently just for pattern)
#define CELL_ROOM 2
#define CELL_CORRIDOR 3

// General
#define GRID_HEIGHT 100
#define GRID_WIDTH 100
#define CELL_SIZE 10
#define GRID_TOTAL_HEIGHT GRID_HEIGHT * CELL_SIZE
#define GRID_TOTAL_WIDTH GRID_WIDTH * CELL_SIZE

void GenerateGrid(int grid[GRID_HEIGHT][GRID_WIDTH]);
void GenerateDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);
void PrintDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);

// Rooms
#define ROOM_MAX_SIZE 30
#define ROOM_MIN_SIZE 10
#define ROOM_MIN_WIDTH 10
#define ROOM_MIN_HEIGHT 10
#define ROOM_AMOUNT 20

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
/* Normally you'd think about the origin being at (0,0), but in computer graphics,
 * The origin is often in the top left corner. Hence, Y has to be negative to go up!
 * So we essentially flip the Y axis.
 * North: (0, -1), East: (1, 0), South: (0, 1), West: (-1, 0)
 *
 * Also, two single-dimension arrays are apparently more performant,
 * and more cache friendly than using a 2D array with more overhead.
 */
#define dirX [0, 1, 0, -1] // North, East, South, West
#define dirY [-1, 0, 1, 0] // North, East, South, West

typedef struct
{
    int x;
    int y;
} Corridor;

bool IsValidCorridorCell(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y);
void RandomizedFloodFill(int grid[GRID_HEIGHT][GRID_WIDTH], int startX, int startY);
void GenerateMazes(int grid[GRID_HEIGHT][GRID_WIDTH]);

#endif //DUNGEON_H
