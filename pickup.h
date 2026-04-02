#ifndef PICKUP_H
#define PICKUP_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include "character.h"

#define HEART_COUNT 2
#define SPEED_COUNT 3
#define HEART_DRAW_WIDTH   16.0f
#define HEART_DRAW_HEIGHT  16.0f
#define SPEED_DRAW_WIDTH   32.0f
#define SPEED_DRAW_HEIGHT  16.0f
#define PICKUP_MIN_SEPARATION 80.0f

typedef enum PickupType {
    PICKUP_HEART,
    PICKUP_SPEED
} PickupType;

typedef struct Pickup {
    Texture2D *texture;
    Vector2 pos;
    Rectangle hitbox;
    bool active;
    PickupType type;
} Pickup;

void SpawnPickups(const Tilemap *map, Pickup hearts[], int heartCount, Texture2D *heartTexture, Pickup speeds[], int speedCount, Texture2D *speedTexture);
void DrawPickup(const Pickup *pickup);
void CheckPickupCollisions(Player *player, Pickup hearts[], Pickup speeds[]);

#ifdef PICKUP_IMPLEMENTATION

static float GetPickupDrawWidth(PickupType type) {
    return (type == PICKUP_SPEED) ? SPEED_DRAW_WIDTH : HEART_DRAW_WIDTH;
}

static float GetPickupDrawHeight(PickupType type) {
    return (type == PICKUP_SPEED) ? SPEED_DRAW_HEIGHT : HEART_DRAW_HEIGHT;
}

static Rectangle GetPickupHitboxByType(Vector2 pos, PickupType type) {
    float width = GetPickupDrawWidth(type);
    float height = GetPickupDrawHeight(type);

    return (Rectangle){
        pos.x - width * 0.5f,
        pos.y - height * 0.5f,
        width,
        height
    };
}

static bool CanPlacePickupAt(Vector2 pos, PickupType type, const Tilemap *map) {
    Rectangle rect = GetPickupHitboxByType(pos, type);
    float mapW = (float)map->width * map->tileWidth;
    float mapH = (float)map->height * map->tileHeight;

    if (rect.x < 0.0f || rect.y < 0.0f || rect.x + rect.width > mapW || rect.y + rect.height > mapH) {
        return false;
    }

    return !CheckMapCollision(map, rect);
}

static Vector2 FindRandomWalkablePoint(const Tilemap *map, PickupType type) {
    for (int attempt = 0; attempt < 1000; attempt++) {
        Vector2 candidate = {
            GetRandomValue(0, map->width - 1) * map->tileWidth + map->tileWidth * 0.5f,
            GetRandomValue(0, map->height - 1) * map->tileHeight + map->tileHeight * 0.5f
        };

        if (CanPlacePickupAt(candidate, type, map)) {
            return candidate;
        }
    }

    return FindWalkableSpawn(map);
}

void SpawnPickups(const Tilemap *map, Pickup hearts[], int heartCount, Texture2D *heartTexture, Pickup speeds[], int speedCount, Texture2D *speedTexture) {
    Pickup all[HEART_COUNT + SPEED_COUNT] = { 0 };
    int totalPlaced = 0;

    for (int i = 0; i < heartCount; i++) {
        hearts[i].active = false;
        hearts[i].type = PICKUP_HEART;
        hearts[i].texture = heartTexture;
    }

    for (int i = 0; i < speedCount; i++) {
        speeds[i].active = false;
        speeds[i].type = PICKUP_SPEED;
        speeds[i].texture = speedTexture;
    }

    float minSepSq = PICKUP_MIN_SEPARATION * PICKUP_MIN_SEPARATION;

    for (int i = 0; i < heartCount; i++) {
        for (int attempt = 0; attempt < 1000; attempt++) {
            Vector2 pos = FindRandomWalkablePoint(map, PICKUP_HEART);
            bool ok = true;

            for (int j = 0; j < totalPlaced; j++) {
                if (Vector2DistanceSqr(pos, all[j].pos) < minSepSq) {
                    ok = false;
                    break;
                }
            }

            if (!ok) {
                continue;
            }

            hearts[i].pos = pos;
            hearts[i].hitbox = GetPickupHitboxByType(pos, hearts[i].type);
            hearts[i].active = true;
            all[totalPlaced++] = hearts[i];
            break;
        }
    }

    for (int i = 0; i < speedCount; i++) {
        for (int attempt = 0; attempt < 1000; attempt++) {
            Vector2 pos = FindRandomWalkablePoint(map, PICKUP_SPEED);
            bool ok = true;

            for (int j = 0; j < totalPlaced; j++) {
                if (Vector2DistanceSqr(pos, all[j].pos) < minSepSq) {
                    ok = false;
                    break;
                }
            }

            if (!ok) {
                continue;
            }

            speeds[i].pos = pos;
            speeds[i].hitbox = GetPickupHitboxByType(pos, speeds[i].type);
            speeds[i].active = true;
            all[totalPlaced++] = speeds[i];
            break;
        }
    }
}

void CheckPickupCollisions(Player *player, Pickup hearts[], Pickup speeds[]) {
    Rectangle pBox = GetPlayerHitbox(player->pos);

    for (int i = 0; i < HEART_COUNT; i++) {
        if (hearts[i].active && CheckCollisionRecs(pBox, hearts[i].hitbox)) {
            HealPlayer(player, 25.0f);
            hearts[i].active = false;
        }
    }

    for (int i = 0; i < SPEED_COUNT; i++) {
        if (speeds[i].active && CheckCollisionRecs(pBox, speeds[i].hitbox)) {
            player->speedMultiplier = 1.25f;
            AddPlayerEnergy(player, 20.0f);
            speeds[i].active = false;
        }
    }
}

void DrawPickup(const Pickup *pickup) {
    if (!pickup->active || pickup->texture == NULL || pickup->texture->id <= 0) {
        return;
    }

    float drawWidth = GetPickupDrawWidth(pickup->type);
    float drawHeight = GetPickupDrawHeight(pickup->type);

    Rectangle src = {
        0,
        0,
        (float)pickup->texture->width,
        (float)pickup->texture->height
    };

    Rectangle dst = {
        pickup->pos.x,
        pickup->pos.y,
        drawWidth,
        drawHeight
    };

    Vector2 origin = {
        drawWidth / 2.0f,
        drawHeight / 2.0f
    };

    DrawTexturePro(*pickup->texture, src, dst, origin, 0.0f, WHITE);
}

#endif // PICKUP_IMPLEMENTATION
#endif // PICKUP_H // <--- Moved from the middle to the bottom!