#include <stdio.h>
#include <raylib.h>
#include "Game.h"

int main(void)
{
    const int width = 1920;
    const int height = 1080;

    InitWindow(width, height, "Dungeon Rogue C!");
    SetTargetFPS(200);

    Game game = InitGame(width, height);

    while (!WindowShouldClose())
    {
        UpdateGame(&game);
        DrawGame(game);
    }

    CloseWindow();
    return 0;
}
