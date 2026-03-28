#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include "tilemap.h"
#include "menu.h"      
#include "character.h" 
#include "story_scene.h" 

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

#define PLAYER_HISTORY_SIZE 900

typedef enum {
    SCREEN_LOADING,
    SCREEN_MENU,
    SCREEN_LOAD_GAME, 
    SCREEN_SETTINGS,  
    SCREEN_SCENE1,
    SCREEN_SCENE2,
    SCREEN_SCENE3,
    SCREEN_SCENE4,
    SCREEN_SCENE5,
    SCREEN_SCENE6,
    SCREEN_GAMEPLAY,
    SCREEN_PAUSE,
    SCREEN_SAVE_GAME
} GameScreen;

// =========================================================
// DATA KONFIGURASI SEMUA SCENE (1 SAMPAI 6)
// =========================================================
SceneData scene1_data = {
    .bgPath = "images/Background/Scene/Scene1.png", .bgScrollSpeed = 0.0f,
    .doFadeIn = false, .doFadeOut = false,
    .portraitCount = 2,
    .portraits = { { "Commander", "images/Character/Commander/CommanderChat.png", 0.30f, 20.0f }, { "Reuben", "images/Character/Reuben/ReubenChat.png", 0.28f, 20.0f } },
    .choice1Text = "Yes Sir!", .choice2Text = "Yes Sir!",
    .narratorText = "Before the world called you a hero,\nyou were just a child who believed in order.",
    .narratorVoicePath = "audio/Voice/Scene 1/Narrator.mp3",
};

