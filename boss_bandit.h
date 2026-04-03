#ifndef BOSS_BANDIT_H
#define BOSS_BANDIT_H

#include "enemy.h"

/*
 * Initializes the Boss Bandit enemy with default stats, animation settings,
 * behavior state, and combat properties.
 *
 * Parameters:
 * - boss: Pointer to the enemy instance to initialize.
 * - startPos: Initial spawn position of the boss.
 */
void InitBossBandit(Enemy *boss, Vector2 startPos);

/*
 * Updates the Boss Bandit every frame.
 *
 * Behavior overview:
 * - Patrols when the player is outside aggro range
 * - Chases when the player enters aggro range
 * - Retreats briefly after a successful attack
 *
 * Parameters:
 * - boss: Pointer to the boss enemy
 * - allEnemies: Array of all active enemies for movement/collision checks
 * - totalEnemyCount: Number of enemies in the array
 * - player: Pointer to the player object
 * - map: Pointer to the game tilemap used for navigation
 */
void UpdateBossBandit(Enemy *boss, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map);

#endif

#ifdef BOSS_BANDIT_IMPLEMENTATION
#ifndef BOSS_BANDIT_IMPLEMENTATION_ONCE
#define BOSS_BANDIT_IMPLEMENTATION_ONCE

void InitBossBandit(Enemy *boss, Vector2 startPos) {
    /* Clear the structure before assigning default values. */
    *boss = (Enemy){ 0 };

    /* Basic identity and spawn setup. */
    boss->pos = startPos;
    boss->type = ENEMY_BANDIT_BOSS;
    boss->active = true;

    /* Initial AI state and default patrol position. */
    boss->state = ENEMY_PATROL;
    boss->patrolTarget = startPos;

    /* Sprite and animation setup. */
    boss->frameCount = 4;
    boss->currentFrame = 0;
    boss->currentLine = 3;
    boss->width = 48.0f;
    boss->height = 48.0f;

    /*
     * Use the same folder naming style as the other Boss Bandit assets.
     * This fixes the invisible-boss issue when the previous path does not exist.
     */
    boss->tex = LoadTexture("images/Character/Boss Bandit/BossBandit_walk.png");

    boss->frameSpeed = 0.12f;

    /* Movement and combat tuning. */
    boss->speed = 95.0f;
    boss->damage = 24.0f;
    boss->aggroRange = 220.0f;
    boss->attackSlowDuration = 2.5f;

    /* Boss enemies should not respawn after defeat. */
    boss->respawnAllowed = false;

    /* Ensure the shared enemy attack sound is loaded before use. */
    EnsureEnemyAttackSfxLoaded();
}

