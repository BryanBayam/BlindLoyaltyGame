#ifndef ENEMY_MANAGER_H
#define ENEMY_MANAGER_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include "enemy.h"
#include "bandit.h"
#include "boss_bandit.h"

/*
 * Enemy spawn configuration.
 *
 * Regular enemies:
 * - Maximum count allowed in the world
 * - Initial count spawned after the opening delay
 * - Respawn delay after initial enemies are active
 *
 * Boss:
 * - Spawned once after its own delay
 *
 * Distance values are used to prevent enemies from spawning
 * too close to the player.
 */
#define MAX_REGULAR_ENEMIES 5
#define INITIAL_REGULAR_ENEMIES 2
#define REGULAR_RESPAWN_DELAY 22.0f
#define INITIAL_REGULAR_SPAWN_DELAY 5.0f
#define BOSS_SPAWN_DELAY 7.0f
#define REGULAR_MIN_SPAWN_DISTANCE 120.0f
#define BOSS_MIN_SPAWN_DISTANCE 160.0f

/*
 * Controls enemy spawn timing and one-time spawn states.
 *
 * Fields:
 * - regularRespawnTimer: Countdown until the next regular enemy respawn check
 * - initialRegularSpawnTimer: Countdown until the first regular enemies appear
 * - bossSpawnTimer: Countdown until the boss appears
 * - initialRegularsSpawned: Tracks whether the first regular wave has already spawned
 * - bossSpawned: Tracks whether the boss has already spawned
 */
typedef struct EnemySpawner {
    float regularRespawnTimer;
    float initialRegularSpawnTimer;
    float bossSpawnTimer;
    bool initialRegularsSpawned;
    bool bossSpawned;
} EnemySpawner;

/*
 * Initialize the spawner with default timers and reset all spawn flags.
 */
void InitEnemySpawner(EnemySpawner *spawner);

/*
 * Update all enemy spawn logic.
 *
 * Responsibilities:
 * - Spawn the initial regular enemies after a short delay
 * - Spawn the boss once after its delay
 * - Respawn regular enemies over time if slots are available
 *
 * Parameters:
 * - spawner: Spawn controller state
 * - dt: Delta time for this frame
 * - regulars: Array of regular enemy slots
 * - regularCount: Total number of regular enemy slots
 * - boss: Pointer to the boss enemy
 * - map: Tilemap used for validating spawn positions
 * - playerPos: Current player position used for spawn distance checks
 */
void UpdateEnemySpawns(EnemySpawner *spawner, float dt, Enemy regulars[], int regularCount, Enemy *boss, const Tilemap *map, Vector2 playerPos);

#endif

#ifdef ENEMY_MANAGER_IMPLEMENTATION
#ifndef ENEMY_MANAGER_IMPLEMENTATION_ONCE
#define ENEMY_MANAGER_IMPLEMENTATION_ONCE

/*
 * Find a valid enemy spawn position that:
 * - is on a walkable tile
 * - is at least minDistance away from the player
 * - does not overlap any active regular enemy
 * - does not overlap the boss if the boss is active
 *
 * If no valid position is found after many attempts,
 * the function falls back to a general walkable spawn position.
 */
static Vector2 FindEnemySpawnWithDistance(const Tilemap *map, Vector2 playerPos, float minDistance, Enemy regulars[], int regularCount, Enemy *boss) {
    float minDistSq = minDistance * minDistance;

    for (int attempt = 0; attempt < 3000; attempt++) {
        int col = GetRandomValue(1, map->width - 2);
        int row = GetRandomValue(1, map->height - 2);

        Vector2 candidate = {
            col * map->tileWidth + map->tileWidth * 0.5f,
            row * map->tileHeight + map->tileHeight * 0.5f
        };

        /* Reject invalid tiles or positions that are too close to the player. */
        if (!CanEnemyOccupy(candidate, map) || Vector2DistanceSqr(candidate, playerPos) < minDistSq) {
            continue;
        }

        bool overlapsEnemy = false;
        Rectangle candBox = GetEnemyHitbox(candidate);

        /* Reject positions that overlap any active regular enemy. */
        for (int i = 0; i < regularCount; i++) {
            if (regulars[i].active && CheckCollisionRecs(candBox, GetEnemyHitbox(regulars[i].pos))) {
                overlapsEnemy = true;
                break;
            }
        }

        /* Reject positions that overlap the boss if it is active. */
        if (!overlapsEnemy && boss != NULL && boss->active &&
            CheckCollisionRecs(candBox, GetEnemyHitbox(boss->pos))) {
            overlapsEnemy = true;
        }

        if (!overlapsEnemy) {
            return candidate;
        }
    }

    /* Fallback if no ideal position was found. */
    return FindWalkableSpawn(map);
}

