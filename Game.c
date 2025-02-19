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
    if (!game->dungeonGenerated)
    {
        GenerateDungeon(game->grid);
        game->dungeonGenerated = true;
    }
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