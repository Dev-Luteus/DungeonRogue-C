#include <raylib.h>
#include "Dungeon.h"
#include "Door.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/* Here, we attempt to create connections between rooms and corridors,
 * First we allocate memory for a boolean array to keep track of connections,
 * Then, we iterate over each room, and check if we can connect it.
 *
 * We then introduce randomness to door placement with some direction bias,
 * and check if we can place a door!
 *
 * If we can, we place a door and mark the room as connected.
 * If we cannot, we try to place a door in a random direction, followed by necessary corridor cell.
 *
 * The goal is to ensure all rooms are connected by doors and corridors,
 * and that the player can then traverse to each and all rooms!
 */
bool ConnectRoomsViaDoors(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int roomCount)
{
    // calloc => runtime heap allocation, initializes 0 (false for bool)
    bool* hasConnection = (bool*)calloc(roomCount, sizeof(bool));

    if (hasConnection == NULL)
    {
        return false; // Allocation failed!
    }

    /* Direction offsets for [checkX, checkY, doorX, doorY] for N, S, W, E directions
     * North: (0, -x, 0, -1)
     * South: (0, +x, 0, +1)
     * West: (-x, 0, -1, 0)
     * East: (+x, 0, +1, 0)
     *
     * x = cells away to check for corridors (1, 2, or 3 cells away)
     * We also place our door to be adjacent to the room!
     */
    const int8_t d = 1;  // Door distance from room is always 1

    // Grid center for directional biasing
    const int centerY = GRID_HEIGHT / 2;

    // Maximum corridor search distance
    const int MAX_CORRIDOR_DISTANCE = 3;

    for (int roomIndex = 0; roomIndex < roomCount; ++roomIndex)
    {
        // Skip rooms that already have connections
        if (hasConnection[roomIndex])
        {
            continue;
        }

        // Get current room
        Room room = rooms[roomIndex];
        bool doorPlaced = false;

        // Calculate room center for directional bias
        int roomCenterY = room.y + (room.height / 2);

        /* Set wall order based on room position!
         * - Rooms in top half: SWEN (South, West, East, North)
         * - Rooms in bottom half: NWES (North, West, East, South)
         *
         * The goal here is to make rooms more easily traversed to!
         * We bias door placement towards the center of the dungeon!
         */
        int wallOrder[4];

        if (roomCenterY < centerY)  // Top
        {
            wallOrder[0] = 1;  // South
            wallOrder[1] = 2;  // West
            wallOrder[2] = 3;  // East
            wallOrder[3] = 0;  // North
        }
        else
        {
            wallOrder[0] = 0;  // North
            wallOrder[1] = 2;  // West
            wallOrder[2] = 3;  // East
            wallOrder[3] = 1;  // South
        }

        // Progressive scanning with increasing distances
        for (int corridorDistance = 1; corridorDistance <= MAX_CORRIDOR_DISTANCE && !doorPlaced; corridorDistance++)
        {
            // Each wall in order!
            for (int i = 0; i < 4 && !doorPlaced; i++)
            {
                int wall = wallOrder[i];
                bool isHorizontal = wall < 2;  // N/S are horizontal walls, W/E are vertical

                // Wall offsets based on current distance
                const int8_t wallOffsets[4][4] =
                {
                    {0, -corridorDistance, 0, -d},    // N
                    {0, corridorDistance, 0, d},      // S
                    {-corridorDistance, 0, -d, 0},    // W
                    {corridorDistance, 0, d, 0}       // E
                };

                int length = isHorizontal ? room.width : room.height;

                /* Changed door scanning pattern: Start from middle of wall and alternate sides
                 * This avoids biasing doors toward room corners and creates more
                 * aesthetically pleasing door placements
                 */
                int middle = length / 2;

                // Check positions outward from the middle of the wall in alternating pattern
                // middle, middle+1, middle-1, middle+2, middle-2, etc.
                for (int offset = 0; offset < length && !doorPlaced; offset++)
                {
                    // Calculate position with alternating offset from middle
                    int pos;
                    if (offset == 0) {
                        pos = middle;  // Start at exact middle
                    } else {
                        // Alternate between right and left of middle
                        int direction = ((offset % 2) == 1) ? 1 : -1;
                        pos = middle + ((offset + 1) / 2) * direction;
                    }

                    // Skip invalid positions
                    if (pos < 0 || pos >= length) {
                        continue;
                    }

                    int x = isHorizontal ? (room.x + pos) : room.x;
                    int y = isHorizontal ? room.y : (room.y + pos);

                    // Calculate corridor and door positions
                    int corridorX = x + wallOffsets[wall][0];
                    int corridorY = y + wallOffsets[wall][1];
                    int doorX = x + wallOffsets[wall][2];
                    int doorY = y + wallOffsets[wall][3];

                    // Check if corridor position is valid and contains a corridor
                    if (IS_IN_GRID(corridorX, corridorY) && grid[corridorY][corridorX] == CELL_CORRIDOR)
                    {
                        if (IS_IN_GRID(doorX, doorY) &&
                            (grid[doorY][doorX] == CELL_EMPTY_1 || grid[doorY][doorX] == CELL_EMPTY_2))
                        {
                            // Place door
                            grid[doorY][doorX] = CELL_DOOR;

                            // Here, we add connecting corridors if necessary!
                            if (corridorDistance > 1)
                            {
                                // ? : because, sometimes corridors 3 cells away aren't properly connected ^^ 
                                int dirX = (corridorX == doorX) ? 0 : ((corridorX > doorX) ? 1 : -1);
                                int dirY = (corridorY == doorY) ? 0 : ((corridorY > doorY) ? 1 : -1);

                                // Fill corridor cells
                                for (int step = 1; step < corridorDistance; step++)
                                {
                                    int fillX = doorX + dirX * step;
                                    int fillY = doorY + dirY * step;

                                    if (IS_IN_GRID(fillX, fillY) &&
                                        (grid[fillY][fillX] == CELL_EMPTY_1 ||
                                            grid[fillY][fillX] == CELL_EMPTY_2))
                                    {
                                        grid[fillY][fillX] = CELL_CORRIDOR;
                                    }
                                }
                            }

                            hasConnection[roomIndex] = true;
                            doorPlaced = true;
                            break;
                        }
                    }
                }
            }
        }

        /* In the case that no corridors were found within expected range,
         * we must create a new connection in a random direction!
         */
        if (!doorPlaced)
        {
            int directions[4] = {0, 1, 2, 3};  // N, S, W, E

            // Here, I use a Fisher-Yates shuffle for a random direction order
            for (int i = 3; i > 0; i--)
            {
                int j = GetRandomValue(0, i);
                int temp = directions[i];

                directions[i] = directions[j];
                directions[j] = temp;
            }

            // Try multiple positions in each direction
            for (int dirIndex = 0; dirIndex < 4 && !doorPlaced; dirIndex++)
            {
                int wall = directions[dirIndex];
                bool isHorizontal = wall < 2;

                // Initialize wall offsets for distance 3
                // This gives more space to place corridors
                int corridorDistance = 3;

                const int8_t wallOffsets[4][4] =
                {
                    {0, -corridorDistance, 0, -d},    // N
                    {0, corridorDistance, 0, d},      // S
                    {-corridorDistance, 0, -d, 0},    // W
                    {corridorDistance, 0, d, 0}       // E
                };

                int length = isHorizontal ? room.width : room.height;
                int middle = length / 2;  // Start from middle position

                // Try middle first, then 1/4 and 3/4 positions if middle fails
                int offsets[] = {0, length / 4, -length / 4};
                int numOffsets = 3;

                for (int offsetIdx = 0; offsetIdx < numOffsets && !doorPlaced; offsetIdx++)
                {
                    int pos = middle + offsets[offsetIdx];

                    // Safety check for valid position
                    if (pos < 0) pos = 0;
                    if (pos >= length) pos = length - 1;

                    int x = isHorizontal ? (room.x + pos) : room.x;
                    int y = isHorizontal ? room.y : (room.y + pos);

                    int doorX = x + wallOffsets[wall][2];
                    int doorY = y + wallOffsets[wall][3];
                    int corridorX = x + wallOffsets[wall][0];
                    int corridorY = y + wallOffsets[wall][1];

                    // Check if door and corridor positions are valid
                    if (IS_IN_GRID(doorX, doorY) && IS_IN_GRID(corridorX, corridorY) &&
                        !IS_ROOM(grid[doorY][doorX]) && !IS_ROOM(grid[corridorY][corridorX]))
                    {
                        // Place door
                        grid[doorY][doorX] = CELL_DOOR;

                        // Place corridor cells between door and final corridor position
                        int dirX = (corridorX - doorX) / corridorDistance;
                        int dirY = (corridorY - doorY) / corridorDistance;

                        for (int step = 1; step <= corridorDistance; step++)
                        {
                            int fillX = doorX + dirX * step;
                            int fillY = doorY + dirY * step;

                            if (IS_IN_GRID(fillX, fillY) && !IS_ROOM(grid[fillY][fillX]))
                            {
                                grid[fillY][fillX] = CELL_CORRIDOR;
                            }
                        }

                        hasConnection[roomIndex] = true;
                        doorPlaced = true;
                    }
                }
            }
        }

        // Report failure if we still couldn't place a door
        if (!doorPlaced)
        {
            printf("Failed to place door for room %d at position (%d,%d)\n",
                   roomIndex, room.x, room.y);
        }
    }

    // Check if all rooms are connected before freeing memory!
    bool allConnected = true;

    for (int i = 0; i < roomCount; i++)
    {
        if (!hasConnection[i])
        {
            allConnected = false;
            printf("Room %d at (%d,%d) has no connection\n", i, rooms[i].x, rooms[i].y);
            break;
        }
    }

    free(hasConnection);
    return allConnected;
}

