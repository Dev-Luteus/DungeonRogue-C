#include <raylib.h>
#include "Dungeon.h"
#include "Door.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/* Here, we attempt to create connections between rooms and corridors,
 * First we allocate memory for a boolean array to keep track of connections,
 * Then, we iterate over each room, and check if we can connect it to a corridor,
 *
 * We then introduce randomness to door placement, and check if we can place a door,
 * If we can, we place a door, and mark the room as connected.
 * If we cannot, we try to place a door in a random direction, followed by a corridor cell.
 *
 */
bool ConnectRoomsViaDoors(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int roomCount)
{
    // calloc => runtime heap allocation, initializes 0 (false for bool)
    bool* hasConnection = (bool*)calloc(roomCount, sizeof(bool));

    if (hasConnection == NULL)
    {
        return false; // Allocation failed!
    }

    // Wall offsets for [checkX, checkY, doorX, doorY] in N-S-W-E order
    const int8_t x = 2; // corridor-room spacing amount!
    const int8_t d = 1;  // Door distance from room!

    const int8_t walls[][4] = {
        {0, -x, 0, -d},    // N
        {0, x, 0, d},      // S
        {-x, 0, -d, 0},    // W
        {x, 0, d, 0}       // E
    };

    // Grid center points for directional biasing
    const int centerX = GRID_WIDTH / 2;
    const int centerY = GRID_HEIGHT / 2;
    const int topThird = GRID_HEIGHT / 3;

    for (int roomIndex = 0; roomIndex < roomCount; ++roomIndex)
    {
        // Prevent multiple connections
        if (hasConnection[roomIndex])
        {
            continue;
        }

        // Get current room
        Room room = rooms[roomIndex];
        bool doorPlaced = false;

        // Calculate room center for directional bias
        int roomCenterX = room.x + (room.width / 2);
        int roomCenterY = room.y + (room.height / 2);

        /* Direction biasing based on room position:
         * Remember: Y-axis is inverted in computer graphics (0 at top, increases downward)
         * If room is above center (Y < centerY) => Start with North (0) because it's closer to top
         * If room is below center (Y > centerY) => Start with South (1) because it's closer to bottom
         * For X axis: Prefer East if room is left of center, West if right of center
         */
        int startWall = (roomCenterY < centerY) ? 0 : 1;  // North : South
        int wallOrder[4] = {startWall, (startWall + 1) % 2};  // First try N/S pair based on Y position

        // Then add E/W based on X position
        wallOrder[2] = (roomCenterX < centerX) ? 3 : 2;  // East : West
        wallOrder[3] = (wallOrder[2] == 2) ? 3 : 2;      // Complete the pair

        // Check if room is in the top third of the grid for south preference
        bool preferSouth = room.y < topThird;

        for (int wallIndex = 0; wallIndex < 4 && !doorPlaced; wallIndex++)
        {
            int wall = wallOrder[wallIndex];
            /* wall = 0 (North) or wall = 1 (South) -> horizontal movement ( <---> )
             * wall = 2 (West) or wall = 3 (East) -> vertical movement ( | )
             */
            bool isHorizontal = wall < 2;

            int start = isHorizontal ? room.x : room.y;
            int length = isHorizontal ? room.width : room.height;

            for (int pos = 0; pos < length && !doorPlaced; pos++)
            {
                // Clever way to get X/Y based on Horizontal/Vertical
                int x = isHorizontal ? (room.x + pos) : room.x;
                int y = isHorizontal ? room.y : (room.y + pos);

                /* 2D Array [(Y: Row)], [(X: Column)], ex:
                 *
                 * When we check the north wall: {0, -2, 0, -1},
                 * walls[0][0] = 0     // X offset
                 * walls[0][1] = -2    // Y offset
                 *
                 * So, we check the grid cell that's offset from our current (x,y) position,
                 * by the amounts stored in the walls array
                 */

                // First, try extended corridor placement for rooms near the top
                if (preferSouth && wall == 1)  // South wall for top rooms
                {
                    int doorX = x + walls[wall][2];
                    int doorY = y + walls[wall][3];
                    int corridorX = x + walls[wall][0];
                    int corridorY = y + walls[wall][1];
                    int extraCorridorY = corridorY + 1;

                    if (IS_IN_GRID(doorX, doorY) &&
                        IS_IN_GRID(corridorX, corridorY) &&
                        IS_IN_GRID(corridorX, extraCorridorY))
                    {
                        if ((grid[doorY][doorX] == CELL_EMPTY_1 || grid[doorY][doorX] == CELL_EMPTY_2) &&
                            (grid[corridorY][corridorX] == CELL_EMPTY_1 || grid[corridorY][corridorX] == CELL_EMPTY_2) &&
                            (grid[extraCorridorY][corridorX] == CELL_CORRIDOR ||
                             grid[extraCorridorY][corridorX] == CELL_EMPTY_1 ||
                             grid[extraCorridorY][corridorX] == CELL_EMPTY_2))
                        {
                            grid[doorY][doorX] = CELL_DOOR;
                            grid[corridorY][corridorX] = CELL_CORRIDOR;
                            if (grid[extraCorridorY][corridorX] != CELL_CORRIDOR)
                            {
                                grid[extraCorridorY][corridorX] = CELL_CORRIDOR;
                            }
                            hasConnection[roomIndex] = true;
                            doorPlaced = true;
                            continue;
                        }
                    }
                }

                // Regular corridor check if extended placement failed
                if (grid[y + walls[wall][1]][x + walls[wall][0]] == CELL_CORRIDOR)
                {
                    // Force check of the next cell first
                    int nextX = x + (isHorizontal ? 1 : 0);
                    int nextY = y + (isHorizontal ? 0 : 1);

                    // If next cell is valid and also a corridor, use it as starting point
                    if (pos + 1 < length &&
                        grid[nextY + walls[wall][1]][nextX + walls[wall][0]] == CELL_CORRIDOR)
                    {
                        x = nextX;
                        y = nextY;
                        pos++; // Update position counter
                    }

                    // Now start the chance-based scanning from this position
                    int chance = DOOR_NEXT_CHANCE_INITIAL;
                    int finalX = x;
                    int finalY = y;

                    while (pos + 1 < length && GetRandomValue(0, 100) < chance)
                    {
                        nextX = finalX + (isHorizontal ? 1 : 0);
                        nextY = finalY + (isHorizontal ? 0 : 1);

                        if (grid[nextY + walls[wall][1]][nextX + walls[wall][0]] == CELL_CORRIDOR)
                        {
                            finalX = nextX;
                            finalY = nextY;
                            chance -= DOOR_CHANCE_DECREASE;
                        }
                        else
                        {
                            break;
                        }
                    }

                    grid[finalY + walls[wall][3]][finalX + walls[wall][2]] = CELL_DOOR;
                    hasConnection[roomIndex] = true;
                    doorPlaced = true;
                }
            }
        }

        /* Safety check: If no corridor was found within normal range,
         * create a new connection in a random direction
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

            // Try each direction
            for (int dirIndex = 0; dirIndex < 4 && !doorPlaced; dirIndex++)
            {
                int wall = directions[dirIndex];
                bool isHorizontal = wall < 2;

                // Pick random position along the wall
                int length = isHorizontal ? room.width : room.height;
                int pos = GetRandomValue(0, length - 1);

                int x = isHorizontal ? (room.x + pos) : room.x;
                int y = isHorizontal ? room.y : (room.y + pos);

                int doorX = x + walls[wall][2];
                int doorY = y + walls[wall][3];
                int corridorX = x + walls[wall][0];
                int corridorY = y + walls[wall][1];

                if (doorX >= 1 && doorX < GRID_WIDTH - 1 &&
                    doorY >= 1 && doorY < GRID_HEIGHT - 1 &&
                    corridorX >= 1 && corridorX < GRID_WIDTH - 1 &&
                    corridorY >= 1 && corridorY < GRID_HEIGHT - 1)
                {
                    if ((grid[doorY][doorX] == CELL_EMPTY_1 || grid[doorY][doorX] == CELL_EMPTY_2) &&
                        (grid[corridorY][corridorX] == CELL_EMPTY_1 || grid[corridorY][corridorX] == CELL_EMPTY_2))
                    {
                        grid[doorY][doorX] = CELL_DOOR;
                        grid[corridorY][corridorX] = CELL_CORRIDOR;
                        hasConnection[roomIndex] = true;
                        doorPlaced = true;
                    }
                }
            }
        }
    }

    // Check if all rooms are connected before freeing memory!
    bool allConnected = true;
    for (int i = 0; i < roomCount; i++)
    {
        if (!hasConnection[i])
        {
            allConnected = false;
            break;
        }
    }

    free(hasConnection);
    return allConnected;
}

// Here, we iterate over the room perimeter and find the door position!
bool FindDoorPosition(int grid[GRID_HEIGHT][GRID_WIDTH], Room room, int* doorX, int* doorY)
{
    for (int y = room.y - 1; y <= room.y + room.height; y++)
    {
        for (int x = room.x - 1; x <= room.x + room.width; x++)
        {
            if (IS_IN_GRID(x, y) && grid[y][x] == CELL_DOOR)
            {
                *doorX = x;
                *doorY = y;

                return true;
            }
        }
    }

    // should never happen but just in case
    printf("No door found for room at (%d, %d)!\n", room.x, room.y);
    return false;
}