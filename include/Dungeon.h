﻿#ifndef DUNGEON_H
#define DUNGEON_H

#define GRID_HEIGHT 100
#define GRID_WIDTH 100
#define CELL_SIZE 10
#define GRID_TOTAL_HEIGHT GRID_HEIGHT * CELL_SIZE
#define GRID_TOTAL_WIDTH GRID_WIDTH * CELL_SIZE

void GenerateDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);
void PrintDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);

#define ROOM_MAX_SIZE 30
#define ROOM_MIN_SIZE 10
#define ROOM_MIN_WIDTH 6
#define ROOM_MIN_HEIGHT 6
#define ROOM_AMOUNT 10

typedef struct
{
    int x;
    int y;
    int width;
    int height;
} Room;

void GenerateGrid(int grid[GRID_HEIGHT][GRID_WIDTH]);

Room CreateRoom(int x, int y, int width, int height);
void GenerateRooms(int grid[GRID_HEIGHT][GRID_WIDTH]);
bool GenerateRoom(int grid[GRID_HEIGHT][GRID_WIDTH]);
bool IsRoomValid(int grid[GRID_HEIGHT][GRID_WIDTH], Room room);

void PlaceRoom(int grid[GRID_HEIGHT][GRID_WIDTH], Room room);
void GenerateDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);

#endif //DUNGEON_H
