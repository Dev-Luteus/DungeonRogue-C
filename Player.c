#include <Player.h>

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

void ReadPlayerInput(Player* player)
{
    if (IsKeyDown(KEY_LEFT))
    {
        player->x -= 1;
    }
    if (IsKeyDown(KEY_RIGHT))
    {
        player->x += 1;
    }
    if (IsKeyDown(KEY_UP))
    {
        player->y -= 1;
    }
    if (IsKeyDown(KEY_DOWN))
    {
        player->y += 1;
    }
}

void UpdatePlayerPosition(Player* player, int x, int y)
{
    player->x = x;
    player->y = y;
}