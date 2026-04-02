#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "raylib.h"
#include <stdbool.h>

#include "tilemap.h"
#include "character.h"
#include "enemy.h"
#include "bandit.h"
#include "boss_bandit.h"
#include "enemy_manager.h"
#include "pickup.h"
#include "objective.h"
#include "menu.h" // Added to get the Keybinds struct

#define PLAYER_HISTORY_SIZE 900

// We bundle all gameplay variables into one clean state struct
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
} GameplayState;

// Declarations
void DrawDeathOverlay(int vWidth, int vHeight);
void DrawWinOverlay(int vWidth, int vHeight);
void DrawInstructionsOverlay(int vWidth, int vHeight);
void ResetGameplay(GameplayState *state);
void UpdateGameplay(GameplayState *state, Keybinds keys, Sound loseSfx, Sound winSfx, Music inGameMusic, bool *requestPause, bool *requestNextScene, bool mouseClicked);
void DrawGameplay(GameplayState *state, int vWidth, int vHeight);

#ifdef GAMEPLAY_IMPLEMENTATION

void DrawDeathOverlay(int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    DrawText("YOU DIED", (vWidth - MeasureText("YOU DIED", 64)) / 2, vHeight / 2 - 60, 64, RED);
    DrawText("Press R to Restart", (vWidth - MeasureText("Press R to Restart", 28)) / 2, vHeight / 2 + 20, 28, MAROON);
}

void DrawWinOverlay(int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    DrawText("VICTORY", (vWidth - MeasureText("VICTORY", 64)) / 2, vHeight / 2 - 60, 64, GOLD);
    DrawText("You found the key!", (vWidth - MeasureText("You found the key!", 28)) / 2, vHeight / 2 + 20, 28, YELLOW);
}

void DrawInstructionsOverlay(int vWidth, int vHeight) {
    Rectangle panel = { vWidth * 0.18f, vHeight * 0.14f, vWidth * 0.64f, vHeight * 0.58f };

    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.45f));
    DrawRectangleRounded(panel, 0.08f, 16, Fade(BLACK, 0.82f));
    DrawRectangleRoundedLinesEx(panel, 0.08f, 16, 3.0f, RAYWHITE);

    int x = (int)panel.x + 40;
    int y = (int)panel.y + 30;

    DrawText("INSTRUCTIONS", x, y, 36, RAYWHITE);
    y += 70;
    DrawText("- Find the Key to win.", x, y, 26, YELLOW);
    y += 45;
    DrawText("- Press keys you bound to Move.", x, y, 26, RAYWHITE);
    y += 45;
    DrawText("- Press bound Run key to Run.", x, y, 26, RAYWHITE);
    y += 45;
    DrawText("- Avoid the BANDITS.", x, y, 26, RED);
    y += 80;
    DrawText("Press any key or click to begin", x, y, 28, GREEN);
}

void ResetGameplay(GameplayState *state) {
    UnloadPlayer(&state->player);
    InitPlayer(&state->player, FindWalkableSpawn(&state->map));
    state->player.isDead = false;

    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) {
        UnloadEnemy(&state->regularBandits[i]);
        InitBandit(&state->regularBandits[i], (Vector2){ 0.0f, 0.0f });
        state->regularBandits[i].active = false;
    }

    UnloadEnemy(&state->bossBandit);
    InitBossBandit(&state->bossBandit, (Vector2){ 0.0f, 0.0f });
    state->bossBandit.active = false;

    for (int i = 0; i < PLAYER_HISTORY_SIZE; i++) {
        state->playerHistory[i] = state->player.pos;
    }

    state->historyIndex = 0;
    InitEnemySpawner(&state->enemySpawner);
    state->camera.target = state->player.pos;

    ResetKey(&state->key);
    SpawnPickups(&state->map, state->hearts, HEART_COUNT, &state->heartTexture, state->speeds, SPEED_COUNT, &state->speedTexture);

    state->gameWon = false;
    state->showInstructions = true;
    state->loseSfxPlayed = false;
    state->winSfxPlayed = false;
}

