#ifndef TILEMAP_H
#define TILEMAP_H

#include "raylib.h"
#include <stdbool.h>

#define MAP_MAX_WIDTH  100
#define MAP_MAX_HEIGHT 100

typedef struct Tilemap {
    int width;
    int height;
    int tileWidth;
    int tileHeight;

    int wall[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
    int floor[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
    int black[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];

    Texture2D texWalls;
    Texture2D texDecor;
    Texture2D texFloor;

    int gidWalls;
    int gidDecor;
    int gidFloor;

    int colsWalls;
    int colsDecor;
    int colsFloor;
} Tilemap;

bool LoadTilemap(Tilemap *map, const char *jsonFile);
void UnloadTilemap(Tilemap *map);

void DrawTilemapGround(const Tilemap *map);
void DrawTilemapWalls(const Tilemap *map);
void DrawTilemapAll(const Tilemap *map);

bool CheckMapCollision(const Tilemap *map, Rectangle rect);
Vector2 FindWalkableSpawn(const Tilemap *map);

#endif