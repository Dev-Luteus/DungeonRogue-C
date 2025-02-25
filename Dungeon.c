#include <raylib.h>
#include "Dungeon.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "GeneratePaths.h"

/* In this loop we make a simple 2d grid
 * We then colour the grid based on the XOR AND of x and y */
void GenerateGrid(int grid[GRID_HEIGHT][GRID_WIDTH])
{
    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            // XOR, Compare x / y
            // & 1 => If both bits are 1, result is 1! Else 0!
            grid[y][x] = (x ^ y) & 1;
        }
    }
}

Room CreateRoom (int x, int y, int width, int height)
{
    Room room =
    {
        x,
        y,
        width,
        height
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

    for (int y = room.y - ROOM_SPACING; y < room.y + room.height + ROOM_SPACING; y++)
    {
        for (int x = room.x - ROOM_SPACING; x < room.x + room.width + ROOM_SPACING; x++)
        {
            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT)
            {
                if (grid[y][x] >= ROOM_ID_START)
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

void GenerateRooms(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int* roomCount)
{
    *roomCount = 0;
    int nextRoomId = ROOM_ID_START;
    const int MAX_ATTEMPTS = 100; // per room

    for (int i = 0; i < ROOM_AMOUNT; i++)
    {
        bool roomPlaced = false;

        for (int attempt = 0; attempt < MAX_ATTEMPTS && !roomPlaced; attempt++)
        {
            int width = GetRandomValue(ROOM_MIN_WIDTH, ROOM_MAX_SIZE);
            int height = GetRandomValue(ROOM_MIN_HEIGHT, ROOM_MAX_SIZE);
            int x = GetRandomValue(0, GRID_WIDTH - width);
            int y = GetRandomValue(0, GRID_HEIGHT - height);

            Room room = CreateRoom(x, y, width, height);

            if (IsRoomValid(grid, room))
            {
                PlaceRoom(grid, room, nextRoomId);
                rooms[*roomCount] = room;
                (*roomCount)++;
                nextRoomId++;
                roomPlaced = true;
            }
        }
    }
}

/* This is our general check if we can generate corridors on cells in the grid!
 * We first check the grid boundaries, then,
 * We check the if the cell is empty ( not room or corridor ),
 */
bool IsValidCorridorCell(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y)
{
    if (x < 1 || y < 1 || x >= GRID_WIDTH - 1 || y >= GRID_HEIGHT - 1)
    {
        return false;
    }

    // Only empty cells are valid!
    if (grid[y][x] != CELL_EMPTY_1 && grid[y][x] != CELL_EMPTY_2)
    {
        return false;
    }

    // Here, we count the adjacent corridors (including diagonals)
    int corridorCount = 0;

    // (-1, -1) to (1, 1) ( 3 x 3)
    for (int cy = -1; cy <= 1; cy++)
    {
        for (int cx = -1; cx <= 1; cx++)
        {
            if (cx == 0 && cy == 0)
            {
                continue;
            }

            int newX = x + cx;
            int newY = y + cy;

            if (newX >= 0 && newX < GRID_WIDTH && newY >= 0 && newY < GRID_HEIGHT)
            {
                // Checking diagonals with using the Manhattan distance formula!
                // = abs(x1 - x2) + abs(y1 - y2)

                int xDist = abs(cx);
                int yDist = abs(cy);

                if (grid[newY][newX] >= ROOM_ID_START && ((xDist == 1 && yDist == 1) || (xDist + yDist < 2)))
                {
                    return false; // Too close to room corner or room
                }

                if (grid[newY][newX] == CELL_CORRIDOR)
                {
                    return false; // Maintain 1 cell spacing from corridors
                }
            }
        }
    }

    // Here, I'm trying to prevent loops by checking if the corridor count is less than or equal to 1
    return corridorCount <= 1;
}


/* This is my attempt at a randomized flood-fill algorithm.
 * The idea is to generate mazes in the empty available cells, which will act as corridors,
 * We can then use this data to find connections, and remove excess corridors.
 */

/* Normally you'd think about the origin being at (0,0), but in computer graphics,
* The origin is often in the top left corner. Hence, Y has to be negative to go up!
* So we essentially flip the Y axis.
* North: (0, -1), East: (1, 0), South: (0, 1), West: (-1, 0)
*
* Also, two single-dimension arrays are apparently more performant,
* and more cache friendly than using a 2D array with more overhead.
*/

const int dirX[] = {0, 1, 0, -1};  // North, East, South, West
const int dirY[] = {-1, 0, 1, 0};  // North, East, South, West

void RandomizedFloodFill(int grid[GRID_HEIGHT][GRID_WIDTH], int startX, int startY)
{
    // Early validation of parameters before allocation
    if (startX < 0 || startX >= GRID_WIDTH || startY < 0 || startY >= GRID_HEIGHT)
    {
        return;
    }

    // Here we allocate memory for our algorithm,
    // Rooms already take space in the grid, so we don't need the whole grid!
    size_t stackCapacity = (size_t)(GRID_WIDTH * GRID_HEIGHT) / 4;
    Corridor* stack = (Corridor*)malloc(stackCapacity * sizeof(Corridor));

    if (stack == NULL)
    {
        return; // Allocation failed!
    }

    int stackSize = 0;
    Direction lastDir = -1; // The intent here is that we track the last direction to produce winding paths!

    // To initiate the "stack" and the corridor generation,
    // We add the startPos to the stack and mark it as a corridor cell
    if (IsValidCorridorCell(grid, startX, startY))
    {
        stack[stackSize++] = (Corridor){ startX, startY };
        grid[startY][startX] = CELL_CORRIDOR;
    }

    // This is an attempt at a Growing-Tree Algorithm
    while (stackSize > 0)
    {
        // Get last corridor in the stack ( our most recently added ) 
        Corridor current = stack[stackSize - 1];

        // Here, we find the valid directions
        bool foundValidDirection = false; 
        int availableDirections[4];
        int numValidDirections = 0; 

        // Check directions
        for (int d = 0; d < 4; d++)
        {
            // Here, we move 2 cells in a direction ( 1 cell padding )
            int newX = current.x + (dirX[d] * 2);  // dirY and dirX in Dungeon.h
            int newY = current.y + (dirY[d] * 2);

            // Check if we can create corridors in this direction
            if (IsValidCorridorCell(grid, newX, newY))
            {
                availableDirections[numValidDirections++] = d;
            }
        }

        // If directions are available, then:
        if (numValidDirections > 0)
        {
            int dirIndex;

            // Introducing direction bias!
            if (lastDir >= 0 && GetRandomValue(0, 100) > 30) // 70% chance to continue same direction
            {
                // Iterate through valid directions
                for (int i = 0; i < numValidDirections; i++)
                {
                    // If we find the last direction in the available directions,
                    // Continue in the same direction!
                    if (availableDirections[i] == lastDir)
                    {
                        dirIndex = i;
                        foundValidDirection = true;

                        break;
                    }
                }
            }

            // If we couldn't continue in same direction, pick a random available direction!
            if (!foundValidDirection)
            {
                dirIndex = GetRandomValue(0, numValidDirections - 1);
            }

            // This keeps track of which direction we chose!
            int direction = availableDirections[dirIndex];
            lastDir = direction; // We then store it for the next iteration, in case we want to continue that way!

            // Calculate middle and end positions
            int midX = current.x + dirX[direction];      // One step in chosen direction
            int midY = current.y + dirY[direction];

            int newX = current.x + (dirX[direction] * 2); // Two steps in chosen direction
            int newY = current.y + (dirY[direction] * 2);

            grid[midY][midX] = CELL_CORRIDOR;    // Set middle cell to corridor
            grid[newY][newX] = CELL_CORRIDOR;    // Set destination cell to corridor

            // Add new position to stack
            if (stackSize < stackCapacity)
            {
                stack[stackSize++] = (Corridor){ newX, newY };
            }
        }
        else
        {
            // No available directions!
            stackSize--;
            lastDir = -1; // Reset preferred direction
        }
    }

    free(stack); // Free stack memory!
}

// Here, we generate our mazes from multiple points!
void GenerateMazes(int grid[GRID_HEIGHT][GRID_WIDTH])
{
    for (int i = 2; i < GRID_HEIGHT-2; i += 2) // 2 at a time!
    {
        for (int j = 2; j < GRID_WIDTH-2; j += 2)
        {
            if (IsValidCorridorCell(grid, j, i))
            {
                RandomizedFloodFill(grid, j, i);
            }
        }
    }
}

/* Here, we attempt to create connections between rooms and corridors,
 * First we allocate memory for a boolean array to keep track of connections,
 * Then, we iterate over each room, and check if we can connect it to a corridor,
 *
 * We then introduce randomness to door placement, and check if we can place a door,
 * If we can, we place a door, and mark the room as connected.
 * If we cannot, we try to place a door in a random direction, followed by a corridor cell.
 *
 */
void ConnectRoomsViaDoors(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int roomCount)
{
    // calloc => runtime heap allocation, initializes 0 (false for bool)
    bool* hasConnection = (bool*)calloc(roomCount, sizeof(bool));

    if (hasConnection == NULL)
    {
        return; // Allocation failed!
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

    free(hasConnection);
}

// Here, we define starting and end rooms for our pathfinding algorithm.
static bool InitializeRoomIndices(Room rooms[], int roomCount, int* startRoomIndex, int* bossRoomIndex)
{
    Room startRoom = FindStartingRoom(rooms, roomCount);
    Room bossRoom = FindBossRoom(rooms, roomCount);

    *startRoomIndex = -1;
    *bossRoomIndex = -1;

    // Find the matching room indices
    for (int i = 0; i < roomCount; i++)
    {
        if (rooms[i].x == startRoom.x && rooms[i].y == startRoom.y &&
            rooms[i].width == startRoom.width && rooms[i].height == startRoom.height)
        {
            *startRoomIndex = i;
        }

        if (rooms[i].x == bossRoom.x && rooms[i].y == bossRoom.y &&
            rooms[i].width == bossRoom.width && rooms[i].height == bossRoom.height)
        {
            *bossRoomIndex = i;
        }
    }

    // Return success/failure status
    if (*startRoomIndex != -1 && *bossRoomIndex != -1)
    {
        return true;
    }

    printf("Failed to find start or boss room indices!\n");
    return false;
}

void GenerateDungeon(int grid[GRID_HEIGHT][GRID_WIDTH])
{
    Room rooms[ROOM_AMOUNT];
    int roomCount;

    GenerateGrid(grid);
    GenerateRooms(grid, rooms, &roomCount);
    GenerateMazes(grid);
    ConnectRoomsViaDoors(grid, rooms, roomCount);

    int startRoomIndex, bossRoomIndex;
    if (InitializeRoomIndices(rooms, roomCount, &startRoomIndex, &bossRoomIndex))
    {
        GeneratePaths(grid, rooms, roomCount, startRoomIndex, bossRoomIndex);
    }
}

/* Our main print function.
 * Currently, we print a checkerboard pattern using even/odd bits from x/y, determined by GenerateDungeon
 */
void PrintDungeon(int grid[GRID_HEIGHT][GRID_WIDTH])
{
    int totalHeight = GRID_HEIGHT * CELL_SIZE;
    int totalWidth = GRID_WIDTH * CELL_SIZE;

    int startX = (GetScreenWidth() - totalWidth) / 2;
    int startY = (GetScreenHeight() - totalHeight) / 2;

    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            int drawX = startX + (x * CELL_SIZE);
            int drawY = startY + (y * CELL_SIZE);

            if (grid[y][x] >= ROOM_ID_START)
            {
                DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, BLACK);
            }
            else
            {
                switch(grid[y][x])
                {
                    case CELL_EMPTY_1:
                    case CELL_EMPTY_2:
                        DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, GRAY);
                    break;

                    case CELL_CORRIDOR:
                        DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, DARKGRAY);
                    break;

                    case CELL_DOOR:
                        DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, RED);
                    break;

                    case CELL_PATH:
                        DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, SKYBLUE);
                    break;
                }
            }
        }
    }
}