#include <raylib.h>
#include "Dungeon.h"
#include <stdio.h>

// Include all component headers
#include "Room.h"
#include "Corridor.h"
#include "Door.h"
#include "Path.h"
#include "Staircase.h"

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

bool GenerateDungeon(int grid[GRID_HEIGHT][GRID_WIDTH], int maxAttempts, int currentFloor,
                     Room rooms[], int* roomCount)
{
    // Initialize the grid with a checkerboard pattern
    GenerateGrid(grid);

    // Step 1: Generate rooms
    if (!GenerateRooms(grid, rooms, roomCount))
    {
        printf("Room generation failed\n");
        return false;
    }

    // Step 2: Generate maze-like corridors in empty spaces
    GenerateMazes(grid);

    // Step 3: Connect rooms using doors
    if (!ConnectRoomsViaDoors(grid, rooms, *roomCount))
    {
        printf("Door connection failed\n");
        return false;
    }

    // Step 4: Find start and boss room indices
    int startRoomIndex, bossRoomIndex;
    if (!InitializeRoomIndices(rooms, *roomCount, &startRoomIndex, &bossRoomIndex))
    {
        printf("Room indices initialization failed\n");
        return false;
    }

    // Step 5: Generate paths between rooms
    GeneratePaths(grid, rooms, *roomCount, startRoomIndex, bossRoomIndex);

    // Step 6: Place up and down staircases
    PlaceStaircases(grid, rooms, *roomCount, currentFloor);

    return true;
}

/* Our main print function.
 * Currently, we print a checkerboard pattern using even/odd bits from x/y, determined by GenerateDungeon
 */
void PrintDungeon(int grid[GRID_HEIGHT][GRID_WIDTH], Room rooms[], int roomCount)
{
    const int totalHeight = GRID_TOTAL_HEIGHT;
    const int totalWidth = GRID_TOTAL_WIDTH;

    const int startX = CENTER_SCREEN_X(totalWidth);
    const int startY = CENTER_SCREEN_Y(totalHeight);

    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            const int drawX = startX + (x * CELL_SIZE);
            const int drawY = startY + (y * CELL_SIZE);
            const int cell = grid[y][x];

            if (IS_ROOM(cell))
            {
                // Default room color
                Color roomColor = BLACK;

                // Highlight special rooms
                for (int i = 0; i < roomCount; i++)
                {
                    // Find which room this cell belongs to by checking coordinates
                    if (x >= rooms[i].x && x < rooms[i].x + rooms[i].width &&
                        y >= rooms[i].y && y < rooms[i].y + rooms[i].height)
                    {
                        if (rooms[i].type == ROOM_TYPE_START) {
                            roomColor = DARKBLUE;    // Starting room color
                        } else if (rooms[i].type == ROOM_TYPE_BOSS) {
                            roomColor = DARKPURPLE;  // Boss room color
                        }
                        break;
                    }
                }

                DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, roomColor);
            }
            else
            {
                switch(cell)
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
                        DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, GREEN);
                        break;

                    case CELL_STAIR_UP:
                        DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, BLUE);
                        DrawText("<", drawX + 5, drawY + 2, 12, WHITE);
                        break;

                    case CELL_STAIR_DOWN:
                        DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, PURPLE);
                        DrawText(">", drawX + 5, drawY + 2, 12, WHITE);
                        break;
                }
            }
        }
    }
}