#include <raylib.h>
#include "Dungeon.h"

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

void GenerateDungeon (int grid[GRID_HEIGHT][GRID_WIDTH])
{
    GenerateGrid(grid);
    GenerateRooms(grid);
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
                    DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, DARKGRAY);
                break;

                case 2:
                    DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, BLACK);
                break;
            }
        }
    }
}