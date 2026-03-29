#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include "character.h"
#include <math.h>
#include <stdlib.h>

/*
 * High-level behavior states used by enemy AI.
 *
 * ENEMY_PATROL   : Move around nearby walkable tiles.
 * ENEMY_CHASE    : Move toward the player until in attack position.
 * ENEMY_RETREAT  : Move away briefly after attacking.
 */
typedef enum EnemyState {
    ENEMY_PATROL,
    ENEMY_CHASE,
    ENEMY_RETREAT
} EnemyState;

/*
 * Enemy category used to distinguish regular enemies from bosses.
 */
typedef enum EnemyType {
    ENEMY_BANDIT_REGULAR,
    ENEMY_BANDIT_BOSS
} EnemyType;

/*
 * Shared enemy data used for movement, animation, combat,
 * AI state, and rendering.
 */
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

    bool active;
    bool defeated;
    bool respawnAllowed;

    float touchCooldown;
    float damage;
    float aggroRange;
    float attackSlowDuration;

    float patrolTimer;
    Vector2 patrolTarget;

    EnemyState state;
    float stateTimer;
    EnemyType type;
} Enemy;

/*
 * Returns the enemy collision hitbox for a given world position.
 */
Rectangle GetEnemyHitbox(Vector2 pos);

/*
 * Returns the center point of a rectangle.
 * Useful for hitbox-based movement and pathfinding.
 */
Vector2 GetHitboxCenter(Rectangle r);

/*
 * Checks whether an enemy can occupy a position on the map.
 * This validates map bounds and wall collision only.
 */
bool CanEnemyOccupy(Vector2 pos, const Tilemap *map);

/*
 * Moves an enemy toward a target point while respecting:
 * - map collision
 * - player collision
 * - collision with other enemies
 *
 * Returns true if movement was attempted successfully.
 */
bool MoveEnemyTowardPoint(Enemy *e, Vector2 targetPoint, const Tilemap *map, const Player *player, Enemy allEnemies[], int totalEnemyCount, float dt);

/*
 * Chooses a nearby patrol point for idle enemy movement.
 */
Vector2 FindPatrolPoint(const Enemy *e, const Tilemap *map);

/*
 * Finds the next tile that moves the enemy toward a valid attack position
 * next to the player.
 *
 * Returns true if a valid next step is found.
 */
bool GetNextPathTileTowardAttackRange(const Enemy *e, const Player *player, const Tilemap *map, Vector2 *outNextPoint);

/*
 * Draw the enemy using its current animation frame.
 */
void DrawEnemy(const Enemy *e);

/*
 * Release texture resources owned by this enemy.
 */
void UnloadEnemy(Enemy *e);

/*
 * Shared enemy attack sound effect management.
 * These functions ensure the sound is loaded once and reused.
 */
void EnsureEnemyAttackSfxLoaded(void);
void PlayEnemyAttackSfx(void);
void UnloadEnemyAttackSfx(void);

#endif

#ifdef ENEMY_IMPLEMENTATION
#ifndef ENEMY_IMPLEMENTATION_ONCE
#define ENEMY_IMPLEMENTATION_ONCE

/*
 * Shared attack sound state.
 * Loaded once and reused by all enemy instances.
 */
static Sound gEnemyAttackSfx = { 0 };
static bool gEnemyAttackSfxLoaded = false;

/*
 * Load the shared enemy attack sound only once.
 */
void EnsureEnemyAttackSfxLoaded(void) {
    if (!gEnemyAttackSfxLoaded) {
        gEnemyAttackSfx = LoadSound("audio/Sfx/enemy_attack.mp3");
        gEnemyAttackSfxLoaded = true;
    }
}

/*
 * Play the shared enemy attack sound.
 * Loads the sound first if needed.
 */
void PlayEnemyAttackSfx(void) {
    EnsureEnemyAttackSfxLoaded();
    PlaySound(gEnemyAttackSfx);
}

