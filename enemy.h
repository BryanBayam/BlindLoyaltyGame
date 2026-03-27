#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include "character.h"
#include <math.h>
#include <stdlib.h>

typedef enum EnemyState { ENEMY_PATROL, ENEMY_CHASE, ENEMY_RETREAT } EnemyState;
typedef enum EnemyType { ENEMY_BANDIT_REGULAR, ENEMY_BANDIT_BOSS } EnemyType;

typedef struct Enemy {
    Vector2 pos;
    float speed;
    Texture2D tex;
    int frameCount, currentFrame, currentLine;
    float frameTimer, frameSpeed, width, height;
    bool active, defeated, respawnAllowed;
    float touchCooldown, damage, aggroRange, attackSlowDuration;
    float patrolTimer;
    Vector2 patrolTarget;
    EnemyState state;
    float stateTimer;
    EnemyType type;
} Enemy;

// Prototipe fungsi yang bisa dipakai oleh file lain
Rectangle GetEnemyHitbox(Vector2 pos);
Vector2 GetHitboxCenter(Rectangle r);
bool CanEnemyOccupy(Vector2 pos, const Tilemap *map);
bool MoveEnemyTowardPoint(Enemy *e, Vector2 targetPoint, const Tilemap *map, const Player *player, Enemy allEnemies[], int totalEnemyCount, float dt);
Vector2 FindPatrolPoint(const Enemy *e, const Tilemap *map);
bool GetNextPathTileTowardAttackRange(const Enemy *e, const Player *player, const Tilemap *map, Vector2 *outNextPoint);
void DrawEnemy(const Enemy *e);
void UnloadEnemy(Enemy *e);

#endif // ENEMY_H


// ==========================================
// IMPLEMENTASI PONDASI DASAR
// ==========================================
#ifdef ENEMY_IMPLEMENTATION
#ifndef ENEMY_IMPLEMENTATION_ONCE // <--- PELINDUNG GANDA UNTUK MENCEGAH REDEFINITION
#define ENEMY_IMPLEMENTATION_ONCE

Rectangle GetEnemyHitbox(Vector2 pos) { return (Rectangle){ pos.x - 7.0f, pos.y + 14.0f, 14.0f, 8.0f }; }
Vector2 GetHitboxCenter(Rectangle r) { return (Vector2){ r.x + r.width * 0.5f, r.y + r.height * 0.5f }; }

bool CanEnemyOccupy(Vector2 pos, const Tilemap *map) {
    Rectangle box = GetEnemyHitbox(pos);
    float mapW = (float)map->width * map->tileWidth, mapH = (float)map->height * map->tileHeight;
    if (box.x < 0.0f || box.y < 0.0f || box.x + box.width > mapW || box.y + box.height > mapH) return false;
    return !CheckMapCollision(map, box);
}

static bool WouldOverlapPlayer(Vector2 enemyPos, const Player *player) { return CheckCollisionRecs(GetEnemyHitbox(enemyPos), GetPlayerHitbox(player->pos)); }
static bool WouldOverlapOtherEnemy(Vector2 enemyPos, const Enemy *self, Enemy allEnemies[], int totalEnemyCount) {
    Rectangle selfBox = GetEnemyHitbox(enemyPos);
    for (int i = 0; i < totalEnemyCount; i++) {
        if (!allEnemies[i].active || &allEnemies[i] == self) continue;
        if (CheckCollisionRecs(selfBox, GetEnemyHitbox(allEnemies[i].pos))) return true;
    }
    return false;
}

bool CanEnemyMoveTo(Vector2 pos, const Tilemap *map, const Player *player, const Enemy *self, Enemy allEnemies[], int totalEnemyCount) {
    return CanEnemyOccupy(pos, map) && !WouldOverlapPlayer(pos, player) && !WouldOverlapOtherEnemy(pos, self, allEnemies, totalEnemyCount);
}

static bool IsWalkableTile(const Tilemap *map, int tx, int ty) {
    if (tx < 0 || ty < 0 || tx >= map->width || ty >= map->height) return false;
    return (map->wall[ty][tx] == 0);
}

