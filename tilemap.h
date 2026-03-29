#ifndef TILEMAP_H
#define TILEMAP_H

#include "raylib.h"
#include <stdbool.h>

/*
 * Maximum supported tilemap dimensions.
 * These limits define the fixed-size storage used by the map layers.
 */
#define MAP_MAX_WIDTH  100
#define MAP_MAX_HEIGHT 100

/*
 * Stores tilemap data, textures, and rendering information.
 *
 * Fields:
 * - width, height: Number of tiles in the map
 * - tileWidth, tileHeight: Size of each tile in pixels
 *
 * Layer data:
 * - wall: Solid tiles used for blocking movement
 * - floor: Ground tiles used as the base visual layer
 * - black: Additional layer, typically used for masking, darkness, or overlays
 *
 * Texture data:
 * - texWalls: Tileset texture used for wall tiles
 * - texDecor: Tileset texture used for decorative or overlay tiles
 * - texFloor: Tileset texture used for floor tiles
 *
 * Tileset metadata:
 * - gidWalls, gidDecor, gidFloor: First global tile ID for each tileset
 * - colsWalls, colsDecor, colsFloor: Number of columns in each tileset texture
 */
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

/*
 * Load tilemap data and related textures from a JSON map file.
 *
 * Returns:
 * - true if the map was loaded successfully
 * - false if loading failed
 */
bool LoadTilemap(Tilemap *map, const char *jsonFile);

/*
 * Release all resources associated with the tilemap.
 */
void UnloadTilemap(Tilemap *map);

/*
 * Draw only the ground/floor layer of the map.
 */
void DrawTilemapGround(const Tilemap *map);

/*
 * Draw only the wall or upper visual layers of the map.
 */
void DrawTilemapWalls(const Tilemap *map);

/*
 * Draw the complete map using all required layers.
 */
void DrawTilemapAll(const Tilemap *map);

/*
 * Check whether a rectangle collides with any blocking map tile.
 *
 * Returns:
 * - true if the rectangle overlaps a blocked area
 * - false if the area is walkable
 */
bool CheckMapCollision(const Tilemap *map, Rectangle rect);

/*
 * Find a valid walkable spawn position on the map.
 *
 * Returns:
 * - a world-space position suitable for spawning an entity
 */
Vector2 FindWalkableSpawn(const Tilemap *map);

#endif