﻿cmake_minimum_required(VERSION 3.30)
project(DungeonRogue_C C)

set(CMAKE_C_STANDARD 11)

# Add the include directory for headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add the library directory for linking
link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(DungeonRogue_C main.c
        Dungeon.c
        include/Dungeon.h
        Game.c
        include/Game.h
        Path.c
        include/Path.h
        Staircase.c
        include/Room.h
        Room.c
        include/Corridor.h
        Corridor.c
        include/Door.h
        Door.c
        include/Staircase.h
        include/DungeonDefs.h
        include/Player.h
        Player.c
)

# Link Raylib library (and required Windows libraries)
target_link_libraries(DungeonRogue_C raylib winmm)