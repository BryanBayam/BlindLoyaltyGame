    #ifndef ENEMY_H
    #define ENEMY_H

    #include "raylib.h"
    #include "raymath.h"
    #include "tilemap.h"
    #include "character.h"
    #include <math.h>
    #include <stdbool.h>
    #include <stddef.h>

    typedef enum EnemyType {
        ENEMY_REGULAR,
        ENEMY_BOSS
    } EnemyType;

    typedef enum EnemyState {
        ENEMY_PATROL,
        ENEMY_CHASE,
        ENEMY_RETREAT
    } EnemyState;

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
        float patrolTimer;
        Vector2 patrolTarget;

        EnemyType type;
        EnemyState state;
        float stateTimer;
    } Enemy;

    static inline Rectangle GetEnemyHitbox(Vector2 pos) {
        Rectangle hitbox = {
            pos.x - 7.0f,
            pos.y + 14.0f,
            14.0f,
            8.0f
        };
        return hitbox;
    }

    static inline Vector2 GetEnemyHitboxCenter(Vector2 pos) {
        Rectangle r = GetEnemyHitbox(pos);
        return (Vector2){ r.x + r.width * 0.5f, r.y + r.height * 0.5f };
    }

    static inline Vector2 GetPlayerHitboxCenter(const Player *player) {
        Rectangle r = GetPlayerHitbox(player->pos);
        return (Vector2){ r.x + r.width * 0.5f, r.y + r.height * 0.5f };
    }

    static inline bool IsEnemyInsideMap(Vector2 pos, const Tilemap *map) {
        Rectangle box = GetEnemyHitbox(pos);

        float mapPixelWidth = (float)map->width * (float)map->tileWidth;
        float mapPixelHeight = (float)map->height * (float)map->tileHeight;

        return box.x >= 0.0f &&
            box.y >= 0.0f &&
            box.x + box.width <= mapPixelWidth &&
            box.y + box.height <= mapPixelHeight;
    }

    static inline bool CanEnemyOccupy(Vector2 pos, const Tilemap *map) {
        Rectangle hitbox = GetEnemyHitbox(pos);

        if (!IsEnemyInsideMap(pos, map)) return false;
        if (CheckMapCollision(map, hitbox)) return false;

        return true;
    }

    static inline bool CanEnemySpawnAt(Vector2 pos, const Tilemap *map) {
        return CanEnemyOccupy(pos, map);
    }

    static inline bool WouldOverlapPlayer(Vector2 enemyPos, const Player *player) {
        Rectangle enemyBox = GetEnemyHitbox(enemyPos);
        Rectangle playerBox = GetPlayerHitbox(player->pos);
        return CheckCollisionRecs(enemyBox, playerBox);
    }

    static inline bool WouldOverlapOtherEnemy(Vector2 enemyPos,
                                            const Enemy *self,
                                            Enemy regulars[],
                                            int regularCount,
                                            Enemy *boss) {
        Rectangle selfBox = GetEnemyHitbox(enemyPos);

        for (int i = 0; i < regularCount; i++) {
            if (!regulars[i].active) continue;
            if (&regulars[i] == self) continue;

            Rectangle otherBox = GetEnemyHitbox(regulars[i].pos);
            if (CheckCollisionRecs(selfBox, otherBox)) return true;
        }

        if (boss != NULL && boss->active && boss != self) {
            Rectangle bossBox = GetEnemyHitbox(boss->pos);
            if (CheckCollisionRecs(selfBox, bossBox)) return true;
        }

        return false;
    }

    static inline bool CanEnemyMoveTo(Vector2 pos,
                                    const Tilemap *map,
                                    const Player *player,
                                    const Enemy *self,
                                    Enemy regulars[],
                                    int regularCount,
                                    Enemy *boss) {
        if (!CanEnemyOccupy(pos, map)) return false;
        if (WouldOverlapPlayer(pos, player)) return false;
        if (WouldOverlapOtherEnemy(pos, self, regulars, regularCount, boss)) return false;
        return true;
    }

    static inline bool IsWalkableTile(const Tilemap *map, int tx, int ty) {
        if (tx < 0 || ty < 0 || tx >= map->width || ty >= map->height) return false;
        return (map->wall[ty][tx] == 0);
    }

    static inline void WorldToTileFromPoint(const Tilemap *map, Vector2 point, int *tx, int *ty) {
        *tx = (int)(point.x / map->tileWidth);
        *ty = (int)(point.y / map->tileHeight);

        if (*tx < 0) *tx = 0;
        if (*ty < 0) *ty = 0;
        if (*tx >= map->width) *tx = map->width - 1;
        if (*ty >= map->height) *ty = map->height - 1;
    }

    static inline void EnemyToTile(const Tilemap *map, const Enemy *e, int *tx, int *ty) {
        WorldToTileFromPoint(map, GetEnemyHitboxCenter(e->pos), tx, ty);
    }

    static inline void PlayerToTile(const Tilemap *map, const Player *p, int *tx, int *ty) {
        WorldToTileFromPoint(map, GetPlayerHitboxCenter(p), tx, ty);
    }

    static inline Vector2 TileToWorldCenter(const Tilemap *map, int tx, int ty) {
        return (Vector2){
            tx * map->tileWidth + map->tileWidth * 0.5f,
            ty * map->tileHeight + map->tileHeight * 0.5f
        };
    }

    static inline bool IsCardinallyAdjacentToPlayer(const Enemy *e, const Player *player, const Tilemap *map) {
        int ex, ey, px, py;
        EnemyToTile(map, e, &ex, &ey);
        PlayerToTile(map, player, &px, &py);

        int dx = ex - px; if (dx < 0) dx = -dx;
        int dy = ey - py; if (dy < 0) dy = -dy;

        return ((dx + dy) == 1);
    }

    static inline void SetEnemyFacingFromDirection(Enemy *e, Vector2 direction) {
        if (fabsf(direction.x) > fabsf(direction.y)) {
            e->currentLine = (direction.x > 0.0f) ? 1 : 2;
        } else {
            e->currentLine = (direction.y < 0.0f) ? 0 : 3;
        }
    }

    static inline bool IsAttackGoalTile(const Tilemap *map, int tx, int ty, int playerTx, int playerTy) {
        if (!IsWalkableTile(map, tx, ty)) return false;

        int dx = tx - playerTx; if (dx < 0) dx = -dx;
        int dy = ty - playerTy; if (dy < 0) dy = -dy;

        return ((dx + dy) == 1);
    }

    static inline bool GetNextPathTileTowardAttackRange(const Enemy *e,
                                                        const Player *player,
                                                        const Tilemap *map,
                                                        Vector2 *outNextPoint) {
        int startX, startY, playerX, playerY;
        EnemyToTile(map, e, &startX, &startY);
        PlayerToTile(map, player, &playerX, &playerY);

        if (!IsWalkableTile(map, startX, startY)) return false;

        if (IsAttackGoalTile(map, startX, startY, playerX, playerY)) {
            *outNextPoint = TileToWorldCenter(map, startX, startY);
            return true;
        }

        bool visited[MAP_MAX_HEIGHT][MAP_MAX_WIDTH] = { false };
        int parentX[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
        int parentY[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];

        for (int y = 0; y < MAP_MAX_HEIGHT; y++) {
            for (int x = 0; x < MAP_MAX_WIDTH; x++) {
                parentX[y][x] = -1;
                parentY[y][x] = -1;
            }
        }

        int qx[MAP_MAX_WIDTH * MAP_MAX_HEIGHT];
        int qy[MAP_MAX_WIDTH * MAP_MAX_HEIGHT];
        int qHead = 0;
        int qTail = 0;

        qx[qTail] = startX;
        qy[qTail] = startY;
        qTail++;
        visited[startY][startX] = true;

        const int dx4[4] = { 1, -1, 0, 0 };
        const int dy4[4] = { 0, 0, 1, -1 };

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

            for (int i = 0; i < 4; i++) {
                int nx = cx + dx4[i];
                int ny = cy + dy4[i];

                if (!IsWalkableTile(map, nx, ny)) continue;
                if (visited[ny][nx]) continue;

                visited[ny][nx] = true;
                parentX[ny][nx] = cx;
                parentY[ny][nx] = cy;

                qx[qTail] = nx;
                qy[qTail] = ny;
                qTail++;
            }
        }

        if (foundX < 0 || foundY < 0) return false;

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

    static inline bool MoveEnemyTowardPoint(Enemy *e,
                                            Vector2 targetPoint,
                                            const Tilemap *map,
                                            const Player *player,
                                            Enemy regulars[],
                                            int regularCount,
                                            Enemy *boss,
                                            float dt) {
        Vector2 toTarget = Vector2Subtract(targetPoint, e->pos);
        float dist = Vector2Length(toTarget);

        if (dist <= 0.001f) return false;

        Vector2 dir = Vector2Scale(toTarget, 1.0f / dist);
        SetEnemyFacingFromDirection(e, dir);

        float maxStep = e->speed * dt;
        if (maxStep > dist) maxStep = dist;

        Vector2 candidate = {
            e->pos.x + dir.x * maxStep,
            e->pos.y + dir.y * maxStep
        };

        if (CanEnemyMoveTo(candidate, map, player, e, regulars, regularCount, boss)) {
            e->pos = candidate;
            return true;
        }

        return false;
    }

    static inline Vector2 FindPatrolPoint(const Enemy *e, const Tilemap *map) {
        int ex, ey;
        EnemyToTile(map, e, &ex, &ey);

        const int dirs[4][2] = {
            { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
        };

        for (int i = 0; i < 4; i++) {
            int nx = ex + dirs[i][0];
            int ny = ey + dirs[i][1];
            if (IsWalkableTile(map, nx, ny)) {
                return TileToWorldCenter(map, nx, ny);
            }
        }

        return e->pos;
    }

    static inline void InitEnemyCommon(Enemy *e, Vector2 startPos, EnemyType type) {
        e->pos = startPos;
        e->type = type;

        if (type == ENEMY_BOSS) {
            e->tex = LoadTexture("images/Character/Boss Bandit/BossBandit_walk.png");
        } else {
            e->tex = LoadTexture("images/Character/Bandit/Bandit_walk.png");
        }

        e->frameCount = 4;
        e->currentFrame = 0;
        e->currentLine = 3;
        e->frameTimer = 0.0f;
        e->frameSpeed = 0.14f;

        e->width = 48.0f;
        e->height = 48.0f;

        e->active = false;
        e->defeated = false;
        e->touchCooldown = 0.0f;
        e->state = ENEMY_PATROL;
        e->stateTimer = 0.0f;
        e->patrolTimer = 0.0f;
        e->patrolTarget = startPos;

        if (type == ENEMY_BOSS) {
            e->speed = 95.0f;
            e->damage = 24.0f;
            e->aggroRange = 220.0f;
            e->respawnAllowed = false;
        } else {
            e->speed = 78.0f;
            e->damage = 16.0f;
            e->aggroRange = 170.0f;
            e->respawnAllowed = true;
        }
    }

    static inline void InitEnemy(Enemy *e, Vector2 startPos) {
        InitEnemyCommon(e, startPos, ENEMY_REGULAR);
    }

    static inline void InitBossEnemy(Enemy *e, Vector2 startPos) {
        InitEnemyCommon(e, startPos, ENEMY_BOSS);
    }

    static inline void SpawnEnemy(Enemy *e, Vector2 pos, const Tilemap *map) {
        if (e->defeated) return;
        if (!CanEnemySpawnAt(pos, map)) return;

        e->pos = pos;
        e->currentFrame = 2;
        e->currentLine = 3;
        e->frameTimer = 0.0f;
        e->touchCooldown = 0.0f;
        e->state = ENEMY_PATROL;
        e->stateTimer = 0.0f;
        e->patrolTimer = 0.0f;
        e->patrolTarget = pos;
        e->active = true;
    }

    static inline void DespawnEnemy(Enemy *e) {
        e->active = false;
        e->touchCooldown = 0.0f;
        e->state = ENEMY_PATROL;
        e->stateTimer = 0.0f;
    }

    static inline void UpdateEnemy(Enemy *e,
                                Enemy regulars[],
                                int regularCount,
                                Enemy *boss,
                                Player *player,
                                const Tilemap *map) {
        if (!e->active) return;
        if (player->isDead) return;

        float dt = GetFrameTime();

        if (e->touchCooldown > 0.0f) {
            e->touchCooldown -= dt;
            if (e->touchCooldown < 0.0f) e->touchCooldown = 0.0f;
        }

        if (e->stateTimer > 0.0f) {
            e->stateTimer -= dt;
            if (e->stateTimer < 0.0f) e->stateTimer = 0.0f;
        }

        e->patrolTimer -= dt;
        if (e->patrolTimer < 0.0f) e->patrolTimer = 0.0f;

        float distToPlayer = Vector2Distance(GetEnemyHitboxCenter(e->pos), GetPlayerHitboxCenter(player));

        if (e->state != ENEMY_RETREAT) {
            if (distToPlayer <= e->aggroRange) {
                e->state = ENEMY_CHASE;
            } else {
                e->state = ENEMY_PATROL;
            }
        }

        bool moved = false;

        if (e->state == ENEMY_RETREAT) {
            if (e->stateTimer <= 0.0f) {
                e->state = (distToPlayer <= e->aggroRange) ? ENEMY_CHASE : ENEMY_PATROL;
            } else {
                int ex, ey, px, py;
                EnemyToTile(map, e, &ex, &ey);
                PlayerToTile(map, player, &px, &py);

                int dx = ex - px;
                int dy = ey - py;

                int retreatTx = ex;
                int retreatTy = ey;

                if (dx > 0) retreatTx = ex + 1;
                else if (dx < 0) retreatTx = ex - 1;
                else if (dy > 0) retreatTy = ey + 1;
                else if (dy < 0) retreatTy = ey - 1;

                if (IsWalkableTile(map, retreatTx, retreatTy)) {
                    Vector2 retreatTarget = TileToWorldCenter(map, retreatTx, retreatTy);
                    moved = MoveEnemyTowardPoint(e, retreatTarget, map, player, regulars, regularCount, boss, dt);
                }
            }
        }

        if (e->state == ENEMY_PATROL) {
            if (e->patrolTimer <= 0.0f || Vector2Distance(e->pos, e->patrolTarget) < 4.0f) {
                e->patrolTarget = FindPatrolPoint(e, map);
                e->patrolTimer = 1.0f;
            }
            moved = MoveEnemyTowardPoint(e, e->patrolTarget, map, player, regulars, regularCount, boss, dt);
        }

        if (e->state == ENEMY_CHASE) {
            if (!IsCardinallyAdjacentToPlayer(e, player, map)) {
                Vector2 nextPoint = { 0 };
                if (GetNextPathTileTowardAttackRange(e, player, map, &nextPoint)) {
                    moved = MoveEnemyTowardPoint(e, nextPoint, map, player, regulars, regularCount, boss, dt);
                }
            }

            if (IsCardinallyAdjacentToPlayer(e, player, map) && e->touchCooldown <= 0.0f) {
                float slowDuration = (e->type == ENEMY_BOSS) ? 2.5f : 1.5f;
                DamagePlayer(player, e->damage, slowDuration);
                e->touchCooldown = 1.0f;
                e->state = ENEMY_RETREAT;
                e->stateTimer = 2.0f;
            }
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