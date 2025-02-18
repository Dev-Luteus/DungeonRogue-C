#include <raylib.h>
#include "Dungeon.h"


void GenerateDungeon (int grid[GRID_HEIGHT][GRID_WIDTH])
{
    // Fill the grid with empty space
    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            grid[y][x] = 0;
        }
    }
}

void PrintDungeon (int grid[GRID_HEIGHT][GRID_WIDTH])
{
    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            if (grid[y][x] == 0)
            {
                DrawRectangle(x * 10, y * 10, 10, 10, WHITE);
            }
            else
            {
                DrawRectangle(x * 10, y * 10, 10, 10, BLACK);
            }
        }
    }
}