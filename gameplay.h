#ifndef GAMEPLAY_H
#define GAMEPLAY_H
#include "raylib.h"

static inline void DrawMap1(int mapWidth, int mapHeight) {
    for (int x = 0; x < mapWidth; x += 100) {
        for (int y = 0; y < mapHeight; y += 100) {
            // color green dark and green
            if ((x + y) % 200 == 0) 
                DrawRectangle(x, y, 100, 100, (Color){ 34, 139, 34, 255 }); // Forest Green
            else 
                DrawRectangle(x, y, 100, 100, (Color){ 0, 100, 0, 255 });    // Dark Green
        }
    }

    // basic border
    DrawRectangleLinesEx((Rectangle){ 0, 0, (float)mapWidth, (float)mapHeight }, 10, MAROON);

    DrawText("MAP TESTING MODE - USE ARROWS TO MOVE", 20, 20, 20, RAYWHITE);
}
#endif