/*
 * Unload the shared enemy attack sound safely.
 */
void UnloadEnemyAttackSfx(void) {
    if (gEnemyAttackSfxLoaded) {
        UnloadSound(gEnemyAttackSfx);
        gEnemyAttackSfx = (Sound){ 0 };
        gEnemyAttackSfxLoaded = false;
    }
}

/*
 * Enemy collision hitbox.
 * The hitbox is intentionally smaller than the full sprite
 * so collisions feel more natural around the character feet/body.
 */
Rectangle GetEnemyHitbox(Vector2 pos) {
    return (Rectangle){ pos.x - 7.0f, pos.y + 14.0f, 14.0f, 8.0f };
}

/*
 * Compute the center of a rectangle.
 */
Vector2 GetHitboxCenter(Rectangle r) {
    return (Vector2){ r.x + r.width * 0.5f, r.y + r.height * 0.5f };
}

/*
 * Check whether an enemy can stand at the given position.
 *
 * Conditions:
 * - hitbox must stay inside map bounds
 * - hitbox must not collide with blocked tiles
 */
bool CanEnemyOccupy(Vector2 pos, const Tilemap *map) {
    Rectangle box = GetEnemyHitbox(pos);
    float mapW = (float)map->width * map->tileWidth;
    float mapH = (float)map->height * map->tileHeight;

    if (box.x < 0.0f || box.y < 0.0f || box.x + box.width > mapW || box.y + box.height > mapH) {
        return false;
    }

    return !CheckMapCollision(map, box);
}

/*
 * Check whether an enemy position would overlap the player.
 */
static bool WouldOverlapPlayer(Vector2 enemyPos, const Player *player) {
    return CheckCollisionRecs(GetEnemyHitbox(enemyPos), GetPlayerHitbox(player->pos));
}

/*
 * Check whether an enemy position would overlap another active enemy.
 * The current enemy itself is ignored.
 */
static bool WouldOverlapOtherEnemy(Vector2 enemyPos, const Enemy *self, Enemy allEnemies[], int totalEnemyCount) {
    Rectangle selfBox = GetEnemyHitbox(enemyPos);

    for (int i = 0; i < totalEnemyCount; i++) {
        if (!allEnemies[i].active || &allEnemies[i] == self) continue;

        if (CheckCollisionRecs(selfBox, GetEnemyHitbox(allEnemies[i].pos))) {
            return true;
        }
    }

    return false;
}

/*
 * Full movement validation for enemies.
 *
 * This extends CanEnemyOccupy() by also preventing overlap
 * with the player and other enemies.
 */
bool CanEnemyMoveTo(Vector2 pos, const Tilemap *map, const Player *player, const Enemy *self, Enemy allEnemies[], int totalEnemyCount) {
    return CanEnemyOccupy(pos, map) &&
           !WouldOverlapPlayer(pos, player) &&
           !WouldOverlapOtherEnemy(pos, self, allEnemies, totalEnemyCount);
}

/*
 * Check whether a tile coordinate is inside the map
 * and marked as walkable.
 */
static bool IsWalkableTile(const Tilemap *map, int tx, int ty) {
    if (tx < 0 || ty < 0 || tx >= map->width || ty >= map->height) {
        return false;
    }

    return (map->wall[ty][tx] == 0);
}

/*
 * Convert a world-space point into tile coordinates.
 * The result is clamped so it always stays inside the map.
 */
static void WorldToTileFromPoint(const Tilemap *map, Vector2 point, int *tx, int *ty) {
    *tx = (int)(point.x / map->tileWidth);
    *ty = (int)(point.y / map->tileHeight);

    if (*tx < 0) *tx = 0;
    if (*ty < 0) *ty = 0;
    if (*tx >= map->width) *tx = map->width - 1;
    if (*ty >= map->height) *ty = map->height - 1;
}

/*
 * Convert a tile coordinate into the world-space center of that tile.
 */