SceneData scene2_data = { .bgPath = "images/Background/Scene/Scene2.jpg", .bgScrollSpeed = -15.0f, .doFadeIn = true, .doFadeOut = true, };
SceneData scene3_data = { .bgPath = "images/Background/Scene/Scene3.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = false, .doFadeOut = false, .narratorText = "And that idea stayed with you...", .narratorVoicePath = "audio/Voice/Scene 3/Narrator part 2.mp3", };
SceneData scene4_data = { .bgPath = "images/Background/Scene/Scene4.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, };
SceneData scene5_data = { .bgPath = "images/Background/Scene/Scene5.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, };
SceneData scene6_data = { .bgPath = "images/Background/Scene/Scene6.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = false, .narratorText = "At least, that's what you were told.", .narratorVoicePath = "audio/Voice/Scene 6/Narrator part 2.mp3", .endPromptText = "PRESS ENTER TO START MISSION" };

// --- FUNGSI UI ---
static void DrawDeathOverlay(int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    DrawText("YOU DIED", (vWidth - MeasureText("YOU DIED", 64)) / 2, vHeight / 2 - 60, 64, RED);
    DrawText("Press R to Restart", (vWidth - MeasureText("Press R to Restart", 28)) / 2, vHeight / 2 + 20, 28, MAROON);
}

static void DrawWinOverlay(int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    DrawText("VICTORY", (vWidth - MeasureText("VICTORY", 64)) / 2, vHeight / 2 - 60, 64, GOLD);
    DrawText("You found the key! Press R to Restart", (vWidth - MeasureText("You found the key! Press R to Restart", 28)) / 2, vHeight / 2 + 20, 28, YELLOW);
}

static void DrawInstructionsOverlay(int vWidth, int vHeight) {
    Rectangle panel = { vWidth * 0.18f, vHeight * 0.14f, vWidth * 0.64f, vHeight * 0.58f };
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.45f));
    DrawRectangleRounded(panel, 0.08f, 16, Fade(BLACK, 0.82f));
    DrawRectangleRoundedLinesEx(panel, 0.08f, 16, 3.0f, RAYWHITE);
    int x = (int)panel.x + 40, y = (int)panel.y + 30;
    DrawText("INSTRUCTIONS", x, y, 36, RAYWHITE);
    y += 70; DrawText("- Find the Key to win.", x, y, 26, YELLOW);
    y += 45; DrawText("- Press keys you bound to Move.", x, y, 26, RAYWHITE);
    y += 45; DrawText("- Press bound Run key to Run.", x, y, 26, RAYWHITE);
    y += 45; DrawText("- Avoid the BANDITS.", x, y, 26, RED);
    y += 80; DrawText("Press any key to begin", x, y, 28, GREEN);
}

static void ResetGameplay(Player *player, Tilemap *map, Enemy regulars[], int regularCount, Enemy *boss, Vector2 playerHistory[], int *historyIndex, EnemySpawner *enemySpawner, Camera2D *camera, KeyItem *key, Pickup hearts[], Pickup speeds[], Texture2D *heartTexture, Texture2D *speedTexture, bool *gameWon, bool *showInstructions, bool *loseSfxPlayed, bool *winSfxPlayed) {
    UnloadPlayer(player);
    InitPlayer(player, FindWalkableSpawn(map));
    for (int i = 0; i < regularCount; i++) { UnloadEnemy(&regulars[i]); InitBandit(&regulars[i], (Vector2){ 0.0f, 0.0f }); regulars[i].active = false; }
    UnloadEnemy(boss); InitBossBandit(boss, (Vector2){ 0.0f, 0.0f }); boss->active = false;
    for (int i = 0; i < PLAYER_HISTORY_SIZE; i++) playerHistory[i] = player->pos;
    *historyIndex = 0;
    InitEnemySpawner(enemySpawner);
    camera->target = player->pos;
    ResetKey(key);
    SpawnPickups(map, hearts, HEART_COUNT, heartTexture, speeds, SPEED_COUNT, speedTexture);
    *gameWon = false; *showInstructions = true; *loseSfxPlayed = false; *winSfxPlayed = false;
}

int main(void) {
    const int vWidth = 1280, vHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(vWidth, vHeight, "Blind Loyalty - C Edition");
    SetAudioStreamBufferSizeDefault(4096);
    InitAudioDevice();
    SetTargetFPS(60);

    GameScreen currentScreen = SCREEN_LOADING;
    int loadStep = 0; float loadProgress = 0.0f;

    Menu menu = { 
        .masterVolume = 1.0f, .musicVolume = 1.0f,
        .keys = { KEY_W, KEY_S, KEY_A, KEY_D, KEY_LEFT_SHIFT },
        .activeLoadSlot = -1
    };
    
    Tilemap map = { 0 }; Player player = { 0 };
    Enemy regularBandits[MAX_REGULAR_ENEMIES] = { 0 }; Enemy bossBandit = { 0 };
    StoryScene scene1 = { 0 }; StoryScene scene2 = { 0 }; StoryScene scene3 = { 0 }; StoryScene scene4 = { 0 }; StoryScene scene5 = { 0 }; StoryScene scene6 = { 0 };
    Vector2 playerHistory[PLAYER_HISTORY_SIZE] = { 0 }; int historyIndex = 0;
    EnemySpawner enemySpawner = { 0 };
    Camera2D camera = { 0 }; camera.offset = (Vector2){ vWidth / 2.0f, vHeight / 2.0f }; camera.rotation = 0.0f; camera.zoom = 3.0f;
    RenderTexture2D target = LoadRenderTexture(vWidth, vHeight);
    Music menuMusic = { 0 }; Music storyMusic = { 0 }; Music inGameMusic = { 0 }; Music *activeMusic = NULL;
    Sound pressButtonSfx = { 0 }; Sound loseSfx = { 0 }; Sound winSfx = { 0 };
    bool fadeOutMusic = false; float musicVolume = 1.0f; GameScreen nextScreenAfterFade = SCREEN_MENU;
    KeyItem key = { 0 }; Texture2D heartTexture = { 0 }, speedTexture = { 0 };
    Pickup hearts[HEART_COUNT] = { 0 }; Pickup speeds[SPEED_COUNT] = { 0 };
    bool gameWon = false; bool showInstructions = true; bool loseSfxPlayed = false; bool winSfxPlayed = false; bool sfxLoaded = false;
    
    GameScreen pausedFromScreen = SCREEN_MENU; 

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        float scale = fminf((float)GetScreenWidth() / vWidth, (float)GetScreenHeight() / vHeight);
        Vector2 vMouse = { (mouse.x - (GetScreenWidth() - (vWidth * scale)) * 0.5f) / scale, (mouse.y - (GetScreenHeight() - (vHeight * scale)) * 0.5f) / scale };
        bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        bool enterPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);

        if (sfxLoaded && (enterPressed || mouseClicked)) PlaySound(pressButtonSfx);
        if (activeMusic != NULL && IsMusicStreamPlaying(*activeMusic)) UpdateMusicStream(*activeMusic);

        if (fadeOutMusic && activeMusic != NULL) {
            musicVolume -= GetFrameTime();
            if (musicVolume <= 0.0f) {
                musicVolume = 0.0f; StopMusicStream(*activeMusic); fadeOutMusic = false;
                currentScreen = nextScreenAfterFade;
                
                // FIXED: Story music will now play correctly for ANY scene!
                if (currentScreen >= SCREEN_SCENE1 && currentScreen <= SCREEN_SCENE6) activeMusic = &storyMusic;
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
                    menu.settingsBg = LoadTexture("images/GUI/Setting.png"); 
                    menu.saveBg = LoadTexture("images/GUI/Gamesave.png");     
                    menuMusic = LoadMusicStream("music/MainMenu.ogg"); storyMusic = LoadMusicStream("music/Story.ogg"); inGameMusic = LoadMusicStream("music/ingame.ogg");
                    pressButtonSfx = LoadSound("audio/Sfx/press_button.mp3"); loseSfx = LoadSound("audio/Sfx/lose.mp3"); winSfx = LoadSound("audio/Sfx/find_objective.mp3");
                    sfxLoaded = true;
                    key.texture = LoadTexture("images/Elements/key.png"); heartTexture = LoadTexture("images/Elements/heart.png"); speedTexture = LoadTexture("images/Elements/speed.png");
                    loadProgress = 0.12f; loadStep++; break;
                case 1: if (!LoadTilemap(&map, "maps/map1/map1.json")) goto cleanup; loadProgress = 0.24f; loadStep++; break;
                case 2: InitPlayer(&player, FindWalkableSpawn(&map)); camera.target = player.pos; for (int i = 0; i < PLAYER_HISTORY_SIZE; i++) playerHistory[i] = player.pos; ResetKey(&key); SpawnPickups(&map, hearts, HEART_COUNT, &heartTexture, speeds, SPEED_COUNT, &speedTexture); loadProgress = 0.36f; loadStep++; break;
                case 3: for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) { InitBandit(&regularBandits[i], (Vector2){0}); regularBandits[i].active = false; } InitBossBandit(&bossBandit, (Vector2){0}); bossBandit.active = false; InitEnemySpawner(&enemySpawner); loadProgress = 0.48f; loadStep++; break;
                case 4: LoadSceneDialogue("data/scene1.txt", &scene1_data); InitStoryScene(&scene1, &scene1_data); loadProgress = 0.58f; loadStep++; break;
                case 5: LoadSceneDialogue("data/scene2.txt", &scene2_data); InitStoryScene(&scene2, &scene2_data); loadProgress = 0.68f; loadStep++; break;
                case 6: LoadSceneDialogue("data/scene3.txt", &scene3_data); InitStoryScene(&scene3, &scene3_data); loadProgress = 0.78f; loadStep++; break;
                case 7: LoadSceneDialogue("data/scene4.txt", &scene4_data); InitStoryScene(&scene4, &scene4_data); loadProgress = 0.86f; loadStep++; break;
                case 8: LoadSceneDialogue("data/scene5.txt", &scene5_data); InitStoryScene(&scene5, &scene5_data); loadProgress = 0.94f; loadStep++; break;
                case 9: LoadSceneDialogue("data/scene6.txt", &scene6_data); InitStoryScene(&scene6, &scene6_data); loadProgress = 1.0f; loadStep++; break;
                case 10: activeMusic = &menuMusic; SetMusicVolume(*activeMusic, 1.0f); PlayMusicStream(*activeMusic); currentScreen = SCREEN_MENU; break;
            }
        }
        else if (currentScreen == SCREEN_MENU) {
            int action = UpdateMenu(&menu, vMouse, vWidth, vHeight);
            if (action == 1) { fadeOutMusic = true; nextScreenAfterFade = SCREEN_SCENE1; }
            if (action == 2) { RefreshSaveSlots(&menu); currentScreen = SCREEN_LOAD_GAME; menu.subSelected = 0; menu.activeLoadSlot = -1; } 
            if (action == 3) { pausedFromScreen = SCREEN_MENU; currentScreen = SCREEN_SETTINGS; menu.subSelected = 0; }  
            if (action == 4) break; 
        }
        else if (currentScreen == SCREEN_LOAD_GAME) {
            int action = UpdateLoadMenu(&menu, vMouse, vWidth, vHeight);
            if (action == 1) currentScreen = SCREEN_MENU;
            
            if (action >= 10 && action <= 13) {
                int slot = action - 10;
                GameSaveData data;
                if (LoadGameData(slot, &data)) {
                    ResetGameplay(&player, &map, regularBandits, MAX_REGULAR_ENEMIES, &bossBandit, playerHistory, &historyIndex, &enemySpawner, &camera, &key, hearts, speeds, &heartTexture, &speedTexture, &gameWon, &showInstructions, &loseSfxPlayed, &winSfxPlayed);
                    player.pos = data.playerPos; player.health = data.health; player.energy = data.energy;
                    camera.target = player.pos; showInstructions = false;
                    
                    fadeOutMusic = true; 
                    
                    // FIXED: Load back exactly into the screen that was saved!
                    nextScreenAfterFade = (GameScreen)data.savedScreen; 
                }
            }
        }
        else if (currentScreen == SCREEN_SETTINGS) {
            if (UpdateSettingsMenu(&menu, vMouse, vWidth, vHeight) == 1) currentScreen = pausedFromScreen;
            musicVolume = menu.musicVolume;
            if (activeMusic != NULL) SetMusicVolume(*activeMusic, musicVolume);
            SetMasterVolume(menu.masterVolume);
        }
        else if (currentScreen >= SCREEN_SCENE1 && currentScreen <= SCREEN_SCENE6) {
            if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) {
                pausedFromScreen = currentScreen;
                currentScreen = SCREEN_PAUSE;
                menu.subSelected = 0;
            } else {
                if (currentScreen == SCREEN_SCENE1) { UpdateStoryScene(&scene1, vMouse, mouseClicked, vWidth); if (scene1.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE2; }
                else if (currentScreen == SCREEN_SCENE2) { UpdateStoryScene(&scene2, vMouse, mouseClicked, vWidth); if (scene2.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE3; }
                else if (currentScreen == SCREEN_SCENE3) { UpdateStoryScene(&scene3, vMouse, mouseClicked, vWidth); if (scene3.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE4; }
                else if (currentScreen == SCREEN_SCENE4) { UpdateStoryScene(&scene4, vMouse, mouseClicked, vWidth); if (scene4.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE5; }
                else if (currentScreen == SCREEN_SCENE5) { UpdateStoryScene(&scene5, vMouse, mouseClicked, vWidth); if (scene5.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE6; }
                else if (currentScreen == SCREEN_SCENE6) { 
                    UpdateStoryScene(&scene6, vMouse, mouseClicked, vWidth); 
                    if (scene6.currentState == SCENE_STATE_DONE) {
                        ResetGameplay(&player, &map, regularBandits, MAX_REGULAR_ENEMIES, &bossBandit, playerHistory, &historyIndex, &enemySpawner, &camera, &key, hearts, speeds, &heartTexture, &speedTexture, &gameWon, &showInstructions, &loseSfxPlayed, &winSfxPlayed);
                        fadeOutMusic = true; nextScreenAfterFade = SCREEN_GAMEPLAY;
                    }
                }
            }
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            bool gameplayPaused = showInstructions || player.isDead || gameWon;
            if (showInstructions && GetKeyPressed() != 0) showInstructions = false;

            if (!gameplayPaused) {
                float dt = GetFrameTime();
                UpdatePlayer(&player, &map, menu.keys); 
                camera.target = player.pos;
                playerHistory[historyIndex] = player.pos; historyIndex = (historyIndex + 1) % PLAYER_HISTORY_SIZE;
                UpdateEnemySpawns(&enemySpawner, dt, regularBandits, MAX_REGULAR_ENEMIES, &bossBandit, &map, player.pos);
                UpdateKeyLogic(&key, dt, &map, player.pos, &gameWon); CheckPickupCollisions(&player, hearts, speeds);
                for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) UpdateBandit(&regularBandits[i], regularBandits, MAX_REGULAR_ENEMIES, &player, &map);
                UpdateBossBandit(&bossBandit, regularBandits, MAX_REGULAR_ENEMIES, &player, &map);
                
                if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) {
                    pausedFromScreen = currentScreen;
                    currentScreen = SCREEN_PAUSE;
                    menu.subSelected = 0;
                }
            }

            if (player.isDead && !loseSfxPlayed) { loseSfxPlayed = true; winSfxPlayed = false; StopMusicStream(inGameMusic); PlaySound(loseSfx); }
            if (gameWon && !winSfxPlayed) { winSfxPlayed = true; loseSfxPlayed = false; StopMusicStream(inGameMusic); PlaySound(winSfx); }

            if ((player.isDead || gameWon) && IsKeyPressed(KEY_R)) {
                ResetGameplay(&player, &map, regularBandits, MAX_REGULAR_ENEMIES, &bossBandit, playerHistory, &historyIndex, &enemySpawner, &camera, &key, hearts, speeds, &heartTexture, &speedTexture, &gameWon, &showInstructions, &loseSfxPlayed, &winSfxPlayed);
                StopSound(loseSfx); StopSound(winSfx); activeMusic = &inGameMusic; musicVolume = 1.0f; SetMusicVolume(inGameMusic, musicVolume); PlayMusicStream(inGameMusic);
            }
        }
        else if (currentScreen == SCREEN_PAUSE) {
            int action = UpdatePauseMenu(&menu, vMouse, vWidth, vHeight);
            
            if (action == 1) currentScreen = pausedFromScreen; 
            if (action == 2) { RefreshSaveSlots(&menu); currentScreen = SCREEN_SAVE_GAME; menu.subSelected = 0; }
            if (action == 3) { currentScreen = SCREEN_SETTINGS; menu.subSelected = 0; }
            if (action == 4) { fadeOutMusic = true; nextScreenAfterFade = SCREEN_MENU; }
        }
        else if (currentScreen == SCREEN_SAVE_GAME) {
            int action = UpdateSaveMenu(&menu, vMouse, vWidth, vHeight);
            if (action == 1) currentScreen = SCREEN_PAUSE;
            
            if (action >= 20 && action <= 23) {
                int slot = action - 20;
                GameSaveData data = {0};
                strcpy(data.name, menu.saveInput);
                data.playerPos = player.pos; data.health = player.health; data.energy = player.energy;
                
                // FIXED: Store the screen you were looking at right before you pressed 'Pause'
                data.savedScreen = (int)pausedFromScreen; 
                
                SaveGameData(slot, data);
                currentScreen = SCREEN_PAUSE;
            }
        }

render_phase:
        BeginTextureMode(target);
            ClearBackground(BLACK);

            GameScreen bgScreen = currentScreen;
            if (currentScreen == SCREEN_PAUSE || currentScreen == SCREEN_SAVE_GAME) {
                bgScreen = pausedFromScreen;
            }

            if (currentScreen == SCREEN_LOADING) {
                DrawText("LOADING ASSETS...", (vWidth / 2) - 150, (vHeight / 2) - 60, 40, RAYWHITE);
                Rectangle barBg = { (vWidth / 2.0f) - 200, (vHeight / 2.0f) + 10, 400, 30 };
                DrawRectangleRec(barBg, DARKGRAY); DrawRectangleRec((Rectangle){ barBg.x, barBg.y, 400 * loadProgress, 30 }, RAYWHITE);
                DrawRectangleLinesEx(barBg, 3.0f, LIGHTGRAY);
            }
            else if (currentScreen == SCREEN_MENU) DrawMenu(&menu, vWidth, vHeight);
            else if (currentScreen == SCREEN_LOAD_GAME) DrawLoadMenu(&menu, vWidth, vHeight);
            else if (currentScreen == SCREEN_SETTINGS) DrawSettingsMenu(&menu, vWidth, vHeight); 
            
            if (bgScreen >= SCREEN_SCENE1 && bgScreen <= SCREEN_SCENE6) {
                if (bgScreen == SCREEN_SCENE1) DrawStoryScene(&scene1, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE2) DrawStoryScene(&scene2, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE3) DrawStoryScene(&scene3, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE4) DrawStoryScene(&scene4, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE5) DrawStoryScene(&scene5, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE6) DrawStoryScene(&scene6, vWidth, vHeight);
            }
            else if (bgScreen == SCREEN_GAMEPLAY) {
                BeginMode2D(camera);
                    DrawTilemapAll(&map); DrawKey(&key);
                    for (int i = 0; i < HEART_COUNT; i++) DrawPickup(&hearts[i]);
                    for (int i = 0; i < SPEED_COUNT; i++) DrawPickup(&speeds[i]);
                    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) DrawEnemy(&regularBandits[i]);
                    DrawEnemy(&bossBandit); DrawPlayer(&player);
                EndMode2D();

                DrawPlayerUI(&player);
                DrawText("PRESS M TO PAUSE", 10, vHeight - 30, 20, RAYWHITE);

                if (currentScreen == SCREEN_GAMEPLAY) {
                    if (!showInstructions && !player.isDead && !gameWon) {
                        if (!enemySpawner.initialRegularsSpawned) DrawText("Bandits arrive in 5 seconds...", 10, vHeight - 90, 20, RED);
                        else if (!enemySpawner.bossSpawned) DrawText("Boss arrives in 7 seconds...", 10, vHeight - 90, 20, ORANGE);
                        if (!key.spawned) DrawText("Key will appear in 10 seconds...", 10, vHeight - 60, 20, YELLOW);
                    }
                    if (showInstructions) DrawInstructionsOverlay(vWidth, vHeight);
                    if (player.isDead) DrawDeathOverlay(vWidth, vHeight);
                    if (gameWon) DrawWinOverlay(vWidth, vHeight);
                }
            }

            if (currentScreen == SCREEN_PAUSE) {
                DrawPauseMenu(&menu, vWidth, vHeight);
            }
            if (currentScreen == SCREEN_SAVE_GAME) {
                DrawSaveMenu(&menu, vWidth, vHeight);
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
    UnloadStoryScene(&scene1); UnloadSceneData(&scene1_data);
    UnloadStoryScene(&scene2); UnloadSceneData(&scene2_data);
    UnloadStoryScene(&scene3); UnloadSceneData(&scene3_data);
    UnloadStoryScene(&scene4); UnloadSceneData(&scene4_data);
    UnloadStoryScene(&scene5); UnloadSceneData(&scene5_data);
    UnloadStoryScene(&scene6); UnloadSceneData(&scene6_data);
    
    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) UnloadEnemy(&regularBandits[i]);
    UnloadEnemy(&bossBandit); UnloadPlayer(&player); UnloadTilemap(&map);
    
    UnloadTexture(menu.background); 
    UnloadTexture(menu.settingsBg);
    UnloadTexture(menu.saveBg);    
    UnloadTexture(key.texture);
    UnloadTexture(heartTexture); UnloadTexture(speedTexture); UnloadRenderTexture(target);
    UnloadMusicStream(menuMusic); UnloadMusicStream(storyMusic); UnloadMusicStream(inGameMusic);
    UnloadEnemyAttackSfx(); UnloadSound(pressButtonSfx); UnloadSound(loseSfx); UnloadSound(winSfx);
    
    CloseAudioDevice(); CloseWindow();
    return 0;
}