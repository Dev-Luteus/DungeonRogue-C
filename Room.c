#include <raylib.h>
#include "Dungeon.h"
#include "Room.h"
#include <stdlib.h>

Room CreateRoom(int x, int y, int width, int height)
{
    Room room =
    {
        x,
        y,
        width,
        height,
        ROOM_TYPE_NORMAL
    };

    return room;
}

/* Here, we're checking if our room is within valid bounds!
 * We first check if our room is within the grid,
 * Then, we check if rooms overlap or are too close (which we don't want)!
 *
 * Then, we check if the room is within the grid area,
 * and if rooms overlap or are within the boundaries of 2 cells (spacing),
 * if they're too close, we return false!
 * This allows us to place rooms until we find a valid one!
 */
bool IsRoomValid(int grid[GRID_HEIGHT][GRID_WIDTH], Room room)
{
    if (room.x < ROOM_BOUNDARY_PADDING || room.y < ROOM_BOUNDARY_PADDING ||
        room.x + room.width >= GRID_WIDTH - ROOM_BOUNDARY_PADDING ||
        room.y + room.height >= GRID_HEIGHT - ROOM_BOUNDARY_PADDING)
    {
        return false;
    }

    const int startY = room.y - ROOM_SPACING;
    const int endY = room.y + room.height + ROOM_SPACING;
    const int startX = room.x - ROOM_SPACING;
    const int endX = room.x + room.width + ROOM_SPACING;

    for (int y = startY; y < endY; y++)
    {
        if (IS_IN_GRID(0, y))
        {
            for (int x = startX; x < endX; x++)
            {
                if (IS_IN_GRID(x, 0) && IS_ROOM(grid[y][x]))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

/* This is where we attempt placing rooms in our grid,
 * By iterating over every cell in the room and marking it */
void PlaceRoom(int grid[GRID_HEIGHT][GRID_WIDTH], Room room, int roomId)
{
    for (int y = room.y; y < room.y + room.height; y++)
    {
        for (int x = room.x; x < room.x + room.width; x++)
        {
            grid[y][x] = roomId; // Mark as room cell
        }
    }
}

static int CalculateRoomSize(int minPercent, int maxPercent, int minSize, int maxSize)
{
    // I had to look this up but apparently dividing by 128 is faster than dividing by 100,
    // Using bit shifts for division by powers of 2, is the idea
    int range = maxSize - minSize;
    int minValue = minSize + ((range * minPercent) >> 7);  // Divide by 128 (~100) (100%)
    int maxValue = minSize + ((range * maxPercent) >> 7);

    return GetRandomValue(minValue, maxValue);
}

bool GenerateRooms(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int* roomCount)
{
    *roomCount = 0;
    int nextRoomId = ROOM_ID_START;
    const int MAX_ATTEMPTS = 50;
    const int ATTEMPTS_PER_ROOM = 20;
    int failedAttempts = 0;

    /* Here, I'm trying to make Room Placement more successful, by dividing rooms into tiers,
     * So we can control how many rooms of which sizes are placed!
     */
    while (*roomCount < ROOM_AMOUNT && failedAttempts < MAX_ATTEMPTS)
    {
        int width, height;

        // Use bit shifts for division by powers of 2
        const int quarterRooms = ROOM_AMOUNT >> 2;  // ROOM_AMOUNT / 4
        const int halfRooms = ROOM_AMOUNT >> 1;     // ROOM_AMOUNT / 2

        // Determine room size tier based on count
        if (*roomCount < quarterRooms)  // First 25% of rooms
        {
            width = CalculateRoomSize(96, 128, ROOM_MIN_WIDTH, ROOM_MAX_SIZE);   // ~75-100%
            height = CalculateRoomSize(96, 128, ROOM_MIN_HEIGHT, ROOM_MAX_SIZE);
        }
        else if (*roomCount < halfRooms)  // Next 25% of rooms
        {
            width = CalculateRoomSize(64, 96, ROOM_MIN_WIDTH, ROOM_MAX_SIZE);    // ~50-75%
            height = CalculateRoomSize(64, 96, ROOM_MIN_HEIGHT, ROOM_MAX_SIZE);
        }
        else  // Remaining 50% of rooms
        {
            width = CalculateRoomSize(32, 64, ROOM_MIN_WIDTH, ROOM_MAX_SIZE);    // ~25-50%
            height = CalculateRoomSize(32, 64, ROOM_MIN_HEIGHT, ROOM_MAX_SIZE);
        }

        bool roomPlaced = false;

        // Try to place the room
        for (int attempt = 0; attempt < ATTEMPTS_PER_ROOM && !roomPlaced; attempt++)
        {
            int x = GetRandomValue(ROOM_WIDTH_MIN_BOUND, ROOM_WIDTH_MAX_BOUND);
            int y = GetRandomValue(ROOM_HEIGHT_MIN_BOUND, ROOM_HEIGHT_MAX_BOUND);

            Room room = CreateRoom(x, y, width, height);

            if (IsRoomValid(grid, room))
            {
                PlaceRoom(grid, room, nextRoomId);
                rooms[*roomCount] = room;
                (*roomCount)++;
                nextRoomId++;
                roomPlaced = true;
                failedAttempts = 0;  // Reset failed attempts on success
            }
        }

        failedAttempts += !roomPlaced;  // Increment if room wasn't placed (using bool to int conversion)
    }

    return *roomCount == ROOM_AMOUNT;
}

// Helper function to return the Manhattan distance between two rooms
static int GetRoomDistance(Room a, Room b)
{
    /* This used to use division, but I learned that Bit Shifts are faster
     * Makes sense, we can prevent a more expensive calculation!
     */
    int centerAX = a.x + (a.width >> 1);
    int centerAY = a.y + (a.height >> 1);
    int centerBX = b.x + (b.width >> 1);
    int centerBY = b.y + (b.height >> 1);

    return abs(centerAX - centerBX) + abs(centerAY - centerBY);
}

// Find the room with smallest area (starting room)
Room FindStartingRoom(Room rooms[], int roomCount)
{
    int smallestIndex = 0;
    int smallestArea = rooms[0].width * rooms[0].height;

    for (int i = 1; i < roomCount; i++)
    {
        int area = rooms[i].width * rooms[i].height;

        if (area < smallestArea)
        {
            smallestArea = area;
            smallestIndex = i;
        }
    }

    return rooms[smallestIndex];
}

// Find the room with largest area (boss room)
Room FindBossRoom(Room rooms[], int roomCount)
{
    int largestIndex = 0;
    int largestArea = rooms[0].width * rooms[0].height;

    for (int i = 1; i < roomCount; i++)
    {
        int area = rooms[i].width * rooms[i].height;

        if (area > largestArea)
        {
            largestArea = area;
            largestIndex = i;
        }
    }

    return rooms[largestIndex];
}

// Get the center position of a room
void GetRoomCenter(Room room, int* centerX, int* centerY)
{
    *centerX = room.x + (room.width / 2);
    *centerY = room.y + (room.height / 2);
}