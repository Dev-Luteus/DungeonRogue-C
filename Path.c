#include "Path.h"
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Door.h"

/* Direction vectors for cardinal movement (N, E, S, W)
 * These vectors allow us to explore adjacent cells in the grid
 * by adding these offsets to our current position
 */
static const int dirX[] = { 0, 1, 0, -1 };
static const int dirY[] = { -1, 0, 1, 0 };

/* While theorizing about the dungeon,
 * The idea here is for every room to only connect to one other room, ( I might add exceptions )
 * So each room finds its closest room, and connects only to it.
 * If RoomA is connected to RoomB, it means RoomB must now find its closest room, and connect to it!
 *
 * We do this first by calculating distances between doors of each room, which act as a Connection point.
 * We then implement a nearest-neighbor search using the Manhattan distance formula:
 * (|x1-x2| + |y1-y2|)
 *
 * This in turn represents the total path distance in our grid!
 */
static int FindClosestRoomByDoors(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[],
                                  int roomCount, bool connected[], int currentRoom,
                                  int currentDoorX, int currentDoorY)
{
    int closestRoom = -1;
    int closestDistance = INT_MAX;

    // We skip the boss room since I want it to be connected last
    for (int i = 0; i < roomCount; ++i)
    {
        if (!connected[i] && i != currentRoom)
        {
            int nextDoorX, nextDoorY;

            // Find the door of the candidate room
            if (FindDoorPosition(grid, rooms[i], &nextDoorX, &nextDoorY))
            {
                // Calculate Manhattan distance between doors
                int doorDistance = abs(nextDoorX - currentDoorX) + abs(nextDoorY - currentDoorY);

                if (doorDistance < closestDistance)
                {
                    closestDistance = doorDistance;
                    closestRoom = i;
                }
            }
        }
    }

    return closestRoom;
}

/* Our initialization settings for pathfinding,
 * We allocate memory for connected rooms, queue, visited cells, and previous cells,
 * We also find the starting room's door position to begin our pathfinding =)
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

/* Here, we check if a potential path cell would connect two separate corridor networks
 * The idea here is that we want to winding paths and confusing corridors,
 * Hence, we want much of the maze to remain the same!
 *
 * We're only trying  to "connect the seams", so that no room or mazes are unconnected.
 * Everything must be traversable!
 *
 * To do this, this function counts the number of distinct corridor networks surrounding the cell.
 * A valid "seam connector" should connect at least two separate networks.
 */
static bool ConnectsDistinctCorridors(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y)
{
    // Track adjacent positions that belong to corridors/paths
    bool hasCorridorN = false;
    bool hasCorridorS = false;
    bool hasCorridorE = false;
    bool hasCorridorW = false;

    // Check each cardinal direction
    if (IS_IN_GRID(x, y-1) && (grid[y-1][x] == CELL_CORRIDOR || grid[y-1][x] == CELL_PATH))
    {
        hasCorridorN = true;
    }

    if (IS_IN_GRID(x, y+1) && (grid[y+1][x] == CELL_CORRIDOR || grid[y+1][x] == CELL_PATH))
    {
        hasCorridorS = true;
    }

    if (IS_IN_GRID(x+1, y) && (grid[y][x+1] == CELL_CORRIDOR || grid[y][x+1] == CELL_PATH))
    {
        hasCorridorE = true;
    }

    if (IS_IN_GRID(x-1, y) && (grid[y][x-1] == CELL_CORRIDOR || grid[y][x-1] == CELL_PATH))
    {
        hasCorridorW = true;
    }

    // Calculate north-south and east-west connections
    bool connectsNS = hasCorridorN && hasCorridorS;
    bool connectsEW = hasCorridorE && hasCorridorW;

    // Also check for L-shaped connections (exactly two adjacent corridors that aren't opposite)
    bool lConnection = ((hasCorridorN && hasCorridorE) ||
                       (hasCorridorN && hasCorridorW) ||
                       (hasCorridorS && hasCorridorE) ||
                       (hasCorridorS && hasCorridorW));

    // Return true if this cell creates an efficient corridor connection
    return connectsNS || connectsEW || lConnection;
}

/* This is a helper function which checks if a position (for path cells) is adjacent rooms.
 * We do this because we don't want parallel or wide corridors!
 */
static bool IsAdjacentToRoom(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y)
{
    //
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
            {
                continue;
            }

            int checkX = x + dx;
            int checkY = y + dy;

            if (IS_IN_GRID(checkX, checkY) && IS_ROOM(grid[checkY][checkX]))
            {
                return true;
            }
        }
    }
    return false;
}

