#include "tilemap.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char *FindAfter(const char *text, const char *key) {
    const char *p = strstr(text, key);
    if (!p) return NULL;
    return p + strlen(key);
}

static int ReadIntAfter(const char *text, const char *key) {
    const char *p = FindAfter(text, key);
    if (!p) return 0;
    return atoi(p);
}

static bool ReadLayerDataByIndex(const char *json, int layerIndex, int out[MAP_MAX_HEIGHT][MAP_MAX_WIDTH], int width, int height) {
    const char *p = json;

    for (int i = 0; i <= layerIndex; i++) {
        p = strstr(p, "\"data\"");
        if (!p) return false;
        p += strlen("\"data\"");
    }

    p = strchr(p, '[');
    if (!p) return false;
    p++;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t' || *p == ',') {
                p++;
            }

            if (*p == '\0' || *p == ']') return false;

            out[y][x] = (int)strtol(p, (char **)&p, 10);
        }
    }

    return true;
}

static void DrawOneTile(Texture2D tex, int gid, int firstGid, int columns, int tileW, int tileH, int x, int y) {
    if (gid < firstGid || tex.id <= 0 || columns <= 0) return;

    int localId = gid - firstGid;
    int srcX = (localId % columns) * tileW;
    int srcY = (localId / columns) * tileH;

    Rectangle src = { (float)srcX, (float)srcY, (float)tileW, (float)tileH };
    Rectangle dst = { (float)x, (float)y, (float)tileW, (float)tileH };

    DrawTexturePro(tex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
}

bool LoadTilemap(Tilemap *map, const char *jsonFile) {
    memset(map, 0, sizeof(Tilemap));

    char *json = LoadFileText(jsonFile);
    if (!json) {
        TraceLog(LOG_ERROR, "Failed to load json file: %s", jsonFile);
        return false;
    }

    map->width = ReadIntAfter(json, "\"width\":");
    map->height = ReadIntAfter(json, "\"height\":");
    map->tileWidth = ReadIntAfter(json, "\"tilewidth\":");
    map->tileHeight = ReadIntAfter(json, "\"tileheight\":");

    if (map->width <= 0 || map->height <= 0 || map->width > MAP_MAX_WIDTH || map->height > MAP_MAX_HEIGHT) {
        TraceLog(LOG_ERROR, "Invalid map size");
        UnloadFileText(json);
        return false;
    }

    map->texWalls = LoadTexture("maps/map1/walls_floor.png");
    map->texDecor = LoadTexture("maps/map1/decorative_cracks_floor.png");
    map->texFloor = LoadTexture("maps/map1/floor_tile.png");

    if (map->texWalls.id <= 0 || map->texDecor.id <= 0 || map->texFloor.id <= 0) {
        TraceLog(LOG_ERROR, "Failed to load one or more tileset textures");
        UnloadFileText(json);
        return false;
    }

    map->gidWalls = 1;
    map->gidDecor = 494;
    map->gidFloor = 614;

    map->colsWalls = map->texWalls.width / map->tileWidth;
    map->colsDecor = map->texDecor.width / map->tileWidth;
    map->colsFloor = map->texFloor.width / map->tileWidth;

    bool okBlack = ReadLayerDataByIndex(json, 0, map->black, map->width, map->height);
    bool okFloor = ReadLayerDataByIndex(json, 1, map->floor, map->width, map->height);
    bool okWall  = ReadLayerDataByIndex(json, 2, map->wall,  map->width, map->height);

    UnloadFileText(json);

    if (!okBlack) TraceLog(LOG_ERROR, "Failed to read layer index 0 (black)");
    if (!okFloor) TraceLog(LOG_ERROR, "Failed to read layer index 1 (floor)");
    if (!okWall)  TraceLog(LOG_ERROR, "Failed to read layer index 2 (wall)");

    if (!okBlack || !okFloor || !okWall) return false;

    return true;
}

void UnloadTilemap(Tilemap *map) {
    if (map->texWalls.id > 0) UnloadTexture(map->texWalls);
    if (map->texDecor.id > 0) UnloadTexture(map->texDecor);
    if (map->texFloor.id > 0) UnloadTexture(map->texFloor);
}

void DrawTilemapGround(const Tilemap *map) {
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            int worldX = x * map->tileWidth;
            int worldY = y * map->tileHeight;

            if (map->black[y][x] != 0) {
                DrawOneTile(map->texWalls, map->black[y][x], map->gidWalls, map->colsWalls,
                            map->tileWidth, map->tileHeight, worldX, worldY);
            }

            if (map->floor[y][x] != 0) {
                int gid = map->floor[y][x];

                if (gid >= map->gidFloor) {
                    DrawOneTile(map->texFloor, gid, map->gidFloor, map->colsFloor,
                                map->tileWidth, map->tileHeight, worldX, worldY);
                } else if (gid >= map->gidDecor) {
                    DrawOneTile(map->texDecor, gid, map->gidDecor, map->colsDecor,
                                map->tileWidth, map->tileHeight, worldX, worldY);
                } else {
                    DrawOneTile(map->texWalls, gid, map->gidWalls, map->colsWalls,
                                map->tileWidth, map->tileHeight, worldX, worldY);
                }
            }
        }
    }
}

void DrawTilemapWalls(const Tilemap *map) {
    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            int worldX = x * map->tileWidth;
            int worldY = y * map->tileHeight;

            if (map->wall[y][x] != 0) {
                DrawOneTile(map->texWalls, map->wall[y][x], map->gidWalls, map->colsWalls,
                            map->tileWidth, map->tileHeight, worldX, worldY);
            }
        }
    }
}

void DrawTilemapAll(const Tilemap *map) {
    DrawTilemapGround(map);
    DrawTilemapWalls(map);
}

bool CheckMapCollision(const Tilemap *map, Rectangle rect) {
    int left   = (int)(rect.x) / map->tileWidth;
    int right  = (int)(rect.x + rect.width - 1) / map->tileWidth;
    int top    = (int)(rect.y) / map->tileHeight;
    int bottom = (int)(rect.y + rect.height - 1) / map->tileHeight;

    if (left < 0 || top < 0 || right >= map->width || bottom >= map->height) {
        return true;
    }

    for (int y = top; y <= bottom; y++) {
        for (int x = left; x <= right; x++) {
            if (map->wall[y][x] != 0) {
                Rectangle wallRect = {
                    (float)(x * map->tileWidth),
                    (float)(y * map->tileHeight),
                    (float)map->tileWidth,
                    (float)map->tileHeight
                };

                if (CheckCollisionRecs(rect, wallRect)) {
                    return true;
                }
            }
        }
    }

    return false;
}

Vector2 FindWalkableSpawn(const Tilemap *map) {
    for (int y = 1; y < map->height - 1; y++) {
        for (int x = 1; x < map->width - 1; x++) {
            if (map->wall[y][x] == 0 && map->floor[y][x] != 0) {
                return (Vector2){
                    x * map->tileWidth + map->tileWidth * 0.5f,
                    y * map->tileHeight + map->tileHeight * 0.5f
                };
            }
        }
    }

    return (Vector2){ 100.0f, 100.0f };
}