static void WorldToTileFromPoint(const Tilemap *map, Vector2 point, int *tx, int *ty) {
    *tx = (int)(point.x / map->tileWidth); 
    *ty = (int)(point.y / map->tileHeight);
    
    if (*tx < 0) { *tx = 0; }
    if (*ty < 0) { *ty = 0; }
    if (*tx >= map->width) { *tx = map->width - 1; }
    if (*ty >= map->height) { *ty = map->height - 1; }
}

static Vector2 TileToWorldCenter(const Tilemap *map, int tx, int ty) { return (Vector2){ tx * map->tileWidth + map->tileWidth * 0.5f, ty * map->tileHeight + map->tileHeight * 0.5f }; }

static bool IsAttackGoalTile(const Tilemap *map, int tx, int ty, int playerTx, int playerTy) {
    if (!IsWalkableTile(map, tx, ty)) return false;
    return ((abs(tx - playerTx) + abs(ty - playerTy)) == 1);
}

bool GetNextPathTileTowardAttackRange(const Enemy *e, const Player *player, const Tilemap *map, Vector2 *outNextPoint) {
    int startX, startY, playerX, playerY;
    WorldToTileFromPoint(map, GetHitboxCenter(GetEnemyHitbox(e->pos)), &startX, &startY);
    WorldToTileFromPoint(map, GetHitboxCenter(GetPlayerHitbox(player->pos)), &playerX, &playerY);

    if (!IsWalkableTile(map, startX, startY)) return false;
    if (IsAttackGoalTile(map, startX, startY, playerX, playerY)) { *outNextPoint = TileToWorldCenter(map, startX, startY); return true; }

    static bool visited[MAP_MAX_HEIGHT][MAP_MAX_WIDTH]; static int parentX[MAP_MAX_HEIGHT][MAP_MAX_WIDTH], parentY[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
    for (int y = 0; y < map->height; y++) for (int x = 0; x < map->width; x++) { visited[y][x] = false; parentX[y][x] = -1; parentY[y][x] = -1; }

    static int qx[MAP_MAX_WIDTH * MAP_MAX_HEIGHT], qy[MAP_MAX_WIDTH * MAP_MAX_HEIGHT];
    int qHead = 0, qTail = 0; qx[qTail] = startX; qy[qTail] = startY; qTail++; visited[startY][startX] = true;
    int foundX = -1, foundY = -1;

while (qHead < qTail) {
        int cx = qx[qHead], cy = qy[qHead]; qHead++;
        if (IsAttackGoalTile(map, cx, cy, playerX, playerY)) { foundX = cx; foundY = cy; break; }
        
        // --- PERBAIKAN 1: Checkerboard Alternating Direction ---
        int dx4[4], dy4[4];
        if ((cx + cy) % 2 == 0) {
            dx4[0] = 1; dx4[1] = -1; dx4[2] = 0; dx4[3] = 0;
            dy4[0] = 0; dy4[1] = 0; dy4[2] = 1; dy4[3] = -1;
        } else {
            dx4[0] = 0; dx4[1] = 0; dx4[2] = 1; dx4[3] = -1;
            dy4[0] = 1; dy4[1] = -1; dy4[2] = 0; dy4[3] = 0;
        }
        // -------------------------------------------------------

        for (int i = 0; i < 4; i++) {
            int nx = cx + dx4[i], ny = cy + dy4[i];
            if (!IsWalkableTile(map, nx, ny) || visited[ny][nx]) continue;
            visited[ny][nx] = true; parentX[ny][nx] = cx; parentY[ny][nx] = cy; qx[qTail] = nx; qy[qTail] = ny; qTail++;
        }
    }
    if (foundX < 0 || foundY < 0) return false;
    int pathX = foundX, pathY = foundY;
    while (!(parentX[pathY][pathX] == startX && parentY[pathY][pathX] == startY)) {
        int px = parentX[pathY][pathX], py = parentY[pathY][pathX];
        if (px < 0 || py < 0) break;
        pathX = px; pathY = py;
    }
    *outNextPoint = TileToWorldCenter(map, pathX, pathY);
    return true;
}

Vector2 FindPatrolPoint(const Enemy *e, const Tilemap *map) {
    int ex, ey; WorldToTileFromPoint(map, GetHitboxCenter(GetEnemyHitbox(e->pos)), &ex, &ey);
    const int dirs[4][2] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
    
    // --- PERBAIKAN 3: Acak arah patroli awal ---
    int startDir = GetRandomValue(0, 3);
    for (int i = 0; i < 4; i++) {
        int dirIdx = (startDir + i) % 4; // Berputar mulai dari arah acak
        int nx = ex + dirs[dirIdx][0], ny = ey + dirs[dirIdx][1];
        if (IsWalkableTile(map, nx, ny)) return TileToWorldCenter(map, nx, ny);
    }
    // -------------------------------------------
    
    return e->pos;
}

static void SetEnemyFacingFromDirection(Enemy *e, Vector2 direction) {
    if (fabsf(direction.x) > fabsf(direction.y)) e->currentLine = (direction.x > 0.0f) ? 1 : 2;
    else e->currentLine = (direction.y < 0.0f) ? 0 : 3;
}

bool MoveEnemyTowardPoint(Enemy *e, Vector2 targetPoint, const Tilemap *map, const Player *player, Enemy allEnemies[], int totalEnemyCount, float dt) {
    Vector2 hitboxCenter = GetHitboxCenter(GetEnemyHitbox(e->pos));
    Vector2 toTarget = Vector2Subtract(targetPoint, hitboxCenter);
    if (fabsf(toTarget.x) > fabsf(toTarget.y)) toTarget.y *= 0.6f;
    else toTarget.x *= 0.6f;

   float dist = Vector2Length(toTarget);
    if (dist <= 0.001f) return false;
    
    Vector2 dir = Vector2Scale(toTarget, 1.0f / dist);
    
    // Putar badan menghadap arah jalan
    SetEnemyFacingFromDirection(e, dir);
    
    float maxStep = (e->speed * dt > dist) ? dist : e->speed * dt;
    Vector2 moveStep = Vector2Scale(dir, maxStep);

    // ==========================================
    // 3. IMPLEMENTASI SLIDING COLLISION (PENTING)
    // ==========================================
    
    // Coba bergerak di sumbu X dulu
    Vector2 desiredXPos = { e->pos.x + moveStep.x, e->pos.y };
    // Cek apakah posisi baru ini kakinya nabrak sesuatu
    if (CanEnemyMoveTo(desiredXPos, map, player, e, allEnemies, totalEnemyCount)) { 
        e->pos.x = desiredXPos.x; 
    }
    
    // Coba bergerak di sumbu Y (terpisah)
    Vector2 desiredYPos = { e->pos.x, e->pos.y + moveStep.y };
    if (CanEnemyMoveTo(desiredYPos, map, player, e, allEnemies, totalEnemyCount)) { 
        e->pos.y = desiredYPos.y; 
    }
    
    return (abs(moveStep.x) > 0.001f || abs(moveStep.y) > 0.001f);
}

void DrawEnemy(const Enemy *e) {
    if (!e->active || e->tex.id <= 0) return;
    float frameW = (float)e->tex.width / (float)e->frameCount, frameH = (float)e->tex.height / 4.0f;
    Rectangle sourceRec = { (float)e->currentFrame * frameW, (float)e->currentLine * frameH, frameW, frameH };
    Rectangle destRec = { e->pos.x, e->pos.y, e->width, e->height };
    Vector2 origin = { e->width / 2.0f, e->height / 2.0f };
    DrawTexturePro(e->tex, sourceRec, destRec, origin, 0.0f, WHITE);
}

void UnloadEnemy(Enemy *e) { if (e->tex.id > 0) UnloadTexture(e->tex); e->tex.id = 0; }

#endif // ENEMY_IMPLEMENTATION_ONCE
#endif // ENEMY_IMPLEMENTATION