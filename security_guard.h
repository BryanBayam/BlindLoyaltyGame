#ifndef SECURITY_GUARD_H
#define SECURITY_GUARD_H

#include "bandit.h" // Includes the base Bandit logic

// Prepares a Security Guard enemy at a specific starting position
void InitSecurityGuard(Enemy *e, Vector2 startPos);
// Runs the frame-by-frame logic (moving, attacking, etc.) for the Security Guard
void UpdateSecurityGuard(Enemy *e, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map);

#endif

// This section only runs if SECURITY_GUARD_IMPLEMENTATION is defined in your main file
#ifdef SECURITY_GUARD_IMPLEMENTATION
#ifndef SECURITY_GUARD_IMPLEMENTATION_ONCE
#define SECURITY_GUARD_IMPLEMENTATION_ONCE

// Sets up the initial stats and look of the Security Guard
void InitSecurityGuard(Enemy *e, Vector2 startPos) {
    // First, set up the guard using the standard Bandit settings
    InitBandit(e, startPos);

    // If there was already a texture loaded, clear it to prevent memory leaks
    if (e->tex.id > 0) {
        UnloadTexture(e->tex);
    }

    // Load the specific visual appearance for the Security Guard
    e->tex = LoadTexture("images/Character/Security Guard/Securityguard_walk.png");
}

// Every frame, the Security Guard behaves exactly like a Bandit
void UpdateSecurityGuard(Enemy *e, Enemy allEnemies[], int totalEnemyCount, Player *player, const Tilemap *map) {
    // This calls the Bandit movement/AI logic from bandit.h
    UpdateBandit(e, allEnemies, totalEnemyCount, player, map);
}

#endif
#endif