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
        .transitioningFloors = false,
        .turnCounter = 0
    };

    // placeholder player
    game.player = InitPlayer(0, 0, CELL_SIZE / 2, CELL_SIZE / 2, YELLOW);

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
            for (int i = 0; i < game->roomCount; i++)
            {
                if (game->rooms[i].type == ROOM_TYPE_START)
                {
                    game->playerPos.x = game->rooms[i].x + (game->rooms[i].width / 2);
                    game->playerPos.y = game->rooms[i].y + (game->rooms[i].height / 2);

                    // Set player's internal position
                    game->player.x = game->playerPos.x;
                    game->player.y = game->playerPos.y;

                    return true;
                }
            }

            // Fallback position if no start room was found
            if (game->roomCount > 0)
            {
                game->playerPos.x = game->rooms[0].x + (game->rooms[0].width / 2);
                game->playerPos.y = game->rooms[0].y + (game->rooms[0].height / 2);

                // Set player's internal position
                game->player.x = game->playerPos.x;
                game->player.y = game->playerPos.y;

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

    int targetX, targetY;
    ActionType actionType;

    if (HandlePlayerInput(&game->player, game->grid, &actionType, &targetX, &targetY))
    {
        game->turnCounter++;

        switch (actionType)
        {
            case ACTION_MOVE:
            {
                UpdatePlayerPosition(&game->player, targetX, targetY);
                game->playerPos.x = targetX;
                game->playerPos.y = targetY;

                printf("Player moved to (%d,%d) - Turn: %d\n",
                       targetX, targetY, game->turnCounter);
            }
            break;

            case ACTION_USE_STAIRS:
            {
                int playerCell = game->grid[game->player.y][game->player.x];

                if (playerCell == CELL_STAIR_DOWN)
                {
                    GoDownStairs(game);
                    return;
                }
                else if (playerCell == CELL_STAIR_UP && game->currentFloor > 1)
                {
                    GoUpStairs(game);
                    return;
                }
            }
            break;
        }
    }
}

void DrawGame(Game game)
{
    BeginDrawing();
    {
        ClearBackground(RAYWHITE);
        PrintDungeon(game.grid, game.rooms, game.roomCount);

        const int totalHeight = GRID_TOTAL_HEIGHT;
        const int totalWidth = GRID_TOTAL_WIDTH;
        const int startX = CENTER_SCREEN_X(totalWidth);
        const int startY = CENTER_SCREEN_Y(totalHeight);

        DrawCircle
        (
            startX + (game.playerPos.x * CELL_SIZE) + CELL_SIZE / 2,
            startY + (game.playerPos.y * CELL_SIZE) + CELL_SIZE / 2,
            CELL_SIZE / 3,
            YELLOW
        );

        char floorText[20];
        sprintf(floorText, "Floor: %d", game.currentFloor);
        DrawText(floorText, 40, 40, 30, BLACK);

        char turnText[20];
        sprintf(turnText, "Turn: %d", game.turnCounter);
        DrawText(turnText, 40, 100, 30, BLACK);

        DrawText("WASD/ARROW - MOVE", 40, 220, 26, DARKGRAY);
        DrawText("SPACE - USE STAIRCASE", 40, 260, 26, DARKGRAY);
    }
    EndDrawing();
}