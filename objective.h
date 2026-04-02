#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include "character.h"

#define KEY_DRAW_SIZE 16.0f
#define KEY_HITBOX_SIZE 20.0f
#define KEY_SPAWN_DELAY 10.0f

typedef struct KeyItem {
    Texture2D texture;
    Vector2 pos;
    Rectangle hitbox;
    bool spawned;
    bool collected;
    float spawnTimer;
} KeyItem;

void ResetKey(KeyItem *key);
void UpdateKeyLogic(KeyItem *key, float dt, const Tilemap *map, Vector2 playerPos, bool *gameWon);
void DrawKey(const KeyItem *key);

#ifdef OBJECTIVE_IMPLEMENTATION

static Rectangle GetKeyHitbox(Vector2 pos) {
    return (Rectangle){
        pos.x - KEY_HITBOX_SIZE * 0.5f,
        pos.y - KEY_HITBOX_SIZE * 0.5f,
        KEY_HITBOX_SIZE,
        KEY_HITBOX_SIZE
    };
}

static Vector2 FindFurthestKeySpawn(const Tilemap *map, Vector2 playerPos) {
    Vector2 bestPos = playerPos;
    float bestDistSq = -1.0f;

    float mapW = (float)map->width * map->tileWidth;
    float mapH = (float)map->height * map->tileHeight;

    for (int row = 0; row < map->height; row++) {
        for (int col = 0; col < map->width; col++) {
            Vector2 candidate = {
                col * map->tileWidth + map->tileWidth * 0.5f,
                row * map->tileHeight + map->tileHeight * 0.5f
            };

            Rectangle rect = GetKeyHitbox(candidate);

            bool insideMap =
                rect.x >= 0.0f &&
                rect.y >= 0.0f &&
                rect.x + rect.width <= mapW &&
                rect.y + rect.height <= mapH;

            if (insideMap && !CheckMapCollision(map, rect)) {
                float distSq = Vector2DistanceSqr(candidate, playerPos);

                if (distSq > bestDistSq) {
                    bestDistSq = distSq;
                    bestPos = candidate;
                }
            }
        }
    }

    return bestPos;
}

void ResetKey(KeyItem *key) {
    key->pos = (Vector2){ 0.0f, 0.0f };
    key->hitbox = GetKeyHitbox(key->pos);
    key->spawned = false;
    key->collected = false;
    key->spawnTimer = KEY_SPAWN_DELAY;
}

void UpdateKeyLogic(KeyItem *key, float dt, const Tilemap *map, Vector2 playerPos, bool *gameWon) {
    key->spawnTimer -= dt;

    if (!key->spawned && key->spawnTimer <= 0.0f) {
        key->pos = FindFurthestKeySpawn(map, playerPos);
        key->hitbox = GetKeyHitbox(key->pos);
        key->spawned = true;
    }

    if (key->spawned &&
        !key->collected &&
        CheckCollisionRecs(GetPlayerHitbox(playerPos), key->hitbox)) {
        key->collected = true;
        *gameWon = true;
    }
}

void DrawKey(const KeyItem *key) {
    if (!key->spawned || key->collected || key->texture.id <= 0) {
        return;
    }

    Rectangle src = {
        0,
        0,
        (float)key->texture.width,
        (float)key->texture.height
    };

    Rectangle dst = {
        key->pos.x,
        key->pos.y,
        KEY_DRAW_SIZE,
        KEY_DRAW_SIZE
    };

    Vector2 origin = {
        KEY_DRAW_SIZE / 2.0f,
        KEY_DRAW_SIZE / 2.0f
    };

    DrawTexturePro(key->texture, src, dst, origin, 0.0f, WHITE);
}

#endif // OBJECTIVE_IMPLEMENTATION
#endif // OBJECTIVE_H // <--- Moved from the middle to the bottom!