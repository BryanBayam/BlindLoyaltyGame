#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "raylib.h"
#include <stdbool.h>

// Includes all the components needed for a level
#include "tilemap.h"
#include "character.h"
#include "enemy.h"
#include "bandit.h"
#include "boss_bandit.h"
#include "security_guard.h"
#include "oasis_media_ceo.h"
#include "enemy_manager.h"
#include "pickup.h"
#include "objective.h"
#include "menu.h"

#define PLAYER_HISTORY_SIZE 900 // Tracks the player's path for AI or effects

// Configuration for a specific level (e.g., Level 1 vs Level 2)
typedef struct GameplayConfig {
    bool useSecurityTheme;         // True if using guards instead of bandits
    const char *avoidEnemyText;     // Text for instructions (e.g. "AVOID BANDITS")
    const char *regularSpawnText;   // Text for arrival countdown
    const char *bossSpawnText;      // Text for boss arrival
} GameplayConfig;

// The "Big Container" for everything happening in a level
typedef struct {
    Tilemap map;
    Player player;
    Enemy regularBandits[MAX_REGULAR_ENEMIES];
    Enemy bossBandit;
    Vector2 playerHistory[PLAYER_HISTORY_SIZE];
    int historyIndex;
    EnemySpawner enemySpawner;
    Camera2D camera;
    KeyItem key;
    Texture2D heartTexture;
    Texture2D speedTexture;
    Pickup hearts[HEART_COUNT];
    Pickup speeds[SPEED_COUNT];
    bool gameWon;
    bool showInstructions;
    bool loseSfxPlayed;
    bool winSfxPlayed;
    const GameplayConfig *config; // Points to the specific level settings
} GameplayState;

// Functions to manage the level
void DrawDeathOverlay(int vWidth, int vHeight);
void DrawWinOverlay(const GameplayState *state, int vWidth, int vHeight);
void DrawInstructionsOverlay(const GameplayState *state, int vWidth, int vHeight);
void UpdateGameplay(GameplayState *state, Keybinds keys, Sound loseSfx, Sound winSfx, Sound heartPickSfx, Sound speedSfx, Music inGameMusic, bool *requestPause, bool *requestNextScene, bool mouseClicked);
void DrawGameplay(GameplayState *state, int vWidth, int vHeight);

#ifdef GAMEPLAY_IMPLEMENTATION

// Helper functions to get level-specific text or paths
static bool GameplayUsesSecurityTheme(const GameplayState *state) {
    return state->config != NULL && state->config->useSecurityTheme;
}

static const char *GameplayAvoidEnemyText(const GameplayState *state) {
    if (state->config != NULL && state->config->avoidEnemyText != NULL) {
        return state->config->avoidEnemyText;
    }
    return "BANDITS";
}

static const char *GameplayRegularSpawnText(const GameplayState *state) {
    if (state->config != NULL && state->config->regularSpawnText != NULL) {
        return state->config->regularSpawnText;
    }
    return "Bandits arrive in 5 seconds.";
}

static const char *GameplayBossSpawnText(const GameplayState *state) {
    if (state->config != NULL && state->config->bossSpawnText != NULL) {
        return state->config->bossSpawnText;
    }
    return "Boss arrives in 7 seconds.";
}

static const char *GameplayObjectiveName(const GameplayState *state) {
    if (GameplayUsesSecurityTheme(state)) {
        return "Document";
    }
    return "Key";
}

static const char *GameplayObjectiveTexturePath(const GameplayState *state) {
    if (GameplayUsesSecurityTheme(state)) {
        return "images/Elements/document.png";
    }
    return "images/Elements/key.png";
}

static const char *GameplayObjectiveAppearText(const GameplayState *state) {
    if (GameplayUsesSecurityTheme(state)) {
        return "Document will appear in 10 seconds.";
    }
    return "Key will appear in 10 seconds.";
}

// Visual screen when the player dies
void DrawDeathOverlay(int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    DrawText("YOU DIED", (vWidth - MeasureText("YOU DIED", 64)) / 2, vHeight / 2 - 60, 64, RED);
    DrawText("Press R to Restart", (vWidth - MeasureText("Press R to Restart", 28)) / 2, vHeight / 2 + 20, 28, MAROON);
}