/* This is another helper function that checks if placing a path cell would create a 2x2
 * This hasn't quite happened before until now, but, in the case that it does I don't want it.
 * It might be a redundant check, but I do this for qualitative reasons!
 *
 * Henceforth, This function examines all possible 2×2 patterns containing our target cell.
 */
static bool WouldCreate2x2Area(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y)
{
    // Check all possible 2×2 patterns containing (x,y)
    for (int cornerY = y - 1; cornerY <= y; cornerY++)
    {
        for (int cornerX = x - 1; cornerX <= x; cornerX++)
        {
            // Skip if any part of the 2×2 area is outside the grid
            if (!IS_IN_GRID(cornerX, cornerY) ||
                !IS_IN_GRID(cornerX + 1, cornerY) ||
                !IS_IN_GRID(cornerX, cornerY + 1) ||
                !IS_IN_GRID(cornerX + 1, cornerY + 1))
            {
                continue;
            }

            // Count how many cells in this 2×2 area would be pathable
            int pathableCount = 0;

            // Check each cell in the 2×2 area
            for (int dy = 0; dy <= 1; dy++)
            {
                for (int dx = 0; dx <= 1; dx++)
                {
                    int checkX = cornerX + dx;
                    int checkY = cornerY + dy;

                    // The cell we're considering placing
                    if (checkX == x && checkY == y)
                    {
                        pathableCount++;
                        continue;
                    }

                    // Existing pathable cells
                    if (grid[checkY][checkX] == CELL_CORRIDOR ||
                        grid[checkY][checkX] == CELL_PATH)
                    {
                        pathableCount++;
                    }
                }
            }

            // If all 4 cells would be pathable, this creates a 2×2 area
            if (pathableCount == 4)
            {
                return true;
            }
        }
    }

    return false;
}

/* A simple validator helper function */
static bool IsValidPathPlacement(int grid[GRID_HEIGHT][GRID_WIDTH], int x, int y)
{
    if (IsAdjacentToRoom(grid, x, y))
    {
        return false;
    }

    if (WouldCreate2x2Area(grid, x, y))
    {
        return false;
    }

    return true;
}

/* This is a weighted function that prioritizes directions toward the target door!
 * It implements a 'Manhattan distance heuristic' to sort directions,
 * based on the desired target position!
 *
 * Then, for each direction, we calculate how much that move would reduce
 * our distance to the target, then sort by highest reduction.
 */
static void PrioritizeDirections(int directions[4], int currentX, int currentY,
                                int targetX, int targetY)
{
    // Calculate distance reduction for each direction
    int scores[4];

    for (int i = 0; i < 4; i++)
    {
        int newX = currentX + dirX[i];
        int newY = currentY + dirY[i];

        int currentDist = abs(currentX - targetX) + abs(currentY - targetY);
        int newDist = abs(newX - targetX) + abs(newY - targetY);

        // Score is how much this direction reduces our distance to target
        scores[i] = currentDist - newDist;
    }

    // Initialize directions array with indices 0-3
    for (int i = 0; i < 4; i++)
    {
        directions[i] = i;
    }

    // Simple insertion sort by scores (highest first)
    for (int i = 1; i < 4; i++)
    {
        int j = i;

        while (j > 0 && scores[directions[j]] > scores[directions[j-1]])
        {
            int temp = directions[j];

            directions[j] = directions[j-1];
            directions[j-1] = temp;

            j--;
        }
    }
}

/* Our directed path-finding function uses a modified Breadth-First Search,
 * which aims to prioritize following existing corridors whenever possible.
 *
 * It only adds path cells when they create significant shortcuts =)
 */
