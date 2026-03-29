#ifndef OBJECTIVE_H
#define OBJECTIVE_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include "character.h"

/*
 * Key item configuration.
 *
 * KEY_DRAW_SIZE   : Size used when rendering the key sprite
 * KEY_HITBOX_SIZE : Size of the collision box used for pickup detection
 * KEY_SPAWN_DELAY : Time before the key appears in the level
 */
#define KEY_DRAW_SIZE 16.0f
#define KEY_HITBOX_SIZE 20.0f
#define KEY_SPAWN_DELAY 10.0f

/*
 * Represents the main objective item that the player must collect.
 *
 * Fields:
 * - texture: Sprite used to draw the key
 * - pos: World position of the key
 * - hitbox: Collision box used for pickup checks
 * - spawned: True once the key has appeared in the map
 * - collected: True once the player has picked up the key
 * - spawnTimer: Countdown until the key is spawned
 */
typedef struct KeyItem {
    Texture2D texture;
    Vector2 pos;
    Rectangle hitbox;
    bool spawned;
    bool collected;
    float spawnTimer;
} KeyItem;

/*
 * Reset the objective key to its initial state.
 * This should be called when starting or restarting gameplay.
 */
void ResetKey(KeyItem *key);

/*
 * Update key spawn and collection logic.
 *
 * Responsibilities:
 * - count down the spawn timer
 * - spawn the key after the delay expires
 * - detect player pickup
 * - trigger the win condition when collected
 */
void UpdateKeyLogic(KeyItem *key, float dt, const Tilemap *map, Vector2 playerPos, bool *gameWon);

/*
 * Draw the key if it has spawned and has not yet been collected.
 */
void DrawKey(const KeyItem *key);

#endif

#ifdef OBJECTIVE_IMPLEMENTATION

/*
 * Build the collision hitbox for the key based on its world position.
 * The hitbox is centered on the key.
 */
static Rectangle GetKeyHitbox(Vector2 pos) {
    return (Rectangle){
        pos.x - KEY_HITBOX_SIZE * 0.5f,
        pos.y - KEY_HITBOX_SIZE * 0.5f,
        KEY_HITBOX_SIZE,
        KEY_HITBOX_SIZE
    };
}

/*
 * Find the walkable position that is farthest from the player.
 *
 * This is used to place the key in a location that encourages exploration.
 * Only positions that:
 * - stay inside map bounds
 * - do not collide with blocked tiles
 * are considered valid.
 */
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

/*
 * Reset the key to its default inactive state.
 *
 * Result:
 * - position is cleared
 * - hitbox is rebuilt from the default position
 * - spawned and collected flags are cleared
 * - spawn timer is restored
 */
void ResetKey(KeyItem *key) {
    key->pos = (Vector2){ 0.0f, 0.0f };
    key->hitbox = GetKeyHitbox(key->pos);
    key->spawned = false;
    key->collected = false;
    key->spawnTimer = KEY_SPAWN_DELAY;
}

/*
 * Update the objective key each frame.
 *
 * Behavior:
 * 1. Count down the spawn timer
 * 2. Spawn the key once the timer reaches zero
 * 3. Check whether the player collides with the key
 * 4. Mark the game as won when the key is collected
 */
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

/*
 * Draw the key only when it is active and still available to collect.
 */
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

#endif