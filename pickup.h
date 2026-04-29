#ifndef PICKUP_H
#define PICKUP_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include "character.h"

// Configuration constants
#define HEART_COUNT 2              // Number of hearts to spawn
#define SPEED_COUNT 3              // Number of speed boosts to spawn
#define HEART_DRAW_WIDTH   16.0f   // Visual size of the heart
#define HEART_DRAW_HEIGHT  16.0f
#define SPEED_DRAW_WIDTH   32.0f   // Visual size of the speed icon
#define SPEED_DRAW_HEIGHT  16.0f
#define PICKUP_MIN_SEPARATION 80.0f // Keep items this far apart from each other

// Types of items available
typedef enum PickupType {
    PICKUP_HEART,
    PICKUP_SPEED
} PickupType;

// The structure for a single item on the ground
typedef struct Pickup {
    Texture2D *texture; // Pointer to the image loaded in memory
    Vector2 pos;        // X and Y coordinates
    Rectangle hitbox;   // Area used for collision detection
    bool active;        // False if the item has already been picked up
    PickupType type;    // Heart or Speed?
} Pickup;

// Functions to manage items
void SpawnPickups(const Tilemap *map, Pickup hearts[], int heartCount, Texture2D *heartTexture, Pickup speeds[], int speedCount, Texture2D *speedTexture);
void DrawPickup(const Pickup *pickup);
void CheckPickupCollisions(Player *player, Pickup hearts[], Pickup speeds[], bool *pickedHeart, bool *pickedSpeed);

#ifdef PICKUP_IMPLEMENTATION

// Helper: Gets the width based on the item type
static float GetPickupDrawWidth(PickupType type) {
    return (type == PICKUP_SPEED) ? SPEED_DRAW_WIDTH : HEART_DRAW_WIDTH;
}

// Helper: Gets the height based on the item type
static float GetPickupDrawHeight(PickupType type) {
    return (type == PICKUP_SPEED) ? SPEED_DRAW_HEIGHT : HEART_DRAW_HEIGHT;
}

// Creates the mathematical rectangle for collisions based on the item's center
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

// Checks if a spot on the map is valid (not inside a wall or off-screen)
static bool CanPlacePickupAt(Vector2 pos, PickupType type, const Tilemap *map) {
    Rectangle rect = GetPickupHitboxByType(pos, type);
    float mapW = (float)map->width * map->tileWidth;
    float mapH = (float)map->height * map->tileHeight;

    // Is it outside map boundaries?
    if (rect.x < 0.0f || rect.y < 0.0f || rect.x + rect.width > mapW || rect.y + rect.height > mapH) {
        return false;
    }

    // Is it hitting a solid wall in the tilemap?
    return !CheckMapCollision(map, rect);
}

// Tries to find a random spot on the map where a character can walk
static Vector2 FindRandomWalkablePoint(const Tilemap *map, PickupType type) {
    for (int attempt = 0; attempt < 1000; attempt++) {
        // Pick a random tile coordinate and convert to pixels
        Vector2 candidate = {
            GetRandomValue(0, map->width - 1) * map->tileWidth + map->tileWidth * 0.5f,
            GetRandomValue(0, map->height - 1) * map->tileHeight + map->tileHeight * 0.5f
        };

        // If the spot is valid, return it
        if (CanPlacePickupAt(candidate, type, map)) {
            return candidate;
        }
    }

    // Fallback: use a known safe spawn point if random picking fails 1000 times
    return FindWalkableSpawn(map);
}

// Logic to place all hearts and speed boosts on the map
void SpawnPickups(const Tilemap *map, Pickup hearts[], int heartCount, Texture2D *heartTexture, Pickup speeds[], int speedCount, Texture2D *speedTexture) {
    Pickup all[HEART_COUNT + SPEED_COUNT] = { 0 };
    int totalPlaced = 0;

    // Initialize arrays
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

    // Place Hearts
    for (int i = 0; i < heartCount; i++) {
        for (int attempt = 0; attempt < 1000; attempt++) {
            Vector2 pos = FindRandomWalkablePoint(map, PICKUP_HEART);
            bool ok = true;

            // Ensure this item isn't too close to one we already placed
            for (int j = 0; j < totalPlaced; j++) {
                if (Vector2DistanceSqr(pos, all[j].pos) < minSepSq) {
                    ok = false;
                    break;
                }
            }

            if (!ok) continue;

            hearts[i].pos = pos;
            hearts[i].hitbox = GetPickupHitboxByType(pos, hearts[i].type);
            hearts[i].active = true;
            all[totalPlaced++] = hearts[i];
            break;
        }
    }

    // Place Speed Boosts (same logic as hearts)
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

            if (!ok) continue;

            speeds[i].pos = pos;
            speeds[i].hitbox = GetPickupHitboxByType(pos, speeds[i].type);
            speeds[i].active = true;
            all[totalPlaced++] = speeds[i];
            break;
        }
    }
}

// Logic to check if the player walked over an item
void CheckPickupCollisions(Player *player, Pickup hearts[], Pickup speeds[], bool *pickedHeart, bool *pickedSpeed) {
    Rectangle pBox = GetPlayerHitbox(player->pos); // Get player's area

    if (pickedHeart) *pickedHeart = false;
    if (pickedSpeed) *pickedSpeed = false;

    // Check hearts
    for (int i = 0; i < HEART_COUNT; i++) {
        if (hearts[i].active && CheckCollisionRecs(pBox, hearts[i].hitbox)) {
            HealPlayer(player, 25.0f); // Give health
            hearts[i].active = false;  // Deactivate so it disappears
            if (pickedHeart) *pickedHeart = true;
        }
    }

    // Check speed boosts
    for (int i = 0; i < SPEED_COUNT; i++) {
        if (speeds[i].active && CheckCollisionRecs(pBox, speeds[i].hitbox)) {
            player->speedMultiplier = 1.25f; // Make player 25% faster
            AddPlayerEnergy(player, 20.0f);  // Give some energy
            speeds[i].active = false;
            if (pickedSpeed) *pickedSpeed = true;
        }
    }
}

// Draw the items on the screen
void DrawPickup(const Pickup *pickup) {
    // Don't draw if the item is picked up or missing a texture
    if (!pickup->active || pickup->texture == NULL || pickup->texture->id <= 0) {
        return;
    }

    float drawWidth = GetPickupDrawWidth(pickup->type);
    float drawHeight = GetPickupDrawHeight(pickup->type);

    // Part of the image to use (the whole thing)
    Rectangle src = { 0, 0, (float)pickup->texture->width, (float)pickup->texture->height };
    // Where to draw it on the screen
    Rectangle dst = { pickup->pos.x, pickup->pos.y, drawWidth, drawHeight };
    // Center the texture so pos.x/y is the middle of the icon
    Vector2 origin = { drawWidth / 2.0f, drawHeight / 2.0f };

    DrawTexturePro(*pickup->texture, src, dst, origin, 0.0f, WHITE);
}

#endif // PICKUP_IMPLEMENTATION
#endif // PICKUP_H