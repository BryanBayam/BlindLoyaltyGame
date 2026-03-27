#include "raylib.h"
#include <math.h>
#include <stdbool.h>

#include "tilemap.h"
#include "character.h"
#include "menu.h"
#include "scene1.h"
#include "scene2.h"
#include "scene3.h"
#include "scene4.h"
#include "scene5.h"
#include "scene6.h"

// --- MEMBUKA KUNCI IMPLEMENTASI (WAJIB) ---
// Bagian ini memberi tahu kompiler C untuk mengubah file .h menjadi kode logika utuh
#define ENEMY_IMPLEMENTATION
#include "enemy.h"

#define BANDIT_IMPLEMENTATION
#include "bandit.h"

#define BOSS_BANDIT_IMPLEMENTATION
#include "boss_bandit.h"

#define ENEMY_MANAGER_IMPLEMENTATION
#include "enemy_manager.h"

#define PICKUP_IMPLEMENTATION
#include "pickup.h"

#define OBJECTIVE_IMPLEMENTATION
#include "objective.h"
// ------------------------------------------

#define PLAYER_HISTORY_SIZE 900

typedef enum {
    SCREEN_LOADING,
    SCREEN_MENU,
    SCREEN_SCENE1,
    SCREEN_SCENE2,
    SCREEN_SCENE3,
    SCREEN_SCENE4,
    SCREEN_SCENE5,
    SCREEN_SCENE6,
    SCREEN_GAMEPLAY
} GameScreen;

// --- FUNGSI UI ---
static void DrawDeathOverlay(int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    const char *title = "YOU DIED";
    const char *msg = "Press R to Restart";
    int titleSize = 64, msgSize = 28;
    int titleW = MeasureText(title, titleSize);
    int msgW = MeasureText(msg, msgSize);
    DrawText(title, (vWidth - titleW) / 2, vHeight / 2 - 60, titleSize, RED);
    DrawText(msg, (vWidth - msgW) / 2, vHeight / 2 + 20, msgSize, MAROON);
}

static void DrawWinOverlay(int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    const char *title = "VICTORY";
    const char *msg = "You found the key! Press R to Restart";
    int titleSize = 64, msgSize = 28;
    int titleW = MeasureText(title, titleSize);
    int msgW = MeasureText(msg, msgSize);
    DrawText(title, (vWidth - titleW) / 2, vHeight / 2 - 60, titleSize, GOLD);
    DrawText(msg, (vWidth - msgW) / 2, vHeight / 2 + 20, msgSize, YELLOW);
}

static void DrawInstructionsOverlay(int vWidth, int vHeight) {
    Rectangle panel = { vWidth * 0.18f, vHeight * 0.14f, vWidth * 0.64f, vHeight * 0.58f };
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.45f));
    DrawRectangleRounded(panel, 0.08f, 16, Fade(BLACK, 0.82f));
    DrawRectangleRoundedLinesEx(panel, 0.08f, 16, 3.0f, RAYWHITE);
    int x = (int)panel.x + 40, y = (int)panel.y + 30;
    DrawText("INSTRUCTIONS", x, y, 36, RAYWHITE);
    y += 70; DrawText("- Find the Key to win.", x, y, 26, YELLOW);
    y += 45; DrawText("- Press WASD to Move.", x, y, 26, RAYWHITE);
    y += 45; DrawText("- Press Shift + WASD to Run.", x, y, 26, RAYWHITE);
    y += 45; DrawText("- Avoid the BANDITS.", x, y, 26, RED);
    y += 80; DrawText("Press any key to begin", x, y, 28, GREEN);
}

// --- FUNGSI RESET GAME ---
static void ResetGameplay(Player *player, Tilemap *map,
                          Enemy regulars[], int regularCount, Enemy *boss,
                          Vector2 playerHistory[], int *historyIndex,
                          EnemySpawner *enemySpawner,
                          Camera2D *camera, KeyItem *key,
                          Pickup hearts[], Pickup speeds[],
                          Texture2D *heartTexture, Texture2D *speedTexture,
                          bool *gameWon, bool *showInstructions) {
    UnloadPlayer(player);
    InitPlayer(player, FindWalkableSpawn(map));

    for (int i = 0; i < regularCount; i++) {
        UnloadEnemy(&regulars[i]);
        InitBandit(&regulars[i], (Vector2){ 0.0f, 0.0f }); // PERBAIKAN: Gunakan InitBandit
        regulars[i].active = false; // Matikan dulu sampai di-spawn oleh manager
    }

    UnloadEnemy(boss);
    InitBossBandit(boss, (Vector2){ 0.0f, 0.0f }); // PERBAIKAN: Gunakan InitBossBandit
    boss->active = false;

    for (int i = 0; i < PLAYER_HISTORY_SIZE; i++) {
        playerHistory[i] = player->pos;
    }
    *historyIndex = 0;

    InitEnemySpawner(enemySpawner);
    camera->target = player->pos;

    ResetKey(key);
    SpawnPickups(map, hearts, HEART_COUNT, heartTexture, speeds, SPEED_COUNT, speedTexture);

    *gameWon = false;
    *showInstructions = true;
}

