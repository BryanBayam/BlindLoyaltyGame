#ifndef BOSS_BANDIT_H
#define BOSS_BANDIT_H

#include "enemy.h" // Membutuhkan pondasi enemy.h

void InitBossBandit(Enemy *boss, Vector2 startPos);
void UpdateBossBandit(Enemy *boss, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map);

#endif // BOSS_BANDIT_H

#ifdef BOSS_BANDIT_IMPLEMENTATION
#ifndef BOSS_BANDIT_IMPLEMENTATION_ONCE
#define BOSS_BANDIT_IMPLEMENTATION_ONCE
void InitBossBandit(Enemy *boss, Vector2 startPos) {
    *boss = (Enemy){ 0 }; boss->pos = startPos; boss->type = ENEMY_BANDIT_BOSS; boss->active = true;
    boss->state = ENEMY_PATROL; boss->patrolTarget = startPos; boss->frameCount = 4; boss->currentLine = 3;
    boss->width = 48.0f; boss->height = 48.0f;
    boss->tex = LoadTexture("images/Character/Boss Bandit/BossBandit_walk.png");
    boss->frameSpeed = 0.12f; boss->speed = 95.0f; boss->damage = 24.0f;
    boss->aggroRange = 220.0f; boss->attackSlowDuration = 2.5f; boss->respawnAllowed = false;
}

void UpdateBossBandit(Enemy *boss, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map) {
    if (!boss->active || player->isDead) return;
    float dt = GetFrameTime();
    if (boss->touchCooldown > 0.0f) boss->touchCooldown = fmaxf(0.0f, boss->touchCooldown - dt);
    if (boss->stateTimer > 0.0f) boss->stateTimer = fmaxf(0.0f, boss->stateTimer - dt);
    if (boss->patrolTimer > 0.0f) boss->patrolTimer = fmaxf(0.0f, boss->patrolTimer - dt);

    float distToPlayer = Vector2Distance(GetHitboxCenter(GetEnemyHitbox(boss->pos)), GetHitboxCenter(GetPlayerHitbox(player->pos)));
    if (boss->state != ENEMY_RETREAT) boss->state = (distToPlayer <= boss->aggroRange) ? ENEMY_CHASE : ENEMY_PATROL;

    bool moved = false;
    if (boss->state == ENEMY_RETREAT) {
        if (boss->stateTimer <= 0.0f) boss->state = (distToPlayer <= boss->aggroRange) ? ENEMY_CHASE : ENEMY_PATROL;
        else {
            int ex, ey, px, py;
            ex = (int)(GetHitboxCenter(GetEnemyHitbox(boss->pos)).x / map->tileWidth); ey = (int)(GetHitboxCenter(GetEnemyHitbox(boss->pos)).y / map->tileHeight);
            px = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).x / map->tileWidth); py = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).y / map->tileHeight);
            int retreatTx = ex + ((ex - px > 0) ? 1 : (ex - px < 0 ? -1 : 0)), retreatTy = ey + ((ey - py > 0) ? 1 : (ey - py < 0 ? -1 : 0));
            moved = MoveEnemyTowardPoint(boss, (Vector2){ retreatTx * map->tileWidth + map->tileWidth * 0.5f, retreatTy * map->tileHeight + map->tileHeight * 0.5f }, map, player, allEnemies, totalEnemyCount, dt);
        }
    } else if (boss->state == ENEMY_PATROL) {
        // Boss patroli lebih cepat jedanya
        if (boss->patrolTimer <= 0.0f || Vector2Distance(boss->pos, boss->patrolTarget) < 4.0f) { boss->patrolTarget = FindPatrolPoint(boss, map); boss->patrolTimer = 1.0f + (float)GetRandomValue(0, 100)/100.0f; }
        moved = MoveEnemyTowardPoint(boss, boss->patrolTarget, map, player, allEnemies, totalEnemyCount, dt);
    } else if (boss->state == ENEMY_CHASE) {
        int ex, ey, px, py;
        ex = (int)(GetHitboxCenter(GetEnemyHitbox(boss->pos)).x / map->tileWidth); ey = (int)(GetHitboxCenter(GetEnemyHitbox(boss->pos)).y / map->tileHeight);
        px = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).x / map->tileWidth); py = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).y / map->tileHeight);
        bool adjacent = (abs(ex - px) + abs(ey - py) == 1);
        
        if (!adjacent) { Vector2 nextPoint; if (GetNextPathTileTowardAttackRange(boss, player, map, &nextPoint)) moved = MoveEnemyTowardPoint(boss, nextPoint, map, player, allEnemies, totalEnemyCount, dt); }
        if (adjacent && boss->touchCooldown <= 0.0f) {
            DamagePlayer(player, boss->damage, boss->attackSlowDuration); 
            boss->touchCooldown = 1.0f; boss->state = ENEMY_RETREAT; boss->stateTimer = 1.0f; // Boss mundur lebih sebentar
        }
    }

    if (moved) { boss->frameTimer += dt; if (boss->frameTimer >= boss->frameSpeed) { boss->frameTimer = 0.0f; boss->currentFrame = (boss->currentFrame + 1) % boss->frameCount; } } 
    else { boss->currentFrame = 2; boss->frameTimer = 0.0f; }
}
#endif // BOSS_BANDIT_IMPLEMENTATION
#endif