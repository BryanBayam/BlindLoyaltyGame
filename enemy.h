#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include "character.h"
#include <math.h>
#include <stdbool.h>

typedef struct Enemy {
    Vector2 pos;
    float speed;

    Texture2D tex;

    int frameCount;
    int currentFrame;
    int currentLine;
    float frameTimer;
    float frameSpeed;

    float width;
    float height;

    float aggroRange;
    float stopDistance;

    bool active;
    float touchCooldown;
} Enemy;

static inline void InitEnemy(Enemy *e, Vector2 startPos) {
    e->pos = startPos;
    e->speed = 80.0f;

    e->tex = LoadTexture("images/Character/Bandit/Bandit_walk.png");

    e->frameCount = 4;
    e->currentFrame = 0;
    e->currentLine = 3;
    e->frameTimer = 0.0f;
    e->frameSpeed = 0.14f;

    e->width = 48.0f;
    e->height = 48.0f;

    e->aggroRange = 220.0f;

    e->stopDistance = 2.0f;

    e->active = false;
    e->touchCooldown = 0.0f;
}

static inline Rectangle GetEnemyHitbox(Vector2 pos) {
    Rectangle hitbox = {
        pos.x - 7.0f,
        pos.y + 14.0f,
        14.0f,
        8.0f
    };
    return hitbox;
}

static inline bool CanEnemySpawnAt(Vector2 pos, const Tilemap *map) {
    Rectangle hitbox = GetEnemyHitbox(pos);
    return !CheckMapCollision(map, hitbox);
}

static inline void SpawnEnemy(Enemy *e, Vector2 pos, const Tilemap *map) {
    if (!CanEnemySpawnAt(pos, map)) return;

    e->pos = pos;
    e->currentFrame = 2;
    e->currentLine = 3;
    e->frameTimer = 0.0f;
    e->touchCooldown = 0.0f;
    e->active = true;
}

static inline void DespawnEnemy(Enemy *e) {
    e->active = false;
    e->touchCooldown = 0.0f;
}

static inline bool EnemyTouchesPlayer(const Enemy *e, const Player *player) {
    Rectangle enemyBox = GetEnemyHitbox(e->pos);
    Rectangle playerBox = GetPlayerHitbox(player->pos);
    return CheckCollisionRecs(enemyBox, playerBox);
}

static inline void UpdateEnemy(Enemy *e, Player *player, const Tilemap *map) {
    if (!e->active) return;
    if (player->isDead) return;

    float dt = GetFrameTime();

    if (e->touchCooldown > 0.0f) {
        e->touchCooldown -= dt;
        if (e->touchCooldown < 0.0f) e->touchCooldown = 0.0f;
    }

    Vector2 toPlayer = Vector2Subtract(player->pos, e->pos);
    float distance = Vector2Length(toPlayer);

    bool moved = false;

    if (distance <= e->aggroRange && distance > e->stopDistance) {
        Vector2 direction = { 0.0f, 0.0f };

        if (distance > 0.001f) {
            direction = Vector2Scale(toPlayer, 1.0f / distance);
        }

        if (fabsf(direction.x) > fabsf(direction.y)) {
            e->currentLine = (direction.x > 0.0f) ? 1 : 2;
        } else {
            e->currentLine = (direction.y < 0.0f) ? 0 : 3;
        }

        Vector2 nextPos = e->pos;
        nextPos.x += direction.x * e->speed * dt;

        Rectangle hitboxX = GetEnemyHitbox((Vector2){ nextPos.x, e->pos.y });
        if (!CheckMapCollision(map, hitboxX)) {
            if (fabsf(nextPos.x - e->pos.x) > 0.001f) moved = true;
            e->pos.x = nextPos.x;
        }

        nextPos = e->pos;
        nextPos.y += direction.y * e->speed * dt;

        Rectangle hitboxY = GetEnemyHitbox((Vector2){ e->pos.x, nextPos.y });
        if (!CheckMapCollision(map, hitboxY)) {
            if (fabsf(nextPos.y - e->pos.y) > 0.001f) moved = true;
            e->pos.y = nextPos.y;
        }
    }

    if (EnemyTouchesPlayer(e, player) && e->touchCooldown <= 0.0f) {
        player->health -= 10.0f;

        if (player->health <= 0.0f) {
            player->health = 0.0f;
            player->isDead = true;
        }

        e->touchCooldown = 1.0f;
    }

    if (moved) {
        e->frameTimer += dt;
        if (e->frameTimer >= e->frameSpeed) {
            e->frameTimer = 0.0f;
            e->currentFrame++;
            if (e->currentFrame >= e->frameCount) e->currentFrame = 0;
        }
    } else {
        e->currentFrame = 2;
        e->frameTimer = 0.0f;
    }
}

static inline void DrawEnemy(Enemy *e) {
    if (!e->active) return;
    if (e->tex.id <= 0) return;

    float frameW = (float)e->tex.width / 4.0f;
    float frameH = (float)e->tex.height / 4.0f;

    Rectangle sourceRec = {
        (float)e->currentFrame * frameW,
        (float)e->currentLine * frameH,
        frameW,
        frameH
    };

    Rectangle destRec = { e->pos.x, e->pos.y, e->width, e->height };
    Vector2 origin = { e->width / 2.0f, e->height / 2.0f };

    DrawTexturePro(e->tex, sourceRec, destRec, origin, 0.0f, WHITE);
}

static inline void UnloadEnemy(Enemy *e) {
    if (e->tex.id > 0) {
        UnloadTexture(e->tex);
        e->tex.id = 0;
    }
}

#endif