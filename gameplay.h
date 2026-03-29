#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "raylib.h"

/*
 * Draws a simple checker-style test map background.
 *
 * The map is rendered as 100x100 tiles using two alternating
 * green shades to improve visual readability during testing.
 * A border is then drawn around the full map area, followed by
 * an on-screen instruction label for movement controls.
 *
 * Parameters:
 * - mapWidth:  Total width of the map in pixels
 * - mapHeight: Total height of the map in pixels
 */
static inline void DrawMap1(int mapWidth, int mapHeight) {
    /*
     * Fill the map using alternating green tiles.
     * This creates a simple visual grid for gameplay testing.
     */
    for (int x = 0; x < mapWidth; x += 100) {
        for (int y = 0; y < mapHeight; y += 100) {
            if ((x + y) % 200 == 0) {
                DrawRectangle(x, y, 100, 100, (Color){ 34, 139, 34, 255 });
            } else {
                DrawRectangle(x, y, 100, 100, (Color){ 0, 100, 0, 255 });
            }
        }
    }

    /* Draw a visible border around the playable map area. */
    DrawRectangleLinesEx(
        (Rectangle){ 0, 0, (float)mapWidth, (float)mapHeight },
        10,
        MAROON
    );

    /* Display a simple instruction banner for test mode. */
    DrawText("MAP TESTING MODE - USE ARROWS TO MOVE", 20, 20, 20, RAYWHITE);
}

#endif