/* This function locates the door cell associated with a specific room.
 *
 * It is primarily used by the Path module during pathfinding to determine
 * the starting and ending points for paths between rooms.
 * When connecting rooms in our dungeon, we need to know where the doors are to create proper
 * pathways from one room's door to another!
 *
 * We do this by:
 * 1. First scanning around the room's perimeter,
 * 2. Identifying our CELL_DOOR,
 * 3. We then verify that the door found is adjacent to the room ( safety check just in case ),
 * 4. Then finally, we return the coordinates of the door!
 */
bool FindDoorPosition(int grid[GRID_HEIGHT][GRID_WIDTH], Room room, int* doorX, int* doorY)
{
    /* Since doors are always adjacent to rooms, we only need to check
     * one cell beyond each wall of the room. This is more efficient than
     * scanning a larger radius around the room.
     */
    const int SEARCH_RADIUS = 1;  // Reduced from 2 to 1 as doors must be adjacent to rooms

    for (int y = room.y - SEARCH_RADIUS; y <= room.y + room.height + SEARCH_RADIUS; y++)
    {
        for (int x = room.x - SEARCH_RADIUS; x <= room.x + room.width + SEARCH_RADIUS; x++)
        {
            if (IS_IN_GRID(x, y) && grid[y][x] == CELL_DOOR)
            {
                // Check if this door is for our room (is it adjacent to this room?)
                bool doorForThisRoom = false;

                // Check if door is adjacent to any room cell
                for (int dy = -1; dy <= 1 && !doorForThisRoom; dy++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        if (dx == 0 && dy == 0)
                        {
                            continue;
                        }

                        int checkX = x + dx;
                        int checkY = y + dy;

                        // Is this adjacent cell part of our room?
                        if (IS_IN_GRID(checkX, checkY) &&
                            checkX >= room.x && checkX < room.x + room.width &&
                            checkY >= room.y && checkY < room.y + room.height)
                        {
                            doorForThisRoom = true;
                            break;
                        }
                    }
                }

                if (doorForThisRoom)
                {
                    *doorX = x;
                    *doorY = y;

                    return true;
                }
            }
        }
    }

    // should never happen but just in case
    printf("No door found for room at (%d, %d)!\n", room.x, room.y);
    return false;
}