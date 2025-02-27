﻿#ifndef GAME_H
#define GAME_H

#include "DungeonDefs.h"
#include "Room.h"
#include "Corridor.h"

typedef struct
{
    int screenWidth;
    int screenHeight;
    int grid[GRID_HEIGHT][GRID_WIDTH];
    bool dungeonGenerated;
    int generationAttempts;

    Corridor playerPos;

    // Rooms
    int currentFloor;
    bool transitioningFloors;
    Room rooms[ROOM_AMOUNT];
    int roomCount;
} Game;

Game InitGame(int width, int height);
void UpdateGame(Game* game);
void DrawGame(Game game);

// Floor transition helpers
void GoDownStairs(Game* game);
void GoUpStairs(Game* game);
bool GenerateFloor(Game* game);

#endif //GAME_H