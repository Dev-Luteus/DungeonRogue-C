#include "Player.h"

Player InitPlayer(int x, int y, int width, int height, Color color)
{
    Player player =
    {
        x,
        y,
        width,
        height,
        color
    };

    return player;
}

void UpdatePlayerPosition(Player* player, int x, int y)
{
    player->x = x;
    player->y = y;
}

bool IsValidPlayerPosition(int gridData[GRID_HEIGHT][GRID_WIDTH], int x, int y)
{
    if (!IS_IN_GRID(x, y))
    {
        return false;
    }

    /* Get cell value at the specified position */
    const int cellValue = gridData[y][x];

    return (IS_ROOM(cellValue) ||
            cellValue == CELL_CORRIDOR ||
            cellValue == CELL_PATH ||
            cellValue == CELL_DOOR ||
            cellValue == CELL_STAIR_UP ||
            cellValue == CELL_STAIR_DOWN);
}

bool HandleMovementInput(Player* player, int gridData[GRID_HEIGHT][GRID_WIDTH],
                        int* targetX, int* targetY)
{
    /* Current position */
    *targetX = player->x;
    *targetY = player->y;

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
    {
        *targetY = player->y - 1;
    }
    else if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
    {
        *targetY = player->y + 1;
    }

    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
    {
        *targetX = player->x - 1;
    }
    else if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
    {
        *targetX = player->x + 1;
    }

    if (*targetX == player->x && *targetY == player->y)
    {
        return false;
    }

    return IsValidPlayerPosition(gridData, *targetX, *targetY);
}

ActionType HandleAction(Player* player, int gridData[GRID_HEIGHT][GRID_WIDTH])
{
    if (IsKeyPressed(KEY_SPACE))
    {
        int currentCell = gridData[player->y][player->x];

        if (currentCell == CELL_STAIR_UP || currentCell == CELL_STAIR_DOWN)
        {
            return ACTION_USE_STAIRS;
        }
    }

    return ACTION_NONE;
}

/* Our main input handler
 */
bool HandlePlayerInput(Player* player, int gridData[GRID_HEIGHT][GRID_WIDTH],
                      ActionType* actionType, int* targetX, int* targetY)
{
    // Default
    *targetX = player->x;
    *targetY = player->y;
    *actionType = ACTION_NONE;

    // Action first
    ActionType action = HandleAction(player, gridData);

    if (action != ACTION_NONE)
    {
        *actionType = action;
        return true; // Valid action
    }

    // Movement second
    if (HandleMovementInput(player, gridData, targetX, targetY))
    {
        *actionType = ACTION_MOVE;
        return true; // Valid movement
    }

    // Safety check just in case^^
    return false;
}