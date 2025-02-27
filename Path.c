#include "Path.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Door.h"

static const int dirX[] = { 0, 1, 0, -1 };
static const int dirY[] = { -1, 0, 1, 0 };

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
            int distance = 0;
            int centerAX = rooms[currentRoom].x + (rooms[currentRoom].width >> 1);
            int centerAY = rooms[currentRoom].y + (rooms[currentRoom].height >> 1);
            int centerBX = rooms[i].x + (rooms[i].width >> 1);
            int centerBY = rooms[i].y + (rooms[i].height >> 1);

            distance = abs(centerAX - centerBX) + abs(centerAY - centerBY);

            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestRoom = i;
            }
        }
    }

    return closestRoom;
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
    int newPathCount = 0;  // Track how many new path cells we've created

    // Set byte size to 0 every time we reset paths! also faster than a loop!
    memset(visited, 0, GRID_SIZE * sizeof(bool));

    int queueFront = 0; // Index to Take Cells from ( Front of Queue )
    int queueBack = 0;  // Index to Add Cells to ( Back of Queue )

    // Starting Position: ( add, increment, set visited )
    queue[queueBack] = currentDoor;
    queueBack++;
    visited[GET_GRID_INDEX(currentDoor.x, currentDoor.y)] = true;

    // BSF, Breadth-First Search Algorithm attempt
    while (queueFront < queueBack)
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
                        // Check if using this corridor would create a 2-cell wide path
                        if (IsValidPathPlacement(grid, newX, newY, isVerticalMove))
                        {
                            canExtend = true;
                            isNewPath = false;
                            break;  // Immediately use existing corridor or path
                        }
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
                        newPathCount++;
                    }
                }
            }
        }
    }

    // Return false when no path is found, keeping the search within our 4-cell new path limit
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

    connected[startRoomIndex] = true;
    int currentRoom = startRoomIndex;
    int roomsConnected = 1;

    // Track current door position for path connections
    Corridor currentDoor = (Corridor){startDoorX, startDoorY};

    // Keep trying until all rooms are connected
    while (roomsConnected < roomCount)
    {
        int nextRoom = FindClosestRoom(rooms, roomCount, connected, currentRoom);

        // If we can't find an unconnected room from current position,
        // try from each connected room until we find a path
        if (nextRoom == -1)
        {
            bool foundNewPath = false;

            // Try from each already connected room
            for (int tryRoom = 0; tryRoom < roomCount && !foundNewPath; tryRoom++)
            {
                if (connected[tryRoom])
                {
                    // Find door position for this room
                    int tryDoorX, tryDoorY;
                    if (FindDoorPosition(grid, rooms[tryRoom], &tryDoorX, &tryDoorY))
                    {
                        // Try to find path from this room to any unconnected room
                        for (int targetRoom = 0; targetRoom < roomCount; targetRoom++)
                        {
                            if (!connected[targetRoom])
                            {
                                int targetDoorX, targetDoorY;
                                if (FindDoorPosition(grid, rooms[targetRoom], &targetDoorX, &targetDoorY))
                                {
                                    currentDoor = (Corridor){tryDoorX, tryDoorY};
                                    if (FindPathBetweenDoors(grid, currentDoor, targetDoorX, targetDoorY,
                                        queue, visited, previous))
                                    {
                                        // Mark the path
                                        int currentX = targetDoorX;
                                        int currentY = targetDoorY;
                                        Corridor prev = previous[GET_GRID_INDEX(currentX, currentY)];

                                        currentX = prev.x;
                                        currentY = prev.y;

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

                                        printf("Connected room %d to room %d (retry path)\n", tryRoom, targetRoom);
                                        currentRoom = targetRoom;
                                        connected[targetRoom] = true;
                                        roomsConnected++;
                                        foundNewPath = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (!foundNewPath)
            {
                printf("Failed to connect all rooms! Connected: %d/%d\n", roomsConnected, roomCount);
                break;
            }
        }
        else
        {
            int nextDoorX, nextDoorY;

            if (FindDoorPosition(grid, rooms[nextRoom], &nextDoorX, &nextDoorY))
            {
                if (FindPathBetweenDoors(grid, currentDoor, nextDoorX, nextDoorY,
                    queue, visited, previous))
                {
                    // Mark the path
                    int currentX = nextDoorX;
                    int currentY = nextDoorY;
                    Corridor prev = previous[GET_GRID_INDEX(currentX, currentY)];

                    currentX = prev.x;
                    currentY = prev.y;

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

                    printf("Connected room %d to room %d\n", currentRoom, nextRoom);
                    currentDoor = (Corridor){nextDoorX, nextDoorY};
                    connected[nextRoom] = true;
                    roomsConnected++;
                    currentRoom = nextRoom;
                }
                else
                {
                    // If we can't connect to closest room, mark it as temporarily "connected"
                    // so we can try other rooms first
                    connected[nextRoom] = true;
                }
            }
        }
    }

    // Verify all rooms are actually connected
    bool allConnected = true;
    for (int i = 0; i < roomCount; i++)
    {
        if (!connected[i])
        {
            printf("Room %d is not connected!\n", i);
            allConnected = false;
        }
    }

    if (!allConnected)
    {
        printf("ERROR: Not all rooms are connected after pathfinding!\n");
    }

    // Free memory
    free(queue);
    free(visited);
    free(previous);
    free(connected);
}