static Vector2 TileToWorldCenter(const Tilemap *map, int tx, int ty) {
    return (Vector2){
        tx * map->tileWidth + map->tileWidth * 0.5f,
        ty * map->tileHeight + map->tileHeight * 0.5f
    };
}

/*
 * A valid attack goal tile is any walkable tile directly adjacent
 * to the player's tile in the four cardinal directions.
 */
static bool IsAttackGoalTile(const Tilemap *map, int tx, int ty, int playerTx, int playerTy) {
    if (!IsWalkableTile(map, tx, ty)) return false;
    return ((abs(tx - playerTx) + abs(ty - playerTy)) == 1);
}

/*
 * Find the next step toward a tile adjacent to the player.
 *
 * This function performs a breadth-first search on the tilemap.
 * If the enemy is already standing in a valid attack tile,
 * that current tile is returned.
 */
bool GetNextPathTileTowardAttackRange(const Enemy *e, const Player *player, const Tilemap *map, Vector2 *outNextPoint) {
    int startX, startY, playerX, playerY;

    WorldToTileFromPoint(map, GetHitboxCenter(GetEnemyHitbox(e->pos)), &startX, &startY);
    WorldToTileFromPoint(map, GetHitboxCenter(GetPlayerHitbox(player->pos)), &playerX, &playerY);

    if (!IsWalkableTile(map, startX, startY)) return false;

    if (IsAttackGoalTile(map, startX, startY, playerX, playerY)) {
        *outNextPoint = TileToWorldCenter(map, startX, startY);
        return true;
    }

    static bool visited[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
    static int parentX[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
    static int parentY[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];

    for (int y = 0; y < map->height; y++) {
        for (int x = 0; x < map->width; x++) {
            visited[y][x] = false;
            parentX[y][x] = -1;
            parentY[y][x] = -1;
        }
    }

    static int qx[MAP_MAX_WIDTH * MAP_MAX_HEIGHT];
    static int qy[MAP_MAX_WIDTH * MAP_MAX_HEIGHT];

    int qHead = 0;
    int qTail = 0;

    qx[qTail] = startX;
    qy[qTail] = startY;
    qTail++;

    visited[startY][startX] = true;

    int foundX = -1;
    int foundY = -1;

    while (qHead < qTail) {
        int cx = qx[qHead];
        int cy = qy[qHead];
        qHead++;

        if (IsAttackGoalTile(map, cx, cy, playerX, playerY)) {
            foundX = cx;
            foundY = cy;
            break;
        }

        /*
         * Neighbor expansion order alternates by tile parity.
         * This helps reduce directional bias when multiple paths are equal.
         */
        int dx4[4], dy4[4];
        if ((cx + cy) % 2 == 0) {
            dx4[0] = 1;  dx4[1] = -1; dx4[2] = 0;  dx4[3] = 0;
            dy4[0] = 0;  dy4[1] = 0;  dy4[2] = 1;  dy4[3] = -1;
        } else {
            dx4[0] = 0;  dx4[1] = 0;  dx4[2] = 1;  dx4[3] = -1;
            dy4[0] = 1;  dy4[1] = -1; dy4[2] = 0;  dy4[3] = 0;
        }

        for (int i = 0; i < 4; i++) {
            int nx = cx + dx4[i];
            int ny = cy + dy4[i];

            if (!IsWalkableTile(map, nx, ny) || visited[ny][nx]) continue;

            visited[ny][nx] = true;
            parentX[ny][nx] = cx;
            parentY[ny][nx] = cy;

            qx[qTail] = nx;
            qy[qTail] = ny;
            qTail++;
        }
    }

    if (foundX < 0 || foundY < 0) return false;

    /*
     * Walk backward through the parent map to find
     * the first step from the start tile toward the goal.
     */
    int pathX = foundX;
    int pathY = foundY;

    while (!(parentX[pathY][pathX] == startX && parentY[pathY][pathX] == startY)) {
        int px = parentX[pathY][pathX];
        int py = parentY[pathY][pathX];

        if (px < 0 || py < 0) break;

        pathX = px;
        pathY = py;
    }

    *outNextPoint = TileToWorldCenter(map, pathX, pathY);
    return true;
}

/*
 * Pick a nearby walkable tile for patrol movement.
 * The enemy checks the four cardinal directions in randomized order.
 * If none are valid, it stays in its current position.
 */
Vector2 FindPatrolPoint(const Enemy *e, const Tilemap *map) {
    int ex, ey;
    WorldToTileFromPoint(map, GetHitboxCenter(GetEnemyHitbox(e->pos)), &ex, &ey);

    const int dirs[4][2] = {
        { 1, 0 },
        { -1, 0 },
        { 0, 1 },
        { 0, -1 }
    };

    int startDir = GetRandomValue(0, 3);

    for (int i = 0; i < 4; i++) {
        int dirIdx = (startDir + i) % 4;
        int nx = ex + dirs[dirIdx][0];
        int ny = ey + dirs[dirIdx][1];

        if (IsWalkableTile(map, nx, ny)) {
            return TileToWorldCenter(map, nx, ny);
        }
    }

    return e->pos;
}

/*
 * Update the sprite row based on movement direction.
 */
static void SetEnemyFacingFromDirection(Enemy *e, Vector2 direction) {
    if (fabsf(direction.x) > fabsf(direction.y)) {
        e->currentLine = (direction.x > 0.0f) ? 1 : 2;
    } else {
        e->currentLine = (direction.y < 0.0f) ? 0 : 3;
    }
}

/*
 * Move the enemy toward a target point.
 *
 * Movement behavior:
 * - uses hitbox center to aim
 * - slightly softens diagonal bias
 * - updates facing direction
 * - resolves X and Y movement separately
 *
 * Returns true if a movement step exists.
 */
bool MoveEnemyTowardPoint(Enemy *e, Vector2 targetPoint, const Tilemap *map, const Player *player, Enemy allEnemies[], int totalEnemyCount, float dt) {
    Vector2 hitboxCenter = GetHitboxCenter(GetEnemyHitbox(e->pos));
    Vector2 toTarget = Vector2Subtract(targetPoint, hitboxCenter);

    if (fabsf(toTarget.x) > fabsf(toTarget.y)) {
        toTarget.y *= 0.6f;
    } else {
        toTarget.x *= 0.6f;
    }

    float dist = Vector2Length(toTarget);
    if (dist <= 0.001f) return false;

    Vector2 dir = Vector2Scale(toTarget, 1.0f / dist);
    SetEnemyFacingFromDirection(e, dir);

    float maxStep = (e->speed * dt > dist) ? dist : e->speed * dt;
    Vector2 moveStep = Vector2Scale(dir, maxStep);

    Vector2 desiredXPos = { e->pos.x + moveStep.x, e->pos.y };
    if (CanEnemyMoveTo(desiredXPos, map, player, e, allEnemies, totalEnemyCount)) {
        e->pos.x = desiredXPos.x;
    }

    Vector2 desiredYPos = { e->pos.x, e->pos.y + moveStep.y };
    if (CanEnemyMoveTo(desiredYPos, map, player, e, allEnemies, totalEnemyCount)) {
        e->pos.y = desiredYPos.y;
    }

    return (fabsf(moveStep.x) > 0.001f || fabsf(moveStep.y) > 0.001f);
}

/*
 * Draw the enemy using the current frame and facing row.
 */
void DrawEnemy(const Enemy *e) {
    if (!e->active || e->tex.id <= 0) return;

    float frameW = (float)e->tex.width / (float)e->frameCount;
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

/*
 * Unload the enemy texture safely.
 */
void UnloadEnemy(Enemy *e) {
    if (e->tex.id > 0) {
        UnloadTexture(e->tex);
    }

    e->tex.id = 0;
}

#endif
#endif