// Visual screen when the player wins
void DrawWinOverlay(const GameplayState *state, int vWidth, int vHeight) {
    const char *objectiveName = GameplayObjectiveName(state);
    const char *message = GameplayUsesSecurityTheme(state) ? "You found the document!" : "You found the key!";

    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    DrawText("VICTORY", (vWidth - MeasureText("VICTORY", 64)) / 2, vHeight / 2 - 60, 64, GOLD);
    DrawText(message, (vWidth - MeasureText(message, 28)) / 2, vHeight / 2 + 20, 28, YELLOW);
    (void)objectiveName;
}

// The "How to Play" screen shown at the start of a level
void DrawInstructionsOverlay(const GameplayState *state, int vWidth, int vHeight) {
    Rectangle panel = { vWidth * 0.18f, vHeight * 0.14f, vWidth * 0.64f, vHeight * 0.58f };

    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.45f));
    DrawRectangleRounded(panel, 0.08f, 16, Fade(BLACK, 0.82f));
    DrawRectangleRoundedLinesEx(panel, 0.08f, 16, 3.0f, RAYWHITE);

    int x = (int)panel.x + 40;
    int y = (int)panel.y + 30;

    DrawText("INSTRUCTIONS", x, y, 36, RAYWHITE);
    y += 70;
    DrawText(TextFormat("- Find the %s to win.", GameplayObjectiveName(state)), x, y, 26, YELLOW);
    y += 45;
    DrawText("- Press keys you bound to Move.", x, y, 26, RAYWHITE);
    y += 45;
    DrawText("- Press Shift key to Run.", x, y, 26, RAYWHITE);
    y += 45;
    DrawText(TextFormat("- Avoid the %s.", GameplayAvoidEnemyText(state)), x, y, 26, RED);
    y += 80;
    DrawText("Press any key or click to begin", x, y, 28, GREEN);
}

// Resets everything to start a level fresh
void ResetGameplay(GameplayState *state) {
    UnloadPlayer(&state->player);
    InitPlayer(&state->player, FindWalkableSpawn(&state->map));
    state->player.isDead = false;

    // Set up regular enemies based on theme
    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) {
        UnloadEnemy(&state->regularBandits[i]);
        if (GameplayUsesSecurityTheme(state)) {
            InitSecurityGuard(&state->regularBandits[i], (Vector2){ 0.0f, 0.0f });
        } else {
            InitBandit(&state->regularBandits[i], (Vector2){ 0.0f, 0.0f });
        }
        state->regularBandits[i].active = false;
    }

    // Set up boss based on theme
    UnloadEnemy(&state->bossBandit);
    if (GameplayUsesSecurityTheme(state)) {
        InitOasisMediaCEO(&state->bossBandit, (Vector2){ 0.0f, 0.0f });
    } else {
        InitBossBandit(&state->bossBandit, (Vector2){ 0.0f, 0.0f });
    }
    state->bossBandit.active = false;

    // Reset player history and camera
    for (int i = 0; i < PLAYER_HISTORY_SIZE; i++) {
        state->playerHistory[i] = state->player.pos;
    }
    state->historyIndex = 0;
    InitEnemySpawner(&state->enemySpawner, GameplayUsesSecurityTheme(state));
    state->camera.target = state->player.pos;

    // Load objective textures (Key or Document)
    if (state->key.texture.id > 0) {
        UnloadTexture(state->key.texture);
    }
    state->key.texture = LoadTexture(GameplayObjectiveTexturePath(state));

    ResetKey(&state->key);
    SpawnPickups(&state->map, state->hearts, HEART_COUNT, &state->heartTexture, state->speeds, SPEED_COUNT, &state->speedTexture);

    state->gameWon = false;
    state->showInstructions = true;
    state->loseSfxPlayed = false;
    state->winSfxPlayed = false;
}

