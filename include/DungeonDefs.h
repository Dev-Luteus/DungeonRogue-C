#ifndef DUNGEONDEFS_H
#define DUNGEONDEFS_H

// Cell Types
#define CELL_EMPTY_1 0 // Empty
#define CELL_EMPTY_2 1 // Empty 2 (currently just for pattern)
#define CELL_ROOM 2
#define CELL_CORRIDOR 3
#define CELL_DOOR 4
#define CELL_PATH 5
#define CELL_STAIR_UP 6
#define CELL_STAIR_DOWN 7
#define ROOM_ID_START 10 // ID 10 and Up

// Room Identifiers
#define ROOM_TYPE_NORMAL 0
#define ROOM_TYPE_START 1
#define ROOM_TYPE_BOSS 2

// General
// The Height and Width should be an ODD number
#define GRID_HEIGHT 69
#define GRID_WIDTH 69
#define CELL_SIZE 15
#define GRID_TOTAL_HEIGHT (GRID_HEIGHT * CELL_SIZE)
#define GRID_TOTAL_WIDTH (GRID_WIDTH * CELL_SIZE)

// Grid utility macros
#define GRID_SIZE (GRID_WIDTH * GRID_HEIGHT)
#define HALF(x) ((x) >> 1)
#define CENTER_SCREEN_X(width) ((GetScreenWidth() - (width)) >> 1)
#define CENTER_SCREEN_Y(height) ((GetScreenHeight() - (height)) >> 1)
#define IS_IN_GRID(x, y) ((x) >= 0 && (x) < GRID_WIDTH && (y) >= 0 && (y) < GRID_HEIGHT)
#define IS_ROOM(cell) ((cell) >= ROOM_ID_START)
#define IS_EMPTY(cell) ((cell) == CELL_EMPTY_1 || (cell) == CELL_EMPTY_2)
#define GET_GRID_INDEX(x, y) ((y) * GRID_WIDTH + (x))
#define IS_VALID_CELL(x, y) ((x) >= 1 && (x) < GRID_WIDTH - 1 && (y) >= 1 && (y) < GRID_HEIGHT - 1)
#define CAN_BE_PATH(cell) ((cell) == CELL_CORRIDOR || (cell) == CELL_EMPTY_1 || (cell) == CELL_EMPTY_2)

#endif // DUNGEONDEFS_H