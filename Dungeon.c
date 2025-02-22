#include <raylib.h>
#include "Dungeon.h"

#include <stdlib.h>

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
bool IsRoomValid (int grid[GRID_HEIGHT][GRID_WIDTH], Room room)
{
    if (room.x < 2 || room.y < 2 ||
        room.x + room.width >= GRID_WIDTH-2 ||room.y + room.height >= GRID_HEIGHT-2)
    {
        return false;
    }

    for (int y = room.y - 3; y < room.y + room.height + 3; y++)
    {
        for (int x = room.x - 3; x < room.x + room.width + 3; x++)
        {
            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT)
            {
                if (grid[y][x] == 2)
                {
                    return false;
                }
            }
        }
    }
    // And if all has been checked, return true, the room is valid
    return true;
}

/* This is where we attempt placing rooms in our grid,
 * By iterating over every cell in the room and marking it */
void PlaceRoom(int grid[GRID_HEIGHT][GRID_WIDTH], Room room)
{
    for (int y = room.y; y < room.y + room.height; y++)
    {
        for (int x = room.x; x < room.x + room.width; x++)
        {
            grid[y][x] = 2; // Mark as room cell
        }
    }
}

void GenerateRooms(int grid[GRID_HEIGHT][GRID_WIDTH])
{
    for (int i = 0; i < ROOM_AMOUNT; i++)
    {
        GenerateRoom(grid);
    }
}

/* This is where we generate a room of a random size, then,
 * If the room is in a valid position, we place it in the grid! */
bool GenerateRoom(int grid[GRID_HEIGHT][GRID_WIDTH])
{
    const int MAX_ATTEMPTS = 100;

    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++)
    {
        int width = GetRandomValue(ROOM_MIN_WIDTH, ROOM_MAX_SIZE);
        int height = GetRandomValue(ROOM_MIN_HEIGHT, ROOM_MAX_SIZE);

        int x = GetRandomValue(0, GRID_WIDTH - width);
        int y = GetRandomValue(0, GRID_HEIGHT - height);

        Room room = CreateRoom(x, y, width, height);

        if (IsRoomValid(grid, room))
        {
            PlaceRoom(grid, room);
            return true;
        }
    }

    return false;
}

/* This is our general check if we can generate corridors on cells in the grid!
 * We first check the grid boundaries, then,
 * We check the if the cell is empty ( not room or corridor ), then,
 * As the corridor progresses, we must check if any of the neighbouring cells has a room.
 * We do this to make sure that the corridors has at least 1 cell of spacing between rooms and corridors.
 */
bool IsValidCorridorCell(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y)
{
    if (x < 2 || y < 2 || x >= GRID_WIDTH - 2 || y >= GRID_HEIGHT - 2)
    {
        return false;
    }

    // Only empty cells are valid!
    if (grid[y][x] != CELL_EMPTY_1 && grid[y][x] != CELL_EMPTY_2)
    {
        return false;
    }

    // Here, we check the 5 x 5 area around the cell for rooms!
    // (-2, -2) to (2, 2) ( 5 x 5 )
    for (int cy = -2; cy <= 2; ++cy)
    {
        for (int cx = -2; cx <= 2; ++cx)
        {
            // Check cells
            int newX = x + cx;
            int newY = y + cy;

            // Skip out-of-bounds checks
            if (newX < 0 || newX >= GRID_WIDTH || newY < 0 || newY >= GRID_HEIGHT)
            {
                continue;
            }

            if (grid[newY][newX] == CELL_ROOM)
            {
                // Checking diagonals with using the Manhattan distance formula!
                // = abs(x1 - x2) + abs(y1 - y2)

                int xDist = abs(cx);
                int yDist = abs(cy);

                // If both distances are 1, we found a diagonal point (room corner)
                // Or if we're too close to the room
                if ((xDist == 1 && yDist == 1) || (xDist + yDist < 3))  // padding
                {
                    return false;
                }
            }
        }
    }

    // Here, we count the adjacent corridors (including diagonals)
    int corridorCount = 0;

    // (-1, -1) to (1, 1) ( 3 x 3)
    for (int cy = -1; cy <= 1; ++cy)
    {
        for (int cx = -1; cx <= 1; ++cx)
        {
            if (cx == 0 && cy == 0)
            {
                continue;
            }

            // Check cells
            int newX = x + cx;
            int newY = y + cy;

            if (newX >= 0 && newX < GRID_WIDTH-2 && newY >= 0 && newY < GRID_HEIGHT-2)
            {
                if (grid[newY][newX] == CELL_ROOM)
                {
                    corridorCount++;
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

void GenerateDungeon (int grid[GRID_HEIGHT][GRID_WIDTH])
{
    GenerateGrid(grid);
    GenerateRooms(grid);
    GenerateMazes(grid);
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

            switch(grid[y][x])
            {
                case 0:
                    DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, GRAY);
                break;

                case 1:
                    DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, GRAY);
                break;

                case 2:
                    DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, BLACK);
                break;

                case CELL_CORRIDOR:
                    DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, DARKGRAY);
                break;
            }
        }
    }
}