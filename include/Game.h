#ifndef GAME_H
#define GAME_H
#include "Dungeon.h"

typedef struct
{
    int screenWidth;
    int screenHeight;
    int grid[GRID_HEIGHT][GRID_WIDTH];
    bool dungeonGenerated;
    int generationAttempts;
} Game;

Game InitGame(int width, int height);
void UpdateGame(Game* game);
void DrawGame(Game game);

#endif //GAME_H
