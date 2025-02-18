#ifndef DUNGEON_H
#define DUNGEON_H

#define GRID_HEIGHT 300
#define GRID_WIDTH 300

#define ROOM_MAX_SIZE 30
#define ROOM_MIN_SIZE 10
#define ROOM_MIN_WIDTH 6
#define ROOM_MIN_HEIGHT 6

#define ROOM_AMOUNT 10

void GenerateDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);
void PrintDungeon(int grid[GRID_HEIGHT][GRID_WIDTH]);

typedef struct
{
    int x;
    int y;
    int width;
    int height;
} Room;


#endif //DUNGEON_H
