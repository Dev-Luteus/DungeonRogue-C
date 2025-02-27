#include "Staircase.h"
#include "Room.h"
#include <stdio.h>

void PlaceStaircases(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int roomCount, int currentFloor)
{
    // Use existing functions to find start and boss rooms
    Room startRoom = FindStartingRoom(rooms, roomCount);
    Room bossRoom = FindBossRoom(rooms, roomCount);

    int startRoomIndex = -1;
    int bossRoomIndex = -1;

    // Find matching indices in our rooms array
    for (int i = 0; i < roomCount; i++)
    {
        if (rooms[i].x == startRoom.x && rooms[i].y == startRoom.y &&
            rooms[i].width == startRoom.width && rooms[i].height == startRoom.height)
        {
            startRoomIndex = i;
            rooms[i].type = ROOM_TYPE_START;  // Mark as starting room
        }

        if (rooms[i].x == bossRoom.x && rooms[i].y == bossRoom.y &&
            rooms[i].width == bossRoom.width && rooms[i].height == bossRoom.height)
        {
            bossRoomIndex = i;
            rooms[i].type = ROOM_TYPE_BOSS;   // Mark as boss room
        }
    }

    if (startRoomIndex == -1 || bossRoomIndex == -1)
    {
        printf("Error: Failed to find start or boss room indices!\n");
        return;
    }
    
    // Get center positions for both rooms
    int startX, startY, bossX, bossY;
    GetRoomCenter(rooms[startRoomIndex], &startX, &startY);
    GetRoomCenter(rooms[bossRoomIndex], &bossX, &bossY);
    
    // Place the down staircase in boss room
    grid[bossY][bossX] = CELL_STAIR_DOWN;
    printf("Placed down staircase at (%d, %d) in boss room\n", bossX, bossY);
    
    // Place up staircase in start room, but only if not on first floor
    if (currentFloor > 1)
    {
        grid[startY][startX] = CELL_STAIR_UP;
        printf("Placed up staircase at (%d, %d) in start room on floor %d\n", 
               startX, startY, currentFloor);
    }
    else
    {
        printf("No up staircase placed on first floor\n");
    }
}