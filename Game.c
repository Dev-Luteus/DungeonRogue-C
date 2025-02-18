#include "Raylib.h"
#include "Game.h"
#include "Dungeon.h"

Game InitGame (int width, int height)
{
    const Game game =
    {
        .screenWidth = width,
        .screenHeight = height
    };

    return game;
}

void UpdateGame (Game* game)
{

}

void DrawGame (Game game)
{
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);
    }
    EndDrawing();
}