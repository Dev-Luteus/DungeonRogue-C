#include <raylib.h>
#include "Dungeon.h"

void GenerateDungeon (int grid[GRID_HEIGHT][GRID_WIDTH])
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

/* Our main print function.
 * Currently, we print a checkerboard pattern using even/odd bits from x/y, determined by GenerateDungeon
 */
void PrintDungeon (int grid[GRID_HEIGHT][GRID_WIDTH])
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

            if (grid[y][x] == 0)
            {
                DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, GRAY);
            }
            else
            {
                DrawRectangle(drawX, drawY, CELL_SIZE, CELL_SIZE, BLACK);
            }
        }
    }
}