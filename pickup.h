#ifndef PICKUP_H
#define PICKUP_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include "character.h"

#define HEART_COUNT 2
#define SPEED_COUNT 3
#define PICKUP_DRAW_SIZE 16.0f
#define PICKUP_HITBOX_SIZE 18.0f
#define PICKUP_MIN_SEPARATION 80.0f

typedef enum PickupType { PICKUP_HEART, PICKUP_SPEED } PickupType;

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

#endif // PICKUP_H

#ifdef PICKUP_IMPLEMENTATION
static Rectangle GetPickupHitbox(Vector2 pos) { return (Rectangle){ pos.x - PICKUP_HITBOX_SIZE * 0.5f, pos.y - PICKUP_HITBOX_SIZE * 0.5f, PICKUP_HITBOX_SIZE, PICKUP_HITBOX_SIZE }; }

static bool CanPlacePickupAt(Vector2 pos, const Tilemap *map) {
    Rectangle rect = GetPickupHitbox(pos);
    float mapW = (float)map->width * map->tileWidth, mapH = (float)map->height * map->tileHeight;
    if (rect.x < 0.0f || rect.y < 0.0f || rect.x + rect.width > mapW || rect.y + rect.height > mapH) return false;
    return !CheckMapCollision(map, rect);
}

static Vector2 FindRandomWalkablePoint(const Tilemap *map) {
    for (int attempt = 0; attempt < 1000; attempt++) {
        Vector2 candidate = { GetRandomValue(0, map->width - 1) * map->tileWidth + map->tileWidth * 0.5f, GetRandomValue(0, map->height - 1) * map->tileHeight + map->tileHeight * 0.5f };
        if (CanPlacePickupAt(candidate, map)) return candidate;
    }
    return FindWalkableSpawn(map);
}

void SpawnPickups(const Tilemap *map, Pickup hearts[], int heartCount, Texture2D *heartTexture, Pickup speeds[], int speedCount, Texture2D *speedTexture) {
    Pickup all[HEART_COUNT + SPEED_COUNT] = { 0 }; int totalPlaced = 0;
    for (int i = 0; i < heartCount; i++) { hearts[i].active = false; hearts[i].type = PICKUP_HEART; hearts[i].texture = heartTexture; }
    for (int i = 0; i < speedCount; i++) { speeds[i].active = false; speeds[i].type = PICKUP_SPEED; speeds[i].texture = speedTexture; }

    float minSepSq = PICKUP_MIN_SEPARATION * PICKUP_MIN_SEPARATION;
    for (int i = 0; i < heartCount; i++) {
        for (int attempt = 0; attempt < 1000; attempt++) {
            Vector2 pos = FindRandomWalkablePoint(map); bool ok = true;
            for (int j = 0; j < totalPlaced; j++) if (Vector2DistanceSqr(pos, all[j].pos) < minSepSq) { ok = false; break; }
            if (!ok) continue;
            hearts[i].pos = pos; hearts[i].hitbox = GetPickupHitbox(pos); hearts[i].active = true;
            all[totalPlaced++] = hearts[i]; break;
        }
    }
    for (int i = 0; i < speedCount; i++) {
        for (int attempt = 0; attempt < 1000; attempt++) {
            Vector2 pos = FindRandomWalkablePoint(map); bool ok = true;
            for (int j = 0; j < totalPlaced; j++) if (Vector2DistanceSqr(pos, all[j].pos) < minSepSq) { ok = false; break; }
            if (!ok) continue;
            speeds[i].pos = pos; speeds[i].hitbox = GetPickupHitbox(pos); speeds[i].active = true;
            all[totalPlaced++] = speeds[i]; break;
        }
    }
}

void CheckPickupCollisions(Player *player, Pickup hearts[], Pickup speeds[]) {
    Rectangle pBox = GetPlayerHitbox(player->pos);
    for (int i = 0; i < HEART_COUNT; i++) if (hearts[i].active && CheckCollisionRecs(pBox, hearts[i].hitbox)) { HealPlayer(player, 25.0f); hearts[i].active = false; }
    for (int i = 0; i < SPEED_COUNT; i++) if (speeds[i].active && CheckCollisionRecs(pBox, speeds[i].hitbox)) { player->speedMultiplier = 1.25f; AddPlayerEnergy(player, 20.0f); speeds[i].active = false; }
}

void DrawPickup(const Pickup *pickup) {
    if (!pickup->active || pickup->texture == NULL || pickup->texture->id <= 0) return;
    Rectangle src = { 0, 0, (float)pickup->texture->width, (float)pickup->texture->height }, dst = { pickup->pos.x, pickup->pos.y, PICKUP_DRAW_SIZE, PICKUP_DRAW_SIZE };
    Vector2 origin = { PICKUP_DRAW_SIZE / 2.0f, PICKUP_DRAW_SIZE / 2.0f };
    DrawTexturePro(*pickup->texture, src, dst, origin, 0.0f, WHITE);
}
#endif // PICKUP_IMPLEMENTATION