#include "Raylib.h"
#include "Game.h"

#include <stdio.h>
#include <string.h>

#include "Dungeon.h"

Game InitGame (int width, int height)
{
    const Game game =
    {
        .screenWidth = width,
        .screenHeight = height,
        .grid = {0}, // default initialization
        .generationAttempts = 0
    };

    return game;
}

void UpdateGame(Game* game)
{
    if (!game->dungeonGenerated)
    {
        const int MAX_GENERATION_ATTEMPTS = 5;

        if (game->generationAttempts >= MAX_GENERATION_ATTEMPTS)
        {
            printf("Failed to generate dungeon after %d attempts\n", MAX_GENERATION_ATTEMPTS);
            game->dungeonGenerated = true;  // Force exit generation loop
            return;
        }

        game->generationAttempts++;

        if (GenerateDungeon(game->grid, MAX_GENERATION_ATTEMPTS))
        {
            game->dungeonGenerated = true;
            printf("Dungeon generated successfully on attempt %d\n", game->generationAttempts);
        }
        else
        {
            // Clear the grid for next attempt
            memset(game->grid, 0, sizeof(game->grid));
        }
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