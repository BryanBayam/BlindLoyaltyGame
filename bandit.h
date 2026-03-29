#ifndef BANDIT_H
#define BANDIT_H

#include "enemy.h"

/*
 * Initialize a Bandit enemy instance with its default stats,
 * animation settings, and behavior configuration.
 *
 * Parameters:
 * - e: Pointer to the enemy object to initialize.
 * - startPos: Initial world position of the bandit.
 */
void InitBandit(Enemy *e, Vector2 startPos);

/*
 * Update Bandit behavior every frame.
 *
 * The Bandit uses a simple state machine:
 * - ENEMY_PATROL: Roams between patrol points.
 * - ENEMY_CHASE: Moves toward the player when inside aggro range.
 * - ENEMY_RETREAT: Steps away briefly after attacking.
 *
 * Parameters:
 * - e: Pointer to the bandit enemy.
 * - allEnemies: Array of all enemies in the scene for collision/avoidance.
 * - totalEnemyCount: Total number of enemies in the array.
 * - player: Pointer to the player object.
 * - map: Pointer to the tilemap used for pathing and movement.
 */
void UpdateBandit(Enemy *e, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map);

#endif

#ifdef BANDIT_IMPLEMENTATION
#ifndef BANDIT_IMPLEMENTATION_ONCE
#define BANDIT_IMPLEMENTATION_ONCE

void InitBandit(Enemy *e, Vector2 startPos) {
    /* Reset the full enemy structure before assigning values. */
    *e = (Enemy){ 0 };

    /* Core identity and spawn setup. */
    e->pos = startPos;
    e->type = ENEMY_BANDIT_REGULAR;
    e->active = true;
    e->state = ENEMY_PATROL;
    e->patrolTarget = startPos;

    /* Sprite and animation configuration. */
    e->frameCount = 4;
    e->currentLine = 3;
    e->width = 48.0f;
    e->height = 48.0f;
    e->tex = LoadTexture("images/Character/Bandit/Bandit_walk.png");
    e->frameSpeed = 0.14f;

    /* Combat and movement tuning. */
    e->speed = 78.0f;
    e->damage = 16.0f;
    e->aggroRange = 170.0f;
    e->attackSlowDuration = 1.5f;

    /* Allow this enemy to respawn when the game system supports it. */
    e->respawnAllowed = true;

    /* Ensure shared enemy attack sound effect is ready before use. */
    EnsureEnemyAttackSfxLoaded();
}

void UpdateBandit(Enemy *e, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map) {
    /* Do not process inactive enemies or update if the player is already dead. */
    if (!e->active || player->isDead) return;

    float dt = GetFrameTime();

    /* Reduce all time-based cooldowns and timers each frame. */
    if (e->touchCooldown > 0.0f) e->touchCooldown = fmaxf(0.0f, e->touchCooldown - dt);
    if (e->stateTimer > 0.0f) e->stateTimer = fmaxf(0.0f, e->stateTimer - dt);
    if (e->patrolTimer > 0.0f) e->patrolTimer = fmaxf(0.0f, e->patrolTimer - dt);

    /* Measure distance between the bandit and the player using hitbox centers. */
    float distToPlayer = Vector2Distance(
        GetHitboxCenter(GetEnemyHitbox(e->pos)),
        GetHitboxCenter(GetPlayerHitbox(player->pos))
    );

    /*
     * State selection:
     * If the bandit is not currently retreating, it switches between
     * patrol and chase depending on whether the player is within aggro range.
     */
    if (e->state != ENEMY_RETREAT) {
        if (distToPlayer <= e->aggroRange) {
            e->state = ENEMY_CHASE;
        } else {
            e->state = ENEMY_PATROL;
        }
    }

    bool moved = false;

    if (e->state == ENEMY_RETREAT) {
        /*
         * After attacking, the bandit retreats for a short duration.
         * Once the retreat timer ends, it returns to either patrol or chase.
         */
        if (e->stateTimer <= 0.0f) {
            e->state = (distToPlayer <= e->aggroRange) ? ENEMY_CHASE : ENEMY_PATROL;
        } else {
            int ex, ey, px, py;

            /* Convert world positions into tile coordinates. */
            ex = (int)(GetHitboxCenter(GetEnemyHitbox(e->pos)).x / map->tileWidth);
            ey = (int)(GetHitboxCenter(GetEnemyHitbox(e->pos)).y / map->tileHeight);
            px = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).x / map->tileWidth);
            py = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).y / map->tileHeight);

            /*
             * Choose a tile one step farther away from the player.
             * This gives the bandit a simple retreat behavior.
             */
            int retreatTx = ex + ((ex - px > 0) ? 1 : (ex - px < 0 ? -1 : 0));
            int retreatTy = ey + ((ey - py > 0) ? 1 : (ey - py < 0 ? -1 : 0));

            moved = MoveEnemyTowardPoint(
                e,
                (Vector2){
                    retreatTx * map->tileWidth + map->tileWidth * 0.5f,
                    retreatTy * map->tileHeight + map->tileHeight * 0.5f
                },
                map, player, allEnemies, totalEnemyCount, dt
            );
        }
    } else if (e->state == ENEMY_PATROL) {
        /*
         * Patrol behavior:
         * - Pick a new patrol point when the timer expires
         *   or when the current target has been reached.
         * - Move toward that patrol point.
         */
        if (e->patrolTimer <= 0.0f || Vector2Distance(e->pos, e->patrolTarget) < 4.0f) {
            e->patrolTarget = FindPatrolPoint(e, map);
            e->patrolTimer = 2.0f + (float)GetRandomValue(0, 200) / 100.0f;
        }

        moved = MoveEnemyTowardPoint(
            e,
            e->patrolTarget,
            map,
            player,
            allEnemies,
            totalEnemyCount,
            dt
        );
    } else if (e->state == ENEMY_CHASE) {
        int ex, ey, px, py;

        /* Convert enemy and player positions into tile coordinates. */
        ex = (int)(GetHitboxCenter(GetEnemyHitbox(e->pos)).x / map->tileWidth);
        ey = (int)(GetHitboxCenter(GetEnemyHitbox(e->pos)).y / map->tileHeight);
        px = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).x / map->tileWidth);
        py = (int)(GetHitboxCenter(GetPlayerHitbox(player->pos)).y / map->tileHeight);

        /*
         * A bandit attacks only when standing in a tile directly adjacent
         * to the player (4-directional adjacency).
         */
        bool adjacent = (abs(ex - px) + abs(ey - py) == 1);

        /*
         * If not already in attack range, move toward the next path tile
         * that leads into attack position.
         */
        if (!adjacent) {
            Vector2 nextPoint;
            if (GetNextPathTileTowardAttackRange(e, player, map, &nextPoint)) {
                moved = MoveEnemyTowardPoint(
                    e,
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
         * If adjacent and attack cooldown has expired:
         * - damage the player,
         * - play attack sound,
         * - start cooldown,
         * - switch to retreat state.
         */
        if (adjacent && e->touchCooldown <= 0.0f) {
            DamagePlayer(player, e->damage, e->attackSlowDuration);
            PlayEnemyAttackSfx();

            e->touchCooldown = 1.0f;
            e->state = ENEMY_RETREAT;
            e->stateTimer = 1.5f;
        }
    }

    /*
     * Update animation:
     * - Advance walk frames while moving.
     * - Use a fixed idle frame when standing still.
     */
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

#endif
#endif