int main(void) {
    const int vWidth = 1280;
    const int vHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(vWidth, vHeight, "Blind Loyalty - C Edition");
    SetAudioStreamBufferSizeDefault(4096);
    InitAudioDevice();
    SetTargetFPS(60);

    GameScreen currentScreen = SCREEN_LOADING;
    int loadStep = 0;
    float loadProgress = 0.0f;

    Menu menu = { 0 };
    Tilemap map = { 0 };
    Player player = { 0 };

    Enemy regularBandits[MAX_REGULAR_ENEMIES] = { 0 };
    Enemy bossBandit = { 0 };

    Scene1 scene1 = { 0 }; Scene2 scene2 = { 0 }; Scene3 scene3 = { 0 };
    Scene4 scene4 = { 0 }; Scene5 scene5 = { 0 }; Scene6 scene6 = { 0 };

    Vector2 playerHistory[PLAYER_HISTORY_SIZE] = { 0 };
    int historyIndex = 0;

    EnemySpawner enemySpawner = { 0 };

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ vWidth / 2.0f, vHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 3.0f;

    RenderTexture2D target = LoadRenderTexture(vWidth, vHeight);

    Music menuMusic = { 0 }; Music storyMusic = { 0 }; Music inGameMusic = { 0 };
    Music *activeMusic = NULL;

    bool fadeOutMusic = false;
    float musicVolume = 1.0f;
    GameScreen nextScreenAfterFade = SCREEN_MENU;

    KeyItem key = { 0 };
    Texture2D heartTexture = { 0 }, speedTexture = { 0 };
    Pickup hearts[HEART_COUNT] = { 0 };
    Pickup speeds[SPEED_COUNT] = { 0 };

    bool gameWon = false;
    bool showInstructions = true;

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        float scale = fminf((float)GetScreenWidth() / vWidth, (float)GetScreenHeight() / vHeight);
        Vector2 vMouse = {
            (mouse.x - (GetScreenWidth() - (vWidth * scale)) * 0.5f) / scale,
            (mouse.y - (GetScreenHeight() - (vHeight * scale)) * 0.5f) / scale
        };
        bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        if (activeMusic != NULL) UpdateMusicStream(*activeMusic);

        if (fadeOutMusic && activeMusic != NULL) {
            musicVolume -= GetFrameTime();
            if (musicVolume <= 0.0f) {
                musicVolume = 0.0f; StopMusicStream(*activeMusic); fadeOutMusic = false;
                currentScreen = nextScreenAfterFade;

                if (currentScreen == SCREEN_SCENE1) activeMusic = &storyMusic;
                else if (currentScreen == SCREEN_GAMEPLAY) activeMusic = &inGameMusic;
                else if (currentScreen == SCREEN_MENU) activeMusic = &menuMusic;
                
                musicVolume = 1.0f; SetMusicVolume(*activeMusic, musicVolume); PlayMusicStream(*activeMusic);
            }
            SetMusicVolume(*activeMusic, musicVolume); goto render_phase;
        }

        if (currentScreen == SCREEN_LOADING) {
            switch (loadStep) {
                case 0:
                    menu.background = LoadTexture("images/Background/TitleBackground.PNG");
                    menuMusic = LoadMusicStream("Music/MainMenu.ogg"); storyMusic = LoadMusicStream("Music/Story.ogg"); inGameMusic = LoadMusicStream("Music/ingame.ogg");
                    key.texture = LoadTexture("images/Elements/key.png"); heartTexture = LoadTexture("images/Elements/heart.png"); speedTexture = LoadTexture("images/Elements/speed.png");
                    loadProgress = 0.12f; loadStep++; break;
                case 1:
                    if (!LoadTilemap(&map, "maps/map1/map1.json")) goto cleanup;
                    loadProgress = 0.24f; loadStep++; break;
                case 2:
                    InitPlayer(&player, FindWalkableSpawn(&map));
                    camera.target = player.pos;
                    for (int i = 0; i < PLAYER_HISTORY_SIZE; i++) playerHistory[i] = player.pos;
                    ResetKey(&key);
                    SpawnPickups(&map, hearts, HEART_COUNT, &heartTexture, speeds, SPEED_COUNT, &speedTexture);
                    loadProgress = 0.36f; loadStep++; break;
                case 3:
                    // PERBAIKAN: Gunakan InitBandit dan InitBossBandit
                    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) {
                        InitBandit(&regularBandits[i], (Vector2){0});
                        regularBandits[i].active = false;
                    }
                    InitBossBandit(&bossBandit, (Vector2){0});
                    bossBandit.active = false;
                    InitEnemySpawner(&enemySpawner);
                    loadProgress = 0.48f; loadStep++; break;
                case 4: InitScene1(&scene1); loadProgress = 0.58f; loadStep++; break;
                case 5: InitScene2(&scene2); loadProgress = 0.68f; loadStep++; break;
                case 6: InitScene3(&scene3); loadProgress = 0.78f; loadStep++; break;
                case 7: InitScene4(&scene4); loadProgress = 0.86f; loadStep++; break;
                case 8: InitScene5(&scene5); loadProgress = 0.94f; loadStep++; break;
                case 9: InitScene6(&scene6); loadProgress = 1.0f; loadStep++; break;
                case 10:
                    activeMusic = &menuMusic; SetMusicVolume(*activeMusic, 1.0f); PlayMusicStream(*activeMusic);
                    currentScreen = SCREEN_MENU; break;
            }
        }
        else if (currentScreen == SCREEN_MENU) {
            int action = UpdateMenu(&menu, vMouse, vWidth, vHeight);
            if (action == 1) { fadeOutMusic = true; nextScreenAfterFade = SCREEN_SCENE1; }
            if (action == 2) break;
        }
        else if (currentScreen >= SCREEN_SCENE1 && currentScreen <= SCREEN_SCENE6) {
            if (currentScreen == SCREEN_SCENE1) { UpdateScene1(&scene1, vMouse, mouseClicked, vWidth); if (scene1.currentState == SCENE1_DONE) currentScreen = SCREEN_SCENE2; }
            else if (currentScreen == SCREEN_SCENE2) { UpdateScene2(&scene2, vMouse, mouseClicked, vWidth); if (scene2.currentState == SCENE2_DONE) currentScreen = SCREEN_SCENE3; }
            else if (currentScreen == SCREEN_SCENE3) { UpdateScene3(&scene3, vMouse, mouseClicked, vWidth); if (scene3.currentState == SCENE3_DONE) currentScreen = SCREEN_SCENE4; }
            else if (currentScreen == SCREEN_SCENE4) { UpdateScene4(&scene4, vMouse, mouseClicked, vWidth); if (scene4.currentState == SCENE4_DONE) currentScreen = SCREEN_SCENE5; }
            else if (currentScreen == SCREEN_SCENE5) { UpdateScene5(&scene5, vMouse, mouseClicked, vWidth); if (scene5.currentState == SCENE5_DONE) currentScreen = SCREEN_SCENE6; }
            else if (currentScreen == SCREEN_SCENE6) { 
                UpdateScene6(&scene6, vMouse, mouseClicked, vWidth); 
                if (scene6.currentState == SCENE6_DONE) {
                    ResetGameplay(&player, &map, regularBandits, MAX_REGULAR_ENEMIES, &bossBandit,
                                  playerHistory, &historyIndex, &enemySpawner, &camera, &key, 
                                  hearts, speeds, &heartTexture, &speedTexture, &gameWon, &showInstructions);
                    fadeOutMusic = true; nextScreenAfterFade = SCREEN_GAMEPLAY;
                }
            }
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            bool gameplayPaused = showInstructions || player.isDead || gameWon;
            if (showInstructions && GetKeyPressed() != 0) showInstructions = false;

            if (!gameplayPaused) {
                float dt = GetFrameTime();

                UpdatePlayer(&player, &map);
                camera.target = player.pos;
                playerHistory[historyIndex] = player.pos;
                historyIndex = (historyIndex + 1) % PLAYER_HISTORY_SIZE;

                UpdateEnemySpawns(&enemySpawner, dt, regularBandits, MAX_REGULAR_ENEMIES, &bossBandit, &map, player.pos);
                UpdateKeyLogic(&key, dt, &map, player.pos, &gameWon);
                CheckPickupCollisions(&player, hearts, speeds);

                // PERBAIKAN: Gunakan UpdateBandit dan UpdateBossBandit secara spesifik
                for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) {
                    UpdateBandit(&regularBandits[i], regularBandits, MAX_REGULAR_ENEMIES, &player, &map);
                }
                UpdateBossBandit(&bossBandit, regularBandits, MAX_REGULAR_ENEMIES, &player, &map);
            }

            if ((player.isDead || gameWon) && IsKeyPressed(KEY_R)) {
                ResetGameplay(&player, &map, regularBandits, MAX_REGULAR_ENEMIES, &bossBandit,
                              playerHistory, &historyIndex, &enemySpawner, &camera, &key, 
                              hearts, speeds, &heartTexture, &speedTexture, &gameWon, &showInstructions);
            }
            if (IsKeyPressed(KEY_ESCAPE)) { fadeOutMusic = true; nextScreenAfterFade = SCREEN_MENU; }
        }

