#ifndef GAME_H
#define GAME_H

typedef struct
{
    int screenWidth;
    int screenHeight;
} Game;

Game InitGame(int width, int height);
void UpdateGame(Game* game);
void DrawGame(Game game);

#endif //GAME_H