/*
 * Reset all spawn timers and flags to their starting values.
 */
void InitEnemySpawner(EnemySpawner *spawner) {
    spawner->regularRespawnTimer = REGULAR_RESPAWN_DELAY;
    spawner->initialRegularSpawnTimer = INITIAL_REGULAR_SPAWN_DELAY;
    spawner->bossSpawnTimer = BOSS_SPAWN_DELAY;
    spawner->initialRegularsSpawned = false;
    spawner->bossSpawned = false;
}

/*
 * Main spawn update routine.
 *
 * Spawn flow:
 * 1. Count down initial regular spawn timer
 * 2. Count down boss spawn timer
 * 3. Spawn the first wave of regular enemies once
 * 4. Spawn the boss once
 * 5. Continue respawning regular enemies over time when slots are empty
 */
void UpdateEnemySpawns(EnemySpawner *spawner, float dt, Enemy regulars[], int regularCount, Enemy *boss, const Tilemap *map, Vector2 playerPos) {
    spawner->initialRegularSpawnTimer -= dt;
    spawner->bossSpawnTimer -= dt;

    /*
     * Spawn the initial set of regular enemies once the opening timer expires.
     * Only up to INITIAL_REGULAR_ENEMIES are created.
     */
    if (!spawner->initialRegularsSpawned && spawner->initialRegularSpawnTimer <= 0.0f) {
        int spawned = 0;

        for (int i = 0; i < regularCount && spawned < INITIAL_REGULAR_ENEMIES; i++) {
            Vector2 pos = FindEnemySpawnWithDistance(
                map,
                playerPos,
                REGULAR_MIN_SPAWN_DISTANCE,
                regulars,
                regularCount,
                boss
            );

            InitBandit(&regulars[i], pos);

            if (regulars[i].active) {
                spawned++;
            }
        }

        spawner->initialRegularsSpawned = true;
    }

    /*
     * Spawn the boss once when its timer expires.
     */
    if (!spawner->bossSpawned && spawner->bossSpawnTimer <= 0.0f) {
        Vector2 pos = FindEnemySpawnWithDistance(
            map,
            playerPos,
            BOSS_MIN_SPAWN_DISTANCE,
            regulars,
            regularCount,
            boss
        );

        InitBossBandit(boss, pos);
        spawner->bossSpawned = true;
    }

    /*
     * After the initial wave is active, regularly attempt to respawn
     * one missing regular enemy whenever the respawn timer expires.
     */
    if (spawner->initialRegularsSpawned) {
        spawner->regularRespawnTimer -= dt;

        if (spawner->regularRespawnTimer <= 0.0f) {
            int active = 0;
            int freeSlot = -1;

            /* Count active regular enemies and remember one free slot if available. */
            for (int i = 0; i < regularCount; i++) {
                if (regulars[i].active) {
                    active++;
                } else {
                    freeSlot = i;
                }
            }

            /*
             * Spawn one regular enemy only if:
             * - the current active count is below capacity
             * - an unused slot exists
             */
            if (active < regularCount && freeSlot != -1) {
                Vector2 pos = FindEnemySpawnWithDistance(
                    map,
                    playerPos,
                    REGULAR_MIN_SPAWN_DISTANCE,
                    regulars,
                    regularCount,
                    boss
                );

                if (CanEnemyOccupy(pos, map)) {
                    InitBandit(&regulars[freeSlot], pos);
                }
            }

            /* Reset the respawn timer after each respawn check. */
            spawner->regularRespawnTimer = REGULAR_RESPAWN_DELAY;
        }
    }
}

#endif
#endif