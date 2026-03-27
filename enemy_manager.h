#ifndef ENEMY_MANAGER_H
#define ENEMY_MANAGER_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include "enemy.h"

// TAMBAHKAN DUA HEADER INI AGAR FUNGSI INIT BANDIT DAN BOSS DIKENAL
#include "bandit.h" 
#include "boss_bandit.h"

#define MAX_REGULAR_ENEMIES 5
#define INITIAL_REGULAR_ENEMIES 2
#define REGULAR_RESPAWN_DELAY 22.0f
#define INITIAL_REGULAR_SPAWN_DELAY 5.0f
#define BOSS_SPAWN_DELAY 7.0f
#define REGULAR_MIN_SPAWN_DISTANCE 120.0f
#define BOSS_MIN_SPAWN_DISTANCE 160.0f

typedef struct EnemySpawner {
    float regularRespawnTimer, initialRegularSpawnTimer, bossSpawnTimer;
    bool initialRegularsSpawned, bossSpawned;
} EnemySpawner;

void InitEnemySpawner(EnemySpawner *spawner);
void UpdateEnemySpawns(EnemySpawner *spawner, float dt, Enemy regulars[], int regularCount, Enemy *boss, const Tilemap *map, Vector2 playerPos);

#endif // ENEMY_MANAGER_H


// ==========================================
// IMPLEMENTASI ENEMY MANAGER
// ==========================================
#ifdef ENEMY_MANAGER_IMPLEMENTATION
#ifndef ENEMY_MANAGER_IMPLEMENTATION_ONCE // <--- PELINDUNG GANDA
#define ENEMY_MANAGER_IMPLEMENTATION_ONCE

static Vector2 FindEnemySpawnWithDistance(const Tilemap *map, Vector2 playerPos, float minDistance, Enemy regulars[], int regularCount, Enemy *boss) {
    float minDistSq = minDistance * minDistance;
    for (int attempt = 0; attempt < 3000; attempt++) {
        int col = GetRandomValue(1, map->width - 2), row = GetRandomValue(1, map->height - 2);
        Vector2 candidate = { col * map->tileWidth + map->tileWidth * 0.5f, row * map->tileHeight + map->tileHeight * 0.5f };
        if (!CanEnemyOccupy(candidate, map) || Vector2DistanceSqr(candidate, playerPos) < minDistSq) continue;

        bool overlapsEnemy = false;
        Rectangle candBox = GetEnemyHitbox(candidate);
        for (int i = 0; i < regularCount; i++) if (regulars[i].active && CheckCollisionRecs(candBox, GetEnemyHitbox(regulars[i].pos))) { overlapsEnemy = true; break; }
        if (!overlapsEnemy && boss != NULL && boss->active && CheckCollisionRecs(candBox, GetEnemyHitbox(boss->pos))) overlapsEnemy = true;
        
        if (!overlapsEnemy) return candidate;
    }
    return FindWalkableSpawn(map); // Pastikan ini ada di tilemap.h
}

void InitEnemySpawner(EnemySpawner *spawner) {
    spawner->regularRespawnTimer = REGULAR_RESPAWN_DELAY; spawner->initialRegularSpawnTimer = INITIAL_REGULAR_SPAWN_DELAY;
    spawner->bossSpawnTimer = BOSS_SPAWN_DELAY; spawner->initialRegularsSpawned = false; spawner->bossSpawned = false;
}

void UpdateEnemySpawns(EnemySpawner *spawner, float dt, Enemy regulars[], int regularCount, Enemy *boss, const Tilemap *map, Vector2 playerPos) {
    spawner->initialRegularSpawnTimer -= dt; spawner->bossSpawnTimer -= dt;

    if (!spawner->initialRegularsSpawned && spawner->initialRegularSpawnTimer <= 0.0f) {
        int spawned = 0;
        for (int i = 0; i < regularCount && spawned < INITIAL_REGULAR_ENEMIES; i++) {
            Vector2 pos = FindEnemySpawnWithDistance(map, playerPos, REGULAR_MIN_SPAWN_DISTANCE, regulars, regularCount, boss);
            InitBandit(&regulars[i], pos); // MENGGUNAKAN FUNGSI YANG BARU
            if (regulars[i].active) spawned++;
        }
        spawner->initialRegularsSpawned = true;
    }

    if (!spawner->bossSpawned && spawner->bossSpawnTimer <= 0.0f) {
        Vector2 pos = FindEnemySpawnWithDistance(map, playerPos, BOSS_MIN_SPAWN_DISTANCE, regulars, regularCount, boss);
        InitBossBandit(boss, pos); // MENGGUNAKAN FUNGSI YANG BARU
        spawner->bossSpawned = true;
    }

    if (spawner->initialRegularsSpawned) {
        spawner->regularRespawnTimer -= dt;
        if (spawner->regularRespawnTimer <= 0.0f) {
            int active = 0, freeSlot = -1;
            for (int i = 0; i < regularCount; i++) { if (regulars[i].active) active++; else freeSlot = i; }
            if (active < regularCount && freeSlot != -1) {
                Vector2 pos = FindEnemySpawnWithDistance(map, playerPos, REGULAR_MIN_SPAWN_DISTANCE, regulars, regularCount, boss);
                if (CanEnemyOccupy(pos, map)) InitBandit(&regulars[freeSlot], pos); // MENGGUNAKAN FUNGSI YANG BARU
            }
            spawner->regularRespawnTimer = REGULAR_RESPAWN_DELAY;
        }
    }
}

#endif // ENEMY_MANAGER_IMPLEMENTATION_ONCE
#endif // ENEMY_MANAGER_IMPLEMENTATION