// The main loop for Level update logic
void UpdateGameplay(GameplayState *state, Keybinds keys, Sound loseSfx, Sound winSfx, Sound heartPickSfx, Sound speedSfx, Music inGameMusic, bool *requestPause, bool *requestNextScene, bool mouseClicked) {
    bool gameplayPaused = state->showInstructions || state->player.isDead || state->gameWon;

    // Dismiss instructions
    if (state->showInstructions && (GetKeyPressed() != 0 || mouseClicked)) state->showInstructions = false;

    if (!gameplayPaused) {
        float dt = GetFrameTime();
        UpdatePlayer(&state->player, &state->map, keys);
        state->camera.target = state->player.pos;

        // Save position to history
        state->playerHistory[state->historyIndex] = state->player.pos;
        state->historyIndex = (state->historyIndex + 1) % PLAYER_HISTORY_SIZE;

        // Logic for enemies, objectives, and items
        UpdateEnemySpawns(&state->enemySpawner, dt, state->regularBandits, MAX_REGULAR_ENEMIES, &state->bossBandit, &state->map, state->player.pos);
        UpdateKeyLogic(&state->key, dt, &state->map, state->player.pos, &state->gameWon);
        bool pickedHeart = false;
        bool pickedSpeed = false;
        CheckPickupCollisions(&state->player, state->hearts, state->speeds, &pickedHeart, &pickedSpeed);

        if (pickedHeart) PlaySound(heartPickSfx);
        if (pickedSpeed) PlaySound(speedSfx);

        // Update all regular enemies
        for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) {
            if (GameplayUsesSecurityTheme(state)) {
                UpdateSecurityGuard(&state->regularBandits[i], state->regularBandits, MAX_REGULAR_ENEMIES, &state->player, &state->map);
            } else {
                UpdateBandit(&state->regularBandits[i], state->regularBandits, MAX_REGULAR_ENEMIES, &state->player, &state->map);
            }
        }

        // Update Boss
        if (GameplayUsesSecurityTheme(state)) {
            UpdateOasisMediaCEO(&state->bossBandit, state->regularBandits, MAX_REGULAR_ENEMIES, &state->player, &state->map);
        } else {
            UpdateBossBandit(&state->bossBandit, state->regularBandits, MAX_REGULAR_ENEMIES, &state->player, &state->map);
        }

        // Handle pause request
        if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) {
            *requestPause = true;
        }
    }

    // Audio cues for Game Over
    if (state->player.isDead && !state->loseSfxPlayed) {
        state->loseSfxPlayed = true;
        state->winSfxPlayed = false;
        StopMusicStream(inGameMusic);
        PlaySound(loseSfx);
    }

    // Progression cue for Win
    if (state->gameWon && !state->winSfxPlayed) {
        state->winSfxPlayed = true;
        state->loseSfxPlayed = false;
        PlaySound(winSfx);
        *requestNextScene = true;
    }

    // Restart level
    if (state->player.isDead && IsKeyPressed(KEY_R)) {
        ResetGameplay(state);
        StopSound(loseSfx);
        StopSound(winSfx);
        SetMusicVolume(inGameMusic, 1.0f);
        PlayMusicStream(inGameMusic);
    }
}

// Drawing function for Top-Down exploration
void DrawGameplay(GameplayState *state, int vWidth, int vHeight) {
    BeginMode2D(state->camera); // Start drawing relative to the camera
        DrawTilemapAll(&state->map);
        DrawKey(&state->key);
        for (int i = 0; i < HEART_COUNT; i++) DrawPickup(&state->hearts[i]);
        for (int i = 0; i < SPEED_COUNT; i++) DrawPickup(&state->speeds[i]);
        for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) DrawEnemy(&state->regularBandits[i]);
        DrawEnemy(&state->bossBandit);
        DrawPlayer(&state->player);
    EndMode2D(); // End camera drawing

    DrawPlayerUI(&state->player);
    DrawText("PRESS M TO PAUSE", 10, vHeight - 30, 20, RAYWHITE);

    // Show arrival text/timers on screen
    if (!state->showInstructions && !state->player.isDead && !state->gameWon) {
        if (!state->enemySpawner.initialRegularsSpawned) {
            DrawText(GameplayRegularSpawnText(state), 10, vHeight - 90, 20, RED);
        } else if (!state->enemySpawner.bossSpawned) {
            DrawText(GameplayBossSpawnText(state), 10, vHeight - 90, 20, ORANGE);
        }

        if (!state->key.spawned) {
            DrawText(GameplayObjectiveAppearText(state), 10, vHeight - 60, 20, YELLOW);
        }
    }

    // Draw full-screen UI overlays
    if (state->showInstructions) DrawInstructionsOverlay(state, vWidth, vHeight);
    if (state->player.isDead) DrawDeathOverlay(vWidth, vHeight);
    if (state->gameWon) DrawWinOverlay(state, vWidth, vHeight);
}

#endif
#endif