render_phase:
        BeginTextureMode(target);
            ClearBackground(BLACK);

            if (currentScreen == SCREEN_LOADING) {
                DrawText("LOADING ASSETS...", (vWidth / 2) - 150, (vHeight / 2) - 60, 40, RAYWHITE);
                Rectangle barBg = { (vWidth / 2.0f) - 200, (vHeight / 2.0f) + 10, 400, 30 };
                DrawRectangleRec(barBg, DARKGRAY);
                DrawRectangleRec((Rectangle){ barBg.x, barBg.y, 400 * loadProgress, 30 }, RAYWHITE);
                DrawRectangleLinesEx(barBg, 3.0f, LIGHTGRAY);
            }
            else if (currentScreen == SCREEN_MENU) DrawMenu(&menu, vWidth, vHeight);
            else if (currentScreen == SCREEN_SCENE1) DrawScene1(&scene1, vWidth, vHeight);
            else if (currentScreen == SCREEN_SCENE2) DrawScene2(&scene2, vWidth, vHeight);
            else if (currentScreen == SCREEN_SCENE3) DrawScene3(&scene3, vWidth, vHeight);
            else if (currentScreen == SCREEN_SCENE4) DrawScene4(&scene4, vWidth, vHeight);
            else if (currentScreen == SCREEN_SCENE5) DrawScene5(&scene5, vWidth, vHeight);
            else if (currentScreen == SCREEN_SCENE6) DrawScene6(&scene6, vWidth, vHeight);
            else if (currentScreen == SCREEN_GAMEPLAY) {
                BeginMode2D(camera);
                    DrawTilemapAll(&map);
                    
                    DrawKey(&key);
                    for (int i = 0; i < HEART_COUNT; i++) DrawPickup(&hearts[i]);
                    for (int i = 0; i < SPEED_COUNT; i++) DrawPickup(&speeds[i]);

                    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) DrawEnemy(&regularBandits[i]);
                    DrawEnemy(&bossBandit);
                    DrawPlayer(&player);
                EndMode2D();

                DrawPlayerUI(&player);
                DrawText("ESC TO MENU | SHIFT TO RUN", 10, vHeight - 30, 20, RAYWHITE);

                if (!showInstructions && !player.isDead && !gameWon) {
                    if (!enemySpawner.initialRegularsSpawned) DrawText("Bandits arrive in 5 seconds...", 10, vHeight - 90, 20, RED);
                    else if (!enemySpawner.bossSpawned) DrawText("Boss arrives in 7 seconds...", 10, vHeight - 90, 20, ORANGE);
                    if (!key.spawned) DrawText("Key will appear in 10 seconds...", 10, vHeight - 60, 20, YELLOW);
                }

                if (showInstructions) DrawInstructionsOverlay(vWidth, vHeight);
                if (player.isDead) DrawDeathOverlay(vWidth, vHeight);
                if (gameWon) DrawWinOverlay(vWidth, vHeight);
            }
            DrawCircleV(vMouse, 5, RED);
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(target.texture,
                (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height },
                (Rectangle){ (GetScreenWidth() - (vWidth * scale)) * 0.5f, (GetScreenHeight() - (vHeight * scale)) * 0.5f, vWidth * scale, vHeight * scale },
                (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

cleanup:
    UnloadScene1(&scene1); UnloadScene2(&scene2); UnloadScene3(&scene3);
    UnloadScene4(&scene4); UnloadScene5(&scene5); UnloadScene6(&scene6);
    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) UnloadEnemy(&regularBandits[i]);
    UnloadEnemy(&bossBandit);
    UnloadPlayer(&player); UnloadTilemap(&map);
    UnloadTexture(menu.background); UnloadTexture(key.texture);
    UnloadTexture(heartTexture); UnloadTexture(speedTexture);
    UnloadRenderTexture(target);
    UnloadMusicStream(menuMusic); UnloadMusicStream(storyMusic); UnloadMusicStream(inGameMusic);
    
    CloseAudioDevice();
    CloseWindow();
    return 0;
}