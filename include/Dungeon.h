#ifndef DUNGEON_H
#define DUNGEON_H

// Cell Types
#define CELL_EMPTY_1 0 // Empty
#define CELL_EMPTY_2 1 // Empty 2 (currently just for pattern)
#define CELL_ROOM 2
#define CELL_CORRIDOR 3
#define CELL_DOOR 4
#define CELL_PATH 5
#define ROOM_ID_START 10 // ID 10 and Up

// General
// The Height and Width should be an ODD number
#define GRID_HEIGHT 79
#define GRID_WIDTH 79
#define CELL_SIZE 13
#define GRID_TOTAL_HEIGHT (GRID_HEIGHT * CELL_SIZE)
#define GRID_TOTAL_WIDTH (GRID_WIDTH * CELL_SIZE)

// Optimization Macros
#define HALF(x) ((x) >> 1)
#define CENTER_SCREEN_X(width) ((GetScreenWidth() - (width)) >> 1)
#define CENTER_SCREEN_Y(height) ((GetScreenHeight() - (height)) >> 1)
#define IS_IN_GRID(x, y) ((x) >= 0 && (x) < GRID_WIDTH && (y) >= 0 && (y) < GRID_HEIGHT)
#define IS_ROOM(cell) ((cell) >= ROOM_ID_START)
#define IS_EMPTY(cell) ((cell) == CELL_EMPTY_1 || (cell) == CELL_EMPTY_2)

// Rooms
#define ROOM_MAX_SIZE 16
#define ROOM_MIN_SIZE 9
#define ROOM_MIN_WIDTH 7
#define ROOM_MIN_HEIGHT 7
#define ROOM_AMOUNT 12
#define ROOM_BOUNDARY_PADDING 4
#define ROOM_SPACING 3

// Room Size Tiers
#define ROOM_SIZE_LARGE_MIN 75  // 75-100%
#define ROOM_SIZE_MEDIUM_MIN 50 // 50-75%
#define ROOM_SIZE_SMALL_MIN 25  // 25-50%

#define ROOM_WIDTH_MIN_BOUND 0
#define ROOM_WIDTH_MAX_BOUND (GRID_WIDTH - ROOM_MAX_SIZE)
#define ROOM_HEIGHT_MIN_BOUND 0
#define ROOM_HEIGHT_MAX_BOUND (GRID_HEIGHT - ROOM_MAX_SIZE)

typedef struct
{
    int x;
    int y;
    int width;
    int height;
} Room;

Room CreateRoom(int x, int y, int width, int height);
bool IsRoomValid(int grid[GRID_HEIGHT][GRID_WIDTH], Room room);
void PlaceRoom(int grid[GRID_HEIGHT][GRID_WIDTH], Room room, int roomId);
bool GenerateRooms(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int* roomCount);

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

// Doors
#define DOOR_NEXT_CHANCE_INITIAL 100
#define DOOR_CHANCE_DECREASE 15

bool ConnectRoomsViaDoors(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int roomCount);
bool FindDoorPosition(int grid[GRID_HEIGHT][GRID_WIDTH], Room room, int* doorX, int* doorY);

// Dungeon Generation
bool GenerateDungeon(int grid[GRID_HEIGHT][GRID_WIDTH], int maxAttempts);
void GenerateGrid(int grid[GRID_HEIGHT][GRID_WIDTH]);
void PrintDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);

#endif //DUNGEON_H