#ifndef OASIS_MEDIA_CEO_H
#define OASIS_MEDIA_CEO_H

#include "boss_bandit.h" // Includes the logic for a larger, stronger enemy

// Prepares the CEO boss at a starting location
void InitOasisMediaCEO(Enemy *boss, Vector2 startPos);
// Updates the CEO's movement and combat logic every frame
void UpdateOasisMediaCEO(Enemy *boss, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map);

#endif

#ifdef OASIS_MEDIA_CEO_IMPLEMENTATION
#ifndef OASIS_MEDIA_CEO_IMPLEMENTATION_ONCE
#define OASIS_MEDIA_CEO_IMPLEMENTATION_ONCE

// Setup function for the CEO
void InitOasisMediaCEO(Enemy *boss, Vector2 startPos) {
    // Start with the basic "Boss Bandit" settings (health, speed, etc.)
    InitBossBandit(boss, startPos);

    // Clean up any old texture memory to prevent the game from lagging/crashing
    if (boss->tex.id > 0) {
        UnloadTexture(boss->tex);
    }

    // Assign the specific CEO sprite sheet
    boss->tex = LoadTexture("images/Character/Oasis Media CEO/OasisCEO_walk.png");
}

// Logic update function
void UpdateOasisMediaCEO(Enemy *boss, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map) {
    // Uses the pre-existing Boss Bandit AI to hunt the player
    UpdateBossBandit(boss, allEnemies, totalEnemyCount, player, map);
}

#endif
#endif