#include "Raylib.h"
#include "Game.h"
#include "Dungeon.h"

Game InitGame (int width, int height)
{
    const Game game =
    {
        .screenWidth = width,
        .screenHeight = height,
        .grid = {0} // default initialization
    };

    return game;
}

void UpdateGame (Game* game)
{
    GenerateDungeon(game->grid);
}

void DrawGame (Game game)
{
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);
        PrintDungeon(game.grid);
    }
    EndDrawing();
}