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
#define GRID_TOTAL_HEIGHT GRID_HEIGHT * CELL_SIZE
#define GRID_TOTAL_WIDTH GRID_WIDTH * CELL_SIZE

void GenerateGrid(int grid[GRID_HEIGHT][GRID_WIDTH]);
void GenerateDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);
void PrintDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);

// Rooms
#define ROOM_MAX_SIZE 18
#define ROOM_MIN_SIZE 9
#define ROOM_MIN_WIDTH 7
#define ROOM_MIN_HEIGHT 7
#define ROOM_AMOUNT 14

#define ROOM_BOUNDARY_PADDING 4
#define ROOM_SPACING 3

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
void GenerateRooms(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int* roomCount);

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

void ConnectRoomsViaDoors(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int roomCount);

// Paths
bool FindDoorPosition(int grid[GRID_HEIGHT][GRID_WIDTH], Room room, int* doorX, int* doorY);

#endif //DUNGEON_H
