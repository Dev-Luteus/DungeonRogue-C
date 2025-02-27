#include "Raylib.h"
#include "Game.h"

#include <stdio.h>
#include <string.h>

#include "Dungeon.h"

Game InitGame(int width, int height)
{
    Game game =
    {
        .screenWidth = width,
        .screenHeight = height,
        .grid = {0}, // default initialization
        .generationAttempts = 0,
        .currentFloor = 1,  // Starting floor!
        .playerPos = {0, 0},
        .transitioningFloors = false
    };

    return game;
}

bool GenerateFloor(Game* game)
{
    const int MAX_GENERATION_ATTEMPTS = 5;
    game->generationAttempts = 0;

    while (game->generationAttempts < MAX_GENERATION_ATTEMPTS)
    {
        game->generationAttempts++;

        // Clear the grid for fresh generation
        memset(game->grid, 0, sizeof(game->grid));

        if (GenerateDungeon(game->grid, MAX_GENERATION_ATTEMPTS, game->currentFloor,
                            game->rooms, &game->roomCount))
        {
            printf("Floor %d generated successfully on attempt %d\n",
                   game->currentFloor, game->generationAttempts);

            // Find player start position (should be in the start room)
            for (int i = 0; i < game->roomCount; i++) {
                if (game->rooms[i].type == ROOM_TYPE_START) {
                    // Place player in center of start room
                    game->playerPos.x = game->rooms[i].x + (game->rooms[i].width / 2);
                    game->playerPos.y = game->rooms[i].y + (game->rooms[i].height / 2);
                    return true;
                }
            }

            // Fallback position if no start room was found
            if (game->roomCount > 0) {
                game->playerPos.x = game->rooms[0].x + (game->rooms[0].width / 2);
                game->playerPos.y = game->rooms[0].y + (game->rooms[0].height / 2);
                return true;
            }
        }
    }

    printf("Failed to generate floor %d after %d attempts\n",
           game->currentFloor, MAX_GENERATION_ATTEMPTS);
    return false;
}

void GoDownStairs(Game* game)
{
    game->currentFloor++;
    game->transitioningFloors = true;
    printf("Going down to floor %d\n", game->currentFloor);
}

void GoUpStairs(Game* game)
{
    if (game->currentFloor > 1)
    {
        game->currentFloor--;
        game->transitioningFloors = true;
        printf("Going up to floor %d\n", game->currentFloor);
    }
    else
    {
        printf("Already on the top floor!\n");
    }
}

void UpdateGame(Game* game)
{
    if (!game->dungeonGenerated || game->transitioningFloors)
    {
        if (GenerateFloor(game))
        {
            game->dungeonGenerated = true;
            game->transitioningFloors = false;
        }
        return;
    }

    // Check for player interaction with stairs
    int playerX = game->playerPos.x;
    int playerY = game->playerPos.y;
    int playerCell = game->grid[playerY][playerX];

    // Handle keyboard for demo/debug
    if (IsKeyPressed(KEY_DOWN))
    {
        // Find a stair down cell and check if player is on it
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            for (int x = 0; x < GRID_WIDTH; x++)
            {
                if (game->grid[y][x] == CELL_STAIR_DOWN)
                {
                    // Move player to stairs for demonstration
                    game->playerPos.x = x;
                    game->playerPos.y = y;
                    GoDownStairs(game);
                    return;
                }
            }
        }
    }

    if (IsKeyPressed(KEY_UP) && game->currentFloor > 1)
    {
        // Find a stair up cell and check if player is on it
        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            for (int x = 0; x < GRID_WIDTH; x++)
            {
                if (game->grid[y][x] == CELL_STAIR_UP)
                {
                    // Move player to stairs for demonstration
                    game->playerPos.x = x;
                    game->playerPos.y = y;
                    GoUpStairs(game);
                    return;
                }
            }
        }
    }

    // In a real game, you'd check if the player is standing on stairs
    // and only trigger floor transitions when the player presses an action key
    /*
    if (playerCell == CELL_STAIR_DOWN && IsKeyPressed(KEY_SPACE)) {
        GoDownStairs(game);
    } else if (playerCell == CELL_STAIR_UP && IsKeyPressed(KEY_SPACE) && game->currentFloor > 1) {
        GoUpStairs(game);
    }
    */
}

void DrawGame (Game game)
{
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);
        PrintDungeon(game.grid, game.rooms, game.roomCount);

        // Draw player as a yellow circle
        const int totalHeight = GRID_TOTAL_HEIGHT;
        const int totalWidth = GRID_TOTAL_WIDTH;
        const int startX = CENTER_SCREEN_X(totalWidth);
        const int startY = CENTER_SCREEN_Y(totalHeight);

        DrawCircle(
            startX + (game.playerPos.x * CELL_SIZE) + CELL_SIZE / 2,
            startY + (game.playerPos.y * CELL_SIZE) + CELL_SIZE / 2,
            CELL_SIZE / 3,
            YELLOW
        );

        // Draw floor number
        char floorText[20];
        sprintf(floorText, "Floor: %d", game.currentFloor);
        DrawText(floorText, 10, 10, 20, BLACK);

        // Draw help text
        DrawText("Press UP/DOWN keys to change floors", 10, 40, 16, DARKGRAY);
    }
    EndDrawing();
}