void UpdateGameplay(GameplayState *state, Keybinds keys, Sound loseSfx, Sound winSfx, Music inGameMusic, bool *requestPause, bool *requestNextScene, bool mouseClicked) {
    bool gameplayPaused = state->showInstructions || state->player.isDead || state->gameWon;
    
    if (state->showInstructions && (GetKeyPressed() != 0 || mouseClicked)) state->showInstructions = false;

    if (!gameplayPaused) {
        float dt = GetFrameTime();
        UpdatePlayer(&state->player, &state->map, keys);
        state->camera.target = state->player.pos;

        state->playerHistory[state->historyIndex] = state->player.pos;
        state->historyIndex = (state->historyIndex + 1) % PLAYER_HISTORY_SIZE;

        UpdateEnemySpawns(&state->enemySpawner, dt, state->regularBandits, MAX_REGULAR_ENEMIES, &state->bossBandit, &state->map, state->player.pos);
        UpdateKeyLogic(&state->key, dt, &state->map, state->player.pos, &state->gameWon);
        CheckPickupCollisions(&state->player, state->hearts, state->speeds);

        for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) {
            UpdateBandit(&state->regularBandits[i], state->regularBandits, MAX_REGULAR_ENEMIES, &state->player, &state->map);
        }

        UpdateBossBandit(&state->bossBandit, state->regularBandits, MAX_REGULAR_ENEMIES, &state->player, &state->map);

        if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) {
            *requestPause = true;
        }
    }

    if (state->player.isDead && !state->loseSfxPlayed) {
        state->loseSfxPlayed = true;
        state->winSfxPlayed = false;
        StopMusicStream(inGameMusic);
        PlaySound(loseSfx);
    }

    if (state->gameWon && !state->winSfxPlayed) {
        state->winSfxPlayed = true;
        state->loseSfxPlayed = false;
        PlaySound(winSfx);
        *requestNextScene = true;
    }

    if (state->player.isDead && IsKeyPressed(KEY_R)) {
        ResetGameplay(state);
        StopSound(loseSfx);
        StopSound(winSfx);
        SetMusicVolume(inGameMusic, 1.0f);
        PlayMusicStream(inGameMusic);
    }
}

void DrawGameplay(GameplayState *state, int vWidth, int vHeight) {
    BeginMode2D(state->camera);
        DrawTilemapAll(&state->map);
        DrawKey(&state->key);
        for (int i = 0; i < HEART_COUNT; i++) DrawPickup(&state->hearts[i]);
        for (int i = 0; i < SPEED_COUNT; i++) DrawPickup(&state->speeds[i]);
        for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) DrawEnemy(&state->regularBandits[i]);
        DrawEnemy(&state->bossBandit);
        DrawPlayer(&state->player);
    EndMode2D();

    DrawPlayerUI(&state->player);
    DrawText("PRESS M TO PAUSE", 10, vHeight - 30, 20, RAYWHITE);

    if (!state->showInstructions && !state->player.isDead && !state->gameWon) {
        if (!state->enemySpawner.initialRegularsSpawned) DrawText("Bandits arrive in 5 seconds...", 10, vHeight - 90, 20, RED);
        else if (!state->enemySpawner.bossSpawned) DrawText("Boss arrives in 7 seconds...", 10, vHeight - 90, 20, ORANGE);
        if (!state->key.spawned) DrawText("Key will appear in 10 seconds...", 10, vHeight - 60, 20, YELLOW);
    }
    
    if (state->showInstructions) DrawInstructionsOverlay(vWidth, vHeight);
    if (state->player.isDead) DrawDeathOverlay(vWidth, vHeight);
    if (state->gameWon) DrawWinOverlay(vWidth, vHeight);
}

#endif // GAMEPLAY_IMPLEMENTATION
#endif // GAMEPLAY_H // <--- This was missing!