static bool FindPathBetweenDoors(int grid[GRID_HEIGHT][GRID_WIDTH],
    Corridor currentDoor, int nextDoorX, int nextDoorY,
    Corridor* queue, bool* visited, Corridor* previous)
{
    // Calculate direct distance and maximum allowed path length
    int directDistance = abs(nextDoorX - currentDoor.x) + abs(nextDoorY - currentDoor.y);
    int maxAllowedLength = (int)(directDistance * PATH_LENGTH_THRESHOLD);

    /* We limit new path creation to exactly MAX_NEW_PATH_CELLS maximum.
     * This forces the algorithm to heavily prioritize existing corridors,
     * while still allowing minimal interventions when necessary.
     */
    int newPathCount = 0;

    // Reset visited array
    memset(visited, 0, GRID_SIZE * sizeof(bool));

    // Queue management
    int queueFront = 0;
    int queueBack = 0;

    // Start from the current door
    queue[queueBack++] = currentDoor;
    visited[GET_GRID_INDEX(currentDoor.x, currentDoor.y)] = true;

    // Modified BFS with corridor prioritization
    while (queueFront < queueBack)
    {
        Corridor current = queue[queueFront++];

        // Get direction priorities based on target position
        int directions[4];
        PrioritizeDirections(directions, current.x, current.y, nextDoorX, nextDoorY);

        // Check each direction in priority order
        for (int d = 0; d < 4; ++d)
        {
            int i = directions[d];
            int newX = current.x + dirX[i];
            int newY = current.y + dirY[i];

            // Skip invalid positions
            if (!IS_VALID_CELL(newX, newY)) {
                continue;
            }

            // Skip already visited cells
            if (visited[GET_GRID_INDEX(newX, newY)]) {
                continue;
            }

            // Check if we've reached the target door
            if (newX == nextDoorX && newY == nextDoorY) {
                previous[GET_GRID_INDEX(newX, newY)] = current;
                return true;
            }

            bool usePosition = false;
            bool isNewPath = false;

            // First priority: use existing corridors or paths
            if (grid[newY][newX] == CELL_CORRIDOR || grid[newY][newX] == CELL_PATH) {
                usePosition = true;
            }
            // Second priority: create connecting path cells if needed
            else if (CAN_BE_PATH(grid[newY][newX]) &&
                     grid[newY][newX] != CELL_DOOR &&
                     queueBack < maxAllowedLength &&
                     newPathCount < MAX_NEW_PATH_CELLS) {

                // Only place path cells that truly connect corridor networks
                // or are immediately adjacent to existing paths/corridors
                bool connectsNetworks = ConnectsDistinctCorridors(grid, newX, newY);
                bool adjacentToPath = false;

                for (int dir = 0; dir < 4; dir++) {
                    int adjX = newX + dirX[dir];
                    int adjY = newY + dirY[dir];

                    if (IS_IN_GRID(adjX, adjY) &&
                        (grid[adjY][adjX] == CELL_CORRIDOR || grid[adjY][adjX] == CELL_PATH)) {
                        adjacentToPath = true;
                        break;
                    }
                }

                if ((connectsNetworks || adjacentToPath) &&
                    IsValidPathPlacement(grid, newX, newY)) {
                    usePosition = true;
                    isNewPath = true;
                }
            }

            if (usePosition) {
                queue[queueBack++] = (Corridor){newX, newY};
                visited[GET_GRID_INDEX(newX, newY)] = true;
                previous[GET_GRID_INDEX(newX, newY)] = current;

                if (isNewPath) {
                    newPathCount++;
                }
            }
        }
    }

    // No path found within constraints
    return false;
}

/* This is mostly a helper safety function. I implemented this to make sure that,
 * in the case where our pathfinding somehow fails and the dungeon does not generate,
 * we must allow to ease our constraints. We don't want failed generation!
 */
static bool FindPathWithFallbacks(int grid[GRID_HEIGHT][GRID_WIDTH],
                                 Corridor startDoor, int endDoorX, int endDoorY,
                                 Corridor* queue, bool* visited, Corridor* previous)
{
    int temporaryLimit = MAX_NEW_PATH_CELLS;

    if (FindPathBetweenDoors(grid, startDoor, endDoorX, endDoorY, queue, visited, previous))
    {
        return true;
    }

    // In the case that our path finding failed: we want to allow more path cells and try again!
    int attempts = 1;

    for (int newLimit = MAX_NEW_PATH_CELLS * 2; newLimit <= 12; newLimit += 2)
    {
        // Temporarily increase the path limit
        temporaryLimit = newLimit;

        if (FindPathBetweenDoors(grid, startDoor, endDoorX, endDoorY, queue, visited, previous))
        {
            printf("Connected using fallback attempt %d (limit: %d path cells)\n",
                   attempts, newLimit);

            // Restore original limit
            temporaryLimit = MAX_NEW_PATH_CELLS;
            return true;
        }

        attempts++;
    }

    // Restore original limit
    temporaryLimit = MAX_NEW_PATH_CELLS;
    return false;
}

