#include <raylib.h>
#include "Dungeon.h"
#include "Corridor.h"
#include <stdlib.h>
#include <stdio.h>

/* This is our general check if we can generate corridors on cells in the grid!
 * We first check the grid boundaries, then,
 * We check the if the cell is empty ( not room or corridor ),
 */
bool IsValidCorridorCell(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y)
{
    if (!IS_IN_GRID(x, y) || x < 1 || y < 1 || x >= GRID_WIDTH - 1 || y >= GRID_HEIGHT - 1)
    {
        return false;
    }

    // Cache the cell value
    const int cell = grid[y][x];

    // Only empty cells are valid!
    if (!IS_EMPTY(cell))
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
    if (!IS_IN_GRID(startX, startY))
    {
        return;
    }

    // Here we allocate memory for our algorithm,
    // Rooms already take space in the grid, so we don't need the whole grid!
    size_t stackCapacity = (size_t)(GRID_WIDTH * GRID_HEIGHT) >> 2; // same as / 2
    Corridor* stack = (Corridor*)malloc(stackCapacity * sizeof(Corridor));

    if (stack == NULL)
    {
        return; // Allocation failed!
    }

    int stackSize = 0;
    Direction lastDir = -1; // The intent here is that we track the last direction to produce winding paths!

    // To initiate the "stack" and the corridor generation,
    // We add the startPos to the stack and mark it as a corridor cell
    bool isValidStart = IsValidCorridorCell(grid, startX, startY); // caching validity

    if (isValidStart)
    {
        stack[stackSize++] = (Corridor){ startX, startY };
        grid[startY][startX] = CELL_CORRIDOR;
    }

    const int DIRECTION_BIAS_THRESHOLD = 30;  // 70% chance to continue in the same direction!
    const int MAX_ITERATIONS = GRID_WIDTH * GRID_HEIGHT; // Allow enough iterations to fill the grid
    int iterations = 0;

    // This is an attempt at a Growing-Tree Algorithm
    while (stackSize > 0 && iterations++ < MAX_ITERATIONS)
    {
        // Get last corridor in the stack ( our most recently added )
        const Corridor current = stack[stackSize - 1];

        // Here, we find the valid directions
        bool foundValidDirection = false;
        int availableDirections[4];
        int numValidDirections = 0;

        // Check directions
        for (int d = 0; d < 4; d++)
        {
            // Here, we move 2 cells in a direction ( 1 cell padding )
            const int newX = current.x + (dirX[d] << 1); // << 1 is the same as * 2!
            const int newY = current.y + (dirY[d] << 1);

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
            const int randomChance = GetRandomValue(0, 100); // cache random value

            if (lastDir >= 0 && randomChance > DIRECTION_BIAS_THRESHOLD) // 70% chance to continue same direction
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
            const int direction = availableDirections[dirIndex];
            lastDir = direction; // We then store it for the next iteration, in case we want to continue that way!

            // Calculate middle and end positions
            const int midX = current.x + dirX[direction]; // One step in chosen direction
            const int midY = current.y + dirY[direction];

            const int newX = current.x + (dirX[direction] << 1);
            const int newY = current.y + (dirY[direction] << 1);

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

    if (iterations >= MAX_ITERATIONS)
    {
        printf("RandomizedFloodFill exceeded maximum iterations at (%d, %d)!\n", startX, startY);
    }

    free(stack); // Free stack memory!
}

// Here, we generate our mazes from multiple points!
void GenerateMazes(int grid[GRID_HEIGHT][GRID_WIDTH])
{
    /* Instead of writing " 2 ", we use a constant for processing speed.
     * Apparently, this form of caching is faster than using direct value, at least theoretically,
     * So it is a stretch to claim this!
     */
    const int STEP_SIZE = 2;
    const int boundaryY = GRID_HEIGHT - STEP_SIZE;
    const int boundaryX = GRID_WIDTH - STEP_SIZE;

    for (int i = STEP_SIZE; i < boundaryY; i += STEP_SIZE)
    {
        for (int j = STEP_SIZE; j < boundaryX; j += STEP_SIZE)
        {
            if (IS_IN_GRID(j, i) && IsValidCorridorCell(grid, j, i))
            {
                RandomizedFloodFill(grid, j, i);
            }
        }
    }
}