void UpdateBossBandit(Enemy *boss, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map) {
    /* Skip update if the boss is inactive or the player is already dead. */
    if (!boss->active || player->isDead) return;

    float dt = GetFrameTime();

    /* Update cooldowns and internal timers each frame. */
    if (boss->touchCooldown > 0.0f) boss->touchCooldown = fmaxf(0.0f, boss->touchCooldown - dt);
    if (boss->stateTimer > 0.0f) boss->stateTimer = fmaxf(0.0f, boss->stateTimer - dt);
    if (boss->patrolTimer > 0.0f) boss->patrolTimer = fmaxf(0.0f, boss->patrolTimer - dt);

    /* Calculate distance from the boss to the player using hitbox centers. */
    float distToPlayer = Vector2Distance(
        GetHitboxCenter(GetEnemyHitbox(boss->pos)),
        GetHitboxCenter(GetPlayerHitbox(player->pos))
    );

    /*
     * Main state selection:
     * If the boss is not retreating, it either patrols or chases
     * depending on whether the player is inside the aggro range.
     */
    if (boss->state != ENEMY_RETREAT) {
        boss->state = (distToPlayer <= boss->aggroRange) ? ENEMY_CHASE : ENEMY_PATROL;
    }

    bool moved = false;

    if (boss->state == ENEMY_RETREAT) {
        /*
         * After attacking, the boss briefly retreats.
         * When the retreat timer ends, it returns to chase or patrol.
         */
        if (boss->stateTimer <= 0.0f) {
            boss->state = (distToPlayer <= boss->aggroRange) ? ENEMY_CHASE : ENEMY_PATROL;
        } else {
            int ex, ey, px, py;

            /* Convert enemy and player world positions to tile coordinates. */
            ex = (int)(GetHitboxCenter(GetEnemyHitbox(boss->pos)).x / map->tileWidth);
            ey = (int)(GetHitboxCenter(GetEnemyHitbox(boss->pos)).y / map->tileHeight);
            px = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).x / map->tileWidth);
            py = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).y / map->tileHeight);

            /*
             * Move one tile farther away from the player.
             * This creates a simple retreat behavior after an attack.
             */
            int retreatTx = ex + ((ex - px > 0) ? 1 : (ex - px < 0 ? -1 : 0));
            int retreatTy = ey + ((ey - py > 0) ? 1 : (ey - py < 0 ? -1 : 0));

            moved = MoveEnemyTowardPoint(
                boss,
                (Vector2){
                    retreatTx * map->tileWidth + map->tileWidth * 0.5f,
                    retreatTy * map->tileHeight + map->tileHeight * 0.5f
                },
                map, player, allEnemies, totalEnemyCount, dt
            );
        }
    } else if (boss->state == ENEMY_PATROL) {
        /*
         * Patrol behavior:
         * - Choose a new patrol point when the timer runs out
         *   or when the current target has been reached.
         * - Move toward that patrol point.
         */
        if (boss->patrolTimer <= 0.0f || Vector2Distance(boss->pos, boss->patrolTarget) < 4.0f) {
            boss->patrolTarget = FindPatrolPoint(boss, map);
            boss->patrolTimer = 1.0f + (float)GetRandomValue(0, 100) / 100.0f;
        }

        moved = MoveEnemyTowardPoint(
            boss,
            boss->patrolTarget,
            map,
            player,
            allEnemies,
            totalEnemyCount,
            dt
        );
    } else if (boss->state == ENEMY_CHASE) {
        int ex, ey, px, py;

        /* Convert enemy and player positions into tile coordinates. */
        ex = (int)(GetHitboxCenter(GetEnemyHitbox(boss->pos)).x / map->tileWidth);
        ey = (int)(GetHitboxCenter(GetEnemyHitbox(boss->pos)).y / map->tileHeight);
        px = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).x / map->tileWidth);
        py = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).y / map->tileHeight);

        /*
         * The boss can attack only when standing directly next to the player
         * in one of the four cardinal directions.
         */
        bool adjacent = (abs(ex - px) + abs(ey - py) == 1);

        /*
         * If not yet in attack range, move toward the next tile that brings
         * the boss closer to a valid attack position.
         */
        if (!adjacent) {
            Vector2 nextPoint;
            if (GetNextPathTileTowardAttackRange(boss, player, map, &nextPoint)) {
                moved = MoveEnemyTowardPoint(
                    boss,
                    nextPoint,
                    map,
                    player,
                    allEnemies,
                    totalEnemyCount,
                    dt
                );
            }
        }

        /*
         * If adjacent and the attack cooldown is ready:
         * - damage the player
         * - play attack sound
         * - start cooldown
         * - enter retreat state
         */
        if (adjacent && boss->touchCooldown <= 0.0f) {
            DamagePlayer(player, boss->damage, boss->attackSlowDuration);
            PlayEnemyAttackSfx();

            boss->touchCooldown = 1.0f;
            boss->state = ENEMY_RETREAT;
            boss->stateTimer = 1.0f;
        }
    }

    /*
     * Animation handling:
     * - Advance walking animation while moving
     * - Use a fixed idle frame when not moving
     */
    if (moved) {
        boss->frameTimer += dt;
        if (boss->frameTimer >= boss->frameSpeed) {
            boss->frameTimer = 0.0f;
            boss->currentFrame = (boss->currentFrame + 1) % boss->frameCount;
        }
    } else {
        boss->currentFrame = 2;
        boss->frameTimer = 0.0f;
    }
}

#endif
#endif