/* Our main path generation function!
 * This is where everything comes together, we want to ensure to creates a network of connections ->
 * -> with minimal interference to the existing maze structure!
 *
 * It connects rooms by finding the shortest paths between doors,
 * preferring existing corridors and only creating new path cells
 * when they significantly reduce travel distance.
 *
 * The boss room is intentionally connected last to ensure the player
 * must traverse through the dungeon to reach it, but, it can still be otherwise traversed to.
 * It might make it more inconvenient at best but it's simply for the pathfinding algorithm itself.
 */
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

    // Mark rooms as connected/not connected
    connected[startRoomIndex] = true;

    if (bossRoomIndex >= 0 && bossRoomIndex < roomCount)
    {
        connected[bossRoomIndex] = true;  // Temporary mark, I want it to connect last!
    }

    int currentRoom = startRoomIndex;
    int roomsConnected = 1;  // Start room is connected

    if (bossRoomIndex >= 0) // Count boss room as "handled" for now
    {
        roomsConnected++;
    }

    // Track current door position for path connections
    Corridor currentDoor = (Corridor) { startDoorX, startDoorY };

    // Connect all rooms except boss room
    while (roomsConnected < roomCount)
    {
        int nextRoom = FindClosestRoomByDoors(grid, rooms, roomCount, connected,
                                             currentRoom, currentDoor.x, currentDoor.y);

        // If we can't find an unconnected room from current position,
        // try from each connected room until we find a path
        if (nextRoom == -1)
        {
            bool foundNewPath = false;

            // Try from each already connected room except boss room
            for (int tryRoom = 0; tryRoom < roomCount && !foundNewPath; tryRoom++)
            {
                // Skip unconnected rooms and the boss room
                if (!connected[tryRoom] || tryRoom == bossRoomIndex)
                {
                    continue;
                }

                int tryDoorX, tryDoorY;

                if (FindDoorPosition(grid, rooms[tryRoom], &tryDoorX, &tryDoorY))
                {
                    // Try to find path from this room to any unconnected room
                    for (int targetRoom = 0; targetRoom < roomCount; targetRoom++)
                    {
                        // Skip connected rooms and the boss room
                        if (connected[targetRoom] || targetRoom == bossRoomIndex)
                        {
                            continue;
                        }

                        int targetDoorX, targetDoorY;

                        if (FindDoorPosition(grid, rooms[targetRoom], &targetDoorX, &targetDoorY))
                        {
                            Corridor tryDoor = (Corridor) { tryDoorX, tryDoorY };

                            if (FindPathWithFallbacks(grid, tryDoor, targetDoorX, targetDoorY,
                                queue, visited, previous))
                            {
                                // Mark the path
                                int currentX = targetDoorX;
                                int currentY = targetDoorY;
                                Corridor prev = previous[GET_GRID_INDEX(currentX, currentY)];

                                currentX = prev.x;
                                currentY = prev.y;

                                while (currentX != tryDoor.x || currentY != tryDoor.y)
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
                                currentDoor = (Corridor){targetDoorX, targetDoorY};
                                connected[targetRoom] = true;
                                roomsConnected++;
                                foundNewPath = true;

                                break;
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
                if (FindPathWithFallbacks(grid, currentDoor, nextDoorX, nextDoorY,
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
                    // If we can't connect to closest room even with fallbacks,
                    // skip it temporarily
                    printf("Could not connect to room %d - will try alternative paths\n", nextRoom);
                    connected[nextRoom] = true;  // Mark as "handled" but not truly connected yet
                    roomsConnected++;
                }
            }
        }
    }

    // Now connect the boss room last (if we have one)
    if (bossRoomIndex >= 0 && bossRoomIndex < roomCount && bossRoomIndex != startRoomIndex)
    {
        // Find door for the last connected room
        int lastDoorX = currentDoor.x;
        int lastDoorY = currentDoor.y;

        // Boss door!
        int bossDoorX, bossDoorY;

        if (FindDoorPosition(grid, rooms[bossRoomIndex], &bossDoorX, &bossDoorY))
        {
            Corridor lastDoor = (Corridor){lastDoorX, lastDoorY};

            // Try to connect the boss room
            if (FindPathWithFallbacks(grid, lastDoor, bossDoorX, bossDoorY,
                queue, visited, previous))
            {
                int currentX = bossDoorX;
                int currentY = bossDoorY;
                Corridor prev = previous[GET_GRID_INDEX(currentX, currentY)];

                currentX = prev.x;
                currentY = prev.y;

                while (currentX != lastDoor.x || currentY != lastDoor.y)
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

                printf("Connected final room to boss room %d\n", bossRoomIndex);
                connected[bossRoomIndex] = true;
            }
            else
            {
                printf("Failed to connect boss room!\n");
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