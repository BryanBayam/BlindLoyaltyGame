#ifndef PICKUP_H
#define PICKUP_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include "character.h"

/*
 * Number of pickup instances to spawn for each pickup category.
 */
#define HEART_COUNT 2
#define SPEED_COUNT 3

/*
 * Render size for heart pickups.
 */
#define HEART_DRAW_WIDTH   16.0f
#define HEART_DRAW_HEIGHT  16.0f

/*
 * Render size for speed pickups.
 */
#define SPEED_DRAW_WIDTH   32.0f
#define SPEED_DRAW_HEIGHT  16.0f

/*
 * Minimum distance allowed between spawned pickups.
 * This helps prevent item overlap and improves map distribution.
 */
#define PICKUP_MIN_SEPARATION 80.0f

/*
 * Pickup category used to determine appearance and effect.
 *
 * PICKUP_HEART : Restores player health
 * PICKUP_SPEED : Increases movement speed and restores some energy
 */
typedef enum PickupType {
    PICKUP_HEART,
    PICKUP_SPEED
} PickupType;

/*
 * Represents a single collectible pickup in the world.
 *
 * Fields:
 * - texture: Texture used when drawing the pickup
 * - pos: World position of the pickup
 * - hitbox: Collision area used for pickup detection
 * - active: True if the pickup is currently available in the world
 * - type: Pickup category that controls its behavior and draw size
 */
typedef struct Pickup {
    Texture2D *texture;
    Vector2 pos;
    Rectangle hitbox;
    bool active;
    PickupType type;
} Pickup;

/*
 * Spawn and initialize all heart and speed pickups on valid map positions.
 */
void SpawnPickups(const Tilemap *map, Pickup hearts[], int heartCount, Texture2D *heartTexture, Pickup speeds[], int speedCount, Texture2D *speedTexture);

/*
 * Draw a pickup if it is active and has a valid texture.
 */
void DrawPickup(const Pickup *pickup);

/*
 * Check whether the player collides with any active pickup
 * and apply the corresponding effect.
 */
void CheckPickupCollisions(Player *player, Pickup hearts[], Pickup speeds[]);

#endif

#ifdef PICKUP_IMPLEMENTATION

/*
 * Return the draw width for a pickup based on its type.
 */
static float GetPickupDrawWidth(PickupType type) {
    return (type == PICKUP_SPEED) ? SPEED_DRAW_WIDTH : HEART_DRAW_WIDTH;
}

/*
 * Return the draw height for a pickup based on its type.
 */
static float GetPickupDrawHeight(PickupType type) {
    return (type == PICKUP_SPEED) ? SPEED_DRAW_HEIGHT : HEART_DRAW_HEIGHT;
}

/*
 * Build the collision hitbox for a pickup based on its
 * centered world position and pickup type.
 */
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

/*
 * Check whether a pickup can be placed at the given position.
 *
 * Conditions:
 * - the full pickup hitbox must stay inside map bounds
 * - the pickup must not overlap blocked map tiles
 */
static bool CanPlacePickupAt(Vector2 pos, PickupType type, const Tilemap *map) {
    Rectangle rect = GetPickupHitboxByType(pos, type);
    float mapW = (float)map->width * map->tileWidth;
    float mapH = (float)map->height * map->tileHeight;

    if (rect.x < 0.0f || rect.y < 0.0f || rect.x + rect.width > mapW || rect.y + rect.height > mapH) {
        return false;
    }

    return !CheckMapCollision(map, rect);
}

/*
 * Find a random valid world position for a pickup of the given type.
 *
 * The function retries several times to find a valid location.
 * If no random position is found, it falls back to a general
 * walkable spawn position from the map system.
 */
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

/*
 * Spawn and initialize heart and speed pickups.
 *
 * Spawn rules:
 * - all pickups must be placed on valid walkable positions
 * - pickups must not be placed too close to each other
 * - each pickup is initialized with type, texture, position, and hitbox
 */
void SpawnPickups(const Tilemap *map, Pickup hearts[], int heartCount, Texture2D *heartTexture, Pickup speeds[], int speedCount, Texture2D *speedTexture) {
    Pickup all[HEART_COUNT + SPEED_COUNT] = { 0 };
    int totalPlaced = 0;

    /* Initialize heart pickup metadata before placement. */
    for (int i = 0; i < heartCount; i++) {
        hearts[i].active = false;
        hearts[i].type = PICKUP_HEART;
        hearts[i].texture = heartTexture;
    }

    /* Initialize speed pickup metadata before placement. */
    for (int i = 0; i < speedCount; i++) {
        speeds[i].active = false;
        speeds[i].type = PICKUP_SPEED;
        speeds[i].texture = speedTexture;
    }

    float minSepSq = PICKUP_MIN_SEPARATION * PICKUP_MIN_SEPARATION;

    /* Place heart pickups while respecting minimum spacing. */
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

    /* Place speed pickups while respecting minimum spacing. */
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

/*
 * Check collision between the player and all active pickups.
 *
 * Effects:
 * - Heart pickup restores health
 * - Speed pickup increases speed multiplier and restores energy
 *
 * Collected pickups are deactivated after use.
 */
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

/*
 * Draw a pickup using its configured texture and size.
 * Inactive pickups or invalid textures are ignored.
 */
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

#endif