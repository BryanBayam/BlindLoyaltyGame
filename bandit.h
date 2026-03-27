#ifndef BANDIT_H
#define BANDIT_H

#include "enemy.h" // Membutuhkan pondasi enemy.h

void InitBandit(Enemy *e, Vector2 startPos);
void UpdateBandit(Enemy *e, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map);

#endif // BANDIT_H

#ifdef BANDIT_IMPLEMENTATION
#ifndef BANDIT_IMPLEMENTATION_ONCE
#define BANDIT_IMPLEMENTATION_ONCE  

void InitBandit(Enemy *e, Vector2 startPos) {
    *e = (Enemy){ 0 }; 
    e->pos = startPos; 
    e->type = ENEMY_BANDIT_REGULAR; 
    e->active = true;
    e->state = ENEMY_PATROL; 
    e->patrolTarget = startPos; 
    e->frameCount = 4; 
    e->currentLine = 3;
    e->width = 48.0f; 
    e->height = 48.0f;
    e->tex = LoadTexture("images/Character/Bandit/Bandit_walk.png");
    e->frameSpeed = 0.14f; 
    e->speed = 78.0f; 
    e->damage = 16.0f;
    
    // Jarak jangkauan mata Bandit untuk "Lock" Player
    e->aggroRange = 170.0f; 
    e->attackSlowDuration = 1.5f; 
    e->respawnAllowed = true;
}

void UpdateBandit(Enemy *e, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map) {
    if (!e->active || player->isDead) return;
    
    float dt = GetFrameTime();
    
    if (e->touchCooldown > 0.0f) e->touchCooldown = fmaxf(0.0f, e->touchCooldown - dt);
    if (e->stateTimer > 0.0f) e->stateTimer = fmaxf(0.0f, e->stateTimer - dt);
    if (e->patrolTimer > 0.0f) e->patrolTimer = fmaxf(0.0f, e->patrolTimer - dt);

    // Hitung jarak Bandit ke Player
    float distToPlayer = Vector2Distance(GetHitboxCenter(GetEnemyHitbox(e->pos)), GetHitboxCenter(GetPlayerHitbox(player->pos)));
    
    // --- SISTEM AGRO / LOCK PLAYER ---
    // Jika tidak sedang kabur (RETREAT), cek apakah Player masuk ke area jangkauan mata
    if (e->state != ENEMY_RETREAT) {
        if (distToPlayer <= e->aggroRange) {
            e->state = ENEMY_CHASE;  // Player terlihat! Langsung kunci dan kejar
        } else {
            e->state = ENEMY_PATROL; // Player jauh, kembali patroli santai
        }
    }

    bool moved = false;
    
    if (e->state == ENEMY_RETREAT) {
        if (e->stateTimer <= 0.0f) {
            e->state = (distToPlayer <= e->aggroRange) ? ENEMY_CHASE : ENEMY_PATROL;
        } else {
            int ex, ey, px, py; 
            ex = (int)(GetHitboxCenter(GetEnemyHitbox(e->pos)).x / map->tileWidth); 
            ey = (int)(GetHitboxCenter(GetEnemyHitbox(e->pos)).y / map->tileHeight);
            px = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).x / map->tileWidth); 
            py = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).y / map->tileHeight);
            int retreatTx = ex + ((ex - px > 0) ? 1 : (ex - px < 0 ? -1 : 0));
            int retreatTy = ey + ((ey - py > 0) ? 1 : (ey - py < 0 ? -1 : 0));
            moved = MoveEnemyTowardPoint(e, (Vector2){ retreatTx * map->tileWidth + map->tileWidth * 0.5f, retreatTy * map->tileHeight + map->tileHeight * 0.5f }, map, player, allEnemies, totalEnemyCount, dt);
        }
    } else if (e->state == ENEMY_PATROL) {
        if (e->patrolTimer <= 0.0f || Vector2Distance(e->pos, e->patrolTarget) < 4.0f) { 
            e->patrolTarget = FindPatrolPoint(e, map); 
            e->patrolTimer = 2.0f + (float)GetRandomValue(0, 200)/100.0f; 
        }
        moved = MoveEnemyTowardPoint(e, e->patrolTarget, map, player, allEnemies, totalEnemyCount, dt);
    } else if (e->state == ENEMY_CHASE) {
        int ex, ey, px, py;
        ex = (int)(GetHitboxCenter(GetEnemyHitbox(e->pos)).x / map->tileWidth); 
        ey = (int)(GetHitboxCenter(GetEnemyHitbox(e->pos)).y / map->tileHeight);
        px = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).x / map->tileWidth); 
        py = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).y / map->tileHeight);
        
        bool adjacent = (abs(ex - px) + abs(ey - py) == 1);
        
        if (!adjacent) { 
            Vector2 nextPoint; 
            if (GetNextPathTileTowardAttackRange(e, player, map, &nextPoint)) {
                moved = MoveEnemyTowardPoint(e, nextPoint, map, player, allEnemies, totalEnemyCount, dt); 
            }
        }
        
        if (adjacent && e->touchCooldown <= 0.0f) {
            DamagePlayer(player, e->damage, e->attackSlowDuration); 
            e->touchCooldown = 1.0f; 
            e->state = ENEMY_RETREAT; 
            e->stateTimer = 1.5f;
        }
    }

    if (moved) { 
        e->frameTimer += dt; 
        if (e->frameTimer >= e->frameSpeed) { 
            e->frameTimer = 0.0f; 
            e->currentFrame = (e->currentFrame + 1) % e->frameCount; 
        } 
    } else { 
        e->currentFrame = 2; 
        e->frameTimer = 0.0f; 
    }
}

#endif // BANDIT_IMPLEMENTATION_ONCE
#endif // BANDIT_IMPLEMENTATION