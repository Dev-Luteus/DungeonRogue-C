cmake_minimum_required(VERSION 3.30)
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
        include/Game.h)

# Link Raylib library (and required Windows libraries)
target_link_libraries(DungeonRogue_C raylib winmm)