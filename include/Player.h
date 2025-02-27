#ifndef PLAYER_H
#define PLAYER_H
#include <Raylib.h>

typedef struct
{
    int x;
    int y;
    int width;
    int height;
    Color color;
} Player;

Player InitPlayer(int x, int y, int width, int height, Color color);

void ReadPlayerInput(Player* player);
void UpdatePlayerPosition(Player* player, int x, int y);


#endif //PLAYER_H
