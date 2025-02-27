#ifndef ROOM_H
#define ROOM_H

#include <stdbool.h>
#include "DungeonDefs.h"

// Room Size Constants
#define ROOM_MAX_SIZE 12
#define ROOM_MIN_SIZE 5
#define ROOM_MIN_WIDTH 5
#define ROOM_MIN_HEIGHT 5
#define ROOM_AMOUNT 16
#define ROOM_BOUNDARY_PADDING 4
#define ROOM_SPACING 4

// Room Size Tiers
#define ROOM_SIZE_LARGE_MIN 75  // 75-100%
#define ROOM_SIZE_MEDIUM_MIN 50 // 50-75%
#define ROOM_SIZE_SMALL_MIN 25  // 25-50%

#define ROOM_WIDTH_MIN_BOUND 0
#define ROOM_WIDTH_MAX_BOUND (GRID_WIDTH - ROOM_MAX_SIZE)
#define ROOM_HEIGHT_MIN_BOUND 0
#define ROOM_HEIGHT_MAX_BOUND (GRID_HEIGHT - ROOM_MAX_SIZE)

// Complete Room struct definition
typedef struct Room {
    int x;
    int y;
    int width;
    int height;
    int type;
} Room;

// Room generation and management
Room CreateRoom(int x, int y, int width, int height);
bool IsRoomValid(int grid[GRID_HEIGHT][GRID_WIDTH], Room room);
void PlaceRoom(int grid[GRID_HEIGHT][GRID_WIDTH], Room room, int roomId);
bool GenerateRooms(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int* roomCount);

// Room finding functions
Room FindStartingRoom(Room rooms[], int roomCount);
Room FindBossRoom(Room rooms[], int roomCount);

// Helper function to get room center
void GetRoomCenter(Room room, int* centerX, int* centerY);

#endif // ROOM_H