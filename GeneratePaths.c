#include "GeneratePaths.h"

#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int dirX[] = { 0, 1, 0, -1 };
static const int dirY[] = { -1, 0, 1, 0 };

// Static just to encapsulate these functions, essentially C# private
// Helper function to return the Manhattan distance between two rooms!
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

/* While theorizing about the dungeon,
 * The idea here is for every room to only connect to one other room,
 * So each room finds its closest room, and connects only to it.
 * If RoomA is connected to RoomB, it means RoomB must now find its closest room, and connect to it!
 */
static int FindClosestRoom(Room rooms[], int roomCount, bool connected[], int currentRoom)
{
    int closestRoom = -1;
    int closestDistance = INT_MAX;

    for (int i = 0; i < roomCount; ++i)
    {
        if (!connected[i] && i != currentRoom)
        {
            int distance = GetRoomDistance(rooms[currentRoom], rooms[i]);

            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestRoom = i;
            }
        }
    }

    return closestRoom;
}

// Not static, since Dungeon.C needs to use this
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

// Here, we iterate over the room perimeter and find the door position!
bool FindDoorPosition(int grid[GRID_HEIGHT][GRID_WIDTH], Room room, int* doorX, int* doorY)
{
    for (int y = room.y - 1; y <= room.y + room.height; y++)
    {
        for (int x = room.x - 1; x <= room.x + room.width; x++)
        {
            if (grid[y][x] == CELL_DOOR)
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

/* Our initialization settings for pathfinding,
 * We allocate memory for connected rooms, queue, visited cells, and previous cells,
 * We also find the starting room's door position for path generation,
 * And later use this to generate our path between doors
 */
static bool InitializePathfinding(bool** connected, Corridor** queue, bool** visited,
    Corridor** previous, int roomCount, int* startDoorX, int* startDoorY,
    int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int startRoomIndex)
{
    // Keeping track of connected rooms
    *connected = calloc(roomCount, sizeof(bool));
    if (*connected == NULL)
    {
        printf("Connected array allocation failed!\n");
        return false;
    }

    // Storing Cells to Visit
    *queue = malloc(GRID_SIZE * sizeof(Corridor));
    if (*queue == NULL)
    {
        printf("Queue allocation failed!\n");
        free(*connected);
        return false;
    }

    // Storing Visited cells
    *visited = calloc(GRID_SIZE, sizeof(bool));
    if (*visited == NULL)
    {
        printf("Visited array allocation failed!\n");
        free(*connected);
        free(*queue);
        return false;
    }

    // Storing the overall path
    *previous = malloc(GRID_SIZE * sizeof(Corridor));
    if (*previous == NULL)
    {
        printf("Previous array allocation failed!\n");
        free(*connected);
        free(*queue);
        free(*visited);
        return false;
    }

    // Find starting room's door - our entry point for path generation
    if (!FindDoorPosition(grid, rooms[startRoomIndex], startDoorX, startDoorY))
    {
        printf("No door found for starting room!\n");
        free(*connected);
        free(*queue);
        free(*visited);
        free(*previous);
        return false;
    }

    return true;
}

/* This is a Helper function intended to ensure that vertical and horizontal paths are placed correctly,
 * We don't want to create corridors that are 2 cells wide!
 */
static bool IsValidPathPlacement(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y, bool isVertical)
{
    // Check all adjacent cells (up, right, down, left)
    static const int dx[] = {0, 1, 0, -1};
    static const int dy[] = {-1, 0, 1, 0};

    for (int dir = 0; dir < 4; dir++)
    {
        int checkX = x + dx[dir];
        int checkY = y + dy[dir];

        if (IS_VALID_CELL(checkX, checkY) && grid[checkY][checkX] == CELL_PATH)
        {
            bool checkIsVertical = (dx[dir] == 0); // If dx is 0, it's a vertical path
            if (isVertical == checkIsVertical)
            {
                return false; // Found same orientation adjacent
            }
        }
    }
    return true;
}

/* Our main pathfinding function,
 * This function is inspired and uses a modification of Prim's Algorithm and Breadth's First Search,
 * We calculate the direct distance between two doors, etc..
 */
static bool FindPathBetweenDoors(int grid[GRID_HEIGHT][GRID_WIDTH],
    Corridor currentDoor, int nextDoorX, int nextDoorY,
    Corridor* queue, bool* visited, Corridor* previous)
{
    // Calculate direct distance and maximum allowed path length
    int directDistance = abs(nextDoorX - currentDoor.x) + abs(nextDoorY - currentDoor.y);
    int maxAllowedLength = (int)(directDistance * PATH_LENGTH_THRESHOLD);

    /* We limit new path creation to exactly 4 cells maximum.
     * This forces the algorithm to heavily prioritize existing corridors and paths,
     * while still allowing small detours when necessary.
     */
    const int MAX_NEW_PATH_CELLS = 4;
    int maxNewPaths = MAX_NEW_PATH_CELLS;

    // Set byte size to 0 every time we reset paths! also faster than a loop!
    memset(visited, 0, GRID_SIZE * sizeof(bool));

    int queueFront = 0; // Index to Take Cells from ( Front of Queue )
    int queueBack = 0; // Index to Add Cells to ( Back of Queue )

    // Starting Position: ( add, increment, set visited )
    queue[queueBack] = currentDoor;
    queueBack++;
    visited[GET_GRID_INDEX(currentDoor.x, currentDoor.y)] = true;

    bool foundPath = false;
    int newPathCount = 0;  // Track how many new path cells we've created

    // BSF, Breadth-First Search Algorithm attempt
    while (queueFront < queueBack && !foundPath)
    {
        Corridor current = queue[queueFront++];

        for (int i = 0; i < 4; ++i) // directions
        {
            int newX = current.x;
            int newY = current.y;

            bool canExtend = false;
            bool isNewPath = false;
            bool isVerticalMove = (dirX[i] == 0); // If moving up/down

            for (int scout = 1; scout <= CELL_SCOUT_AMOUNT; scout++)
            {
                newX += dirX[i];
                newY += dirY[i];

                if (!IS_VALID_CELL(newX, newY))
                {
                    break;
                }

                // Check if we've reached the next room's door
                if (newX == nextDoorX && newY == nextDoorY)
                {
                    previous[GET_GRID_INDEX(newX, newY)] = current; // Store the path we found!
                    return true;
                }

                // Prioritize existing corridors AND paths, only create new paths if necessary
                if (!visited[GET_GRID_INDEX(newX, newY)])
                {
                    if (grid[newY][newX] == CELL_CORRIDOR || grid[newY][newX] == CELL_PATH)
                    {
                        canExtend = true;
                        isNewPath = false;
                        break;  // Immediately use existing corridor or path
                    }
                    else if (CAN_BE_PATH(grid[newY][newX]) &&
                            grid[newY][newX] != CELL_DOOR &&
                            queueBack < maxAllowedLength &&
                            newPathCount < MAX_NEW_PATH_CELLS &&
                            IsValidPathPlacement(grid, newX, newY, isVerticalMove))
                    {
                        canExtend = true;
                        isNewPath = true;
                    }
                }
            }

            // If paths can be extended,
            if (canExtend)
            {
                newX = current.x + dirX[i];
                newY = current.y + dirY[i];

                // If not visited at our new position,
                if (!visited[GET_GRID_INDEX(newX, newY)])
                {
                    queue[queueBack] = (Corridor){newX, newY}; // add new position to queue
                    queueBack++; // Move queue end forward

                    visited[GET_GRID_INDEX(newX, newY)] = true; // mark as visited
                    previous[GET_GRID_INDEX(newX, newY)] = current; // remember path!

                    if (isNewPath)
                    {
                        newPathCount++;  // Only increment for new paths
                    }
                }
            }
        }
    }

    return false;
}

void GeneratePaths(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int roomCount, int startRoomIndex, int bossRoomIndex)
{
    bool* connected;
    Corridor* queue;
    bool* visited;
    Corridor* previous;
    int startDoorX, startDoorY;

    if (!InitializePathfinding(&connected, &queue, &visited, &previous,
        roomCount, &startDoorX, &startDoorY,
        grid, rooms, startRoomIndex))
    {
        return;
    }

    connected[startRoomIndex] = true; // = visited
    int currentRoom = startRoomIndex;
    int roomsConnected = 1;

    // Track current door position for path connections
    Corridor currentDoor = (Corridor){startDoorX, startDoorY};

    while (roomsConnected < roomCount)
    {
        int nextRoom = FindClosestRoom(rooms, roomCount, connected, currentRoom);

        if (nextRoom != -1)
        {
            int nextDoorX, nextDoorY;

            if (FindDoorPosition(grid, rooms[nextRoom], &nextDoorX, &nextDoorY))
            {
                if (FindPathBetweenDoors(grid, currentDoor, nextDoorX, nextDoorY,
                    queue, visited, previous))
                {
                    // If a reasonable path has been established, we now want to mark the path for illustration purposes
                    int currentX = nextDoorX;
                    int currentY = nextDoorY;
                    Corridor prev = previous[GET_GRID_INDEX(currentX, currentY)];

                    // Start marking from cell before the next door
                    currentX = prev.x;
                    currentY = prev.y;

                    // Mark path until we reach the cell before current door
                    while (currentX != currentDoor.x || currentY != currentDoor.y)
                    {
                        if (grid[currentY][currentX] != CELL_DOOR &&
                            grid[currentY][currentX] != CELL_CORRIDOR)
                        {
                            grid[currentY][currentX] = CELL_PATH;
                        }

                        prev = previous[GET_GRID_INDEX(currentX, currentY)];
                        currentX = prev.x;
                        currentY = prev.y;
                    }

                    // Update current door and room for next iteration
                    currentDoor = (Corridor){nextDoorX, nextDoorY};
                    connected[nextRoom] = true;
                    roomsConnected++;
                    currentRoom = nextRoom;
                }
            }
        }
    }

    // Free memory
    free(queue);
    free(visited);
    free(previous);
    free(connected);
}