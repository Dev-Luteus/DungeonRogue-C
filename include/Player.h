#ifndef PLAYER_H
#define PLAYER_H

#include <Raylib.h>
#include "DungeonDefs.h"
#include <stdbool.h>

typedef enum {
    ACTION_NONE,
    ACTION_MOVE,
    ACTION_USE_STAIRS,
} ActionType;

typedef struct
{
    int x;
    int y;
    int width;
    int height;
    Color color;
} Player;

Player InitPlayer(int x, int y, int width, int height, Color color);
void UpdatePlayerPosition(Player* player, int x, int y);

bool IsValidPlayerPosition(int gridData[GRID_HEIGHT][GRID_WIDTH], int x, int y);
bool HandleMovementInput(Player* player, int gridData[GRID_HEIGHT][GRID_WIDTH],
                        int* targetX, int* targetY);

ActionType HandleAction(Player* player, int gridData[GRID_HEIGHT][GRID_WIDTH]);

bool HandlePlayerInput(Player* player, int gridData[GRID_HEIGHT][GRID_WIDTH],
                      ActionType* actionType, int* targetX, int* targetY);

#endif //PLAYER_H