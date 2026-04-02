#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

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

#define GAMEPLAY_IMPLEMENTATION
#include "gameplay.h" // <-- Including your new gameplay logic

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
    SCREEN_SAVE_GAME,
    SCREEN_SCENE7,
    SCREEN_SCENE8,
    SCREEN_SCENE9,
    SCREEN_SCENE10,
    SCREEN_SCENE11
} GameScreen;

SceneData scene1_data = {
    .bgPath = "images/Background/Scene/Scene1.png",
    .bgScrollSpeed = 0.0f,
    .doFadeIn = false,
    .doFadeOut = false,
    .portraitCount = 2,
    .portraits = {
        { "Commander", "images/Character/Commander/CommanderChat.png", 0.30f, 20.0f },
        { "Reuben", "images/Character/Reuben/ReubenChat.png", 0.28f, 20.0f }
    },
    .choiceText = "Yes Commander!",
    .narratorText = "Before the world called you a hero,\nyou were just a child who believed in order.",
    .narratorVoicePath = "audio/Voice/Scene 1/Narrator.mp3",
};

SceneData scene2_data = {
    .bgPath = "images/Background/Scene/Scene2.jpg",
    .bgScrollSpeed = -15.0f,
    .doFadeIn = true,
    .doFadeOut = true,
};

SceneData scene3_data = {
    .bgPath = "images/Background/Scene/Scene3.jpg",
    .bgScrollSpeed = 0.0f,
    .doFadeIn = false,
    .doFadeOut = false,
    .narratorText = "And that idea stayed with you...",
    .narratorVoicePath = "audio/Voice/Scene 3/Narrator part 2.mp3",
};

SceneData scene4_data = {
    .bgPath = "images/Background/Scene/Scene4.jpg",
    .bgScrollSpeed = 0.0f,
    .doFadeIn = true,
    .doFadeOut = true,
};

SceneData scene5_data = {
    .bgPath = "images/Background/Scene/Scene5.jpg",
    .bgScrollSpeed = 0.0f,
    .doFadeIn = true,
    .doFadeOut = true,
};

SceneData scene6_data = {
    .bgPath = "images/Background/Scene/Scene6.jpg",
    .bgScrollSpeed = 0.0f,
    .doFadeIn = true,
    .doFadeOut = true,
    .narratorText = "At least, that's what you were told.",
    .narratorVoicePath = "audio/Voice/Scene 6/Narrator part 2.mp3"
};

SceneData scene7_data = {
    .bgPath = "images/Background/Scene/Scene7_9.jpg",
    .bgScrollSpeed = 0.0f,
    .doFadeIn = true,
    .doFadeOut = true,
    .portraitCount = 2,
    .portraits = {
        { "Reuben", "images/Character/Reuben/ReubenChat.png", 0.28f, 20.0f }, 
        { "Boss Bandit", "images/Character/Boss_Bandit/BossBanditChat.png", 0.32f, 20.0f }
    },
};

SceneData scene8_data = {
    .bgPath = "images/Background/Scene/Scene8.jpg",
    .bgScrollSpeed = 0.0f,
    .doFadeIn = true,
    .doFadeOut = true,
    .portraitCount = 1,
    .portraits = {
        { "Soldier", "images/Character/Soldier/SoldierChat.png", 0.30f, 20.0f } 
    }
};

SceneData scene9_data = {
    .bgPath = "images/Background/Scene/Scene7_9.jpg",
    .bgScrollSpeed = 0.0f,
    .doFadeIn = true,
    .doFadeOut = true,
    .portraitCount = 2,
    .portraits = {
        { "Reuben", "images/Character/Reuben/ReubenChat.png", 0.28f, 20.0f },
        { "Commander", "images/Character/Commander/CommanderChat.png", 0.32f, 20.0f }
    }
};

SceneData scene10_data = {
    .bgPath = "images/Background/Scene/Scene10.jpg",
    .bgScrollSpeed = 0.0f,
    .doFadeIn = true,
    .doFadeOut = true,
    .portraitCount = 1,
    .portraits = {
        { "Commander", "images/Character/Commander/CommanderChat.png", 0.32f, 20.0f }
    },
    .narratorText = "(Next day)\n*Loud knocking sound*",
};

SceneData scene11_data = {
    .bgPath = "images/Background/Scene/Scene10.jpg",
    .bgScrollSpeed = 0.0f,
    .doFadeIn = true,
    .doFadeOut = true,
    .portraitCount = 2,
    .portraits = {
        { "Reuben", "images/Character/Reuben/ReubenChat.png", 0.28f, 20.0f },
        { "Commander", "images/Character/Commander/CommanderChat.png", 0.32f, 20.0f }
    },
    .choiceText = "Yes Sir! I'll try my best!",
    .endPromptText = "PRESS ENTER TO CONTINUE"
};

static void ResetAllScenes(
    StoryScene* s1, SceneData* sd1, StoryScene* s2, SceneData* sd2,
    StoryScene* s3, SceneData* sd3, StoryScene* s4, SceneData* sd4,
    StoryScene* s5, SceneData* sd5, StoryScene* s6, SceneData* sd6,
    StoryScene* s7, SceneData* sd7, StoryScene* s8, SceneData* sd8,
    StoryScene* s9, SceneData* sd9, StoryScene* s10, SceneData* sd10,
    StoryScene* s11, SceneData* sd11) 
{
    InitStoryScene(s1, sd1);
    InitStoryScene(s2, sd2);
    InitStoryScene(s3, sd3);
    InitStoryScene(s4, sd4);
    InitStoryScene(s5, sd5);
    InitStoryScene(s6, sd6);
    InitStoryScene(s7, sd7);
    InitStoryScene(s8, sd8);
    InitStoryScene(s9, sd9);
    InitStoryScene(s10, sd10);
    InitStoryScene(s11, sd11);
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

    Menu menu = {
        .masterVolume = 1.0f,
        .musicVolume = 1.0f,
        .keys = { KEY_W, KEY_S, KEY_A, KEY_D, KEY_LEFT_SHIFT },
        .activeLoadSlot = -1
    };

    // The new bundled gameplay state struct!
    GameplayState gameState = { 0 };

    StoryScene scene1 = { 0 };
    StoryScene scene2 = { 0 };
    StoryScene scene3 = { 0 };
    StoryScene scene4 = { 0 };
    StoryScene scene5 = { 0 };
    StoryScene scene6 = { 0 };
    StoryScene scene7 = { 0 };
    StoryScene scene8 = { 0 };
    StoryScene scene9 = { 0 };
    StoryScene scene10 = { 0 };
    StoryScene scene11 = { 0 };

    RenderTexture2D target = LoadRenderTexture(vWidth, vHeight);

    Music menuMusic = { 0 };
    Music storyMusic = { 0 };
    Music inGameMusic = { 0 };
    Music *activeMusic = NULL;

    Sound pressButtonSfx = { 0 };
    Sound loseSfx = { 0 };
    Sound winSfx = { 0 };

    bool fadeOutMusic = false;
    float musicVolume = 1.0f;
    GameScreen nextScreenAfterFade = SCREEN_MENU;
    bool sfxLoaded = false;

    GameScreen pausedFromScreen = SCREEN_MENU;

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        float scale = fminf((float)GetScreenWidth() / vWidth, (float)GetScreenHeight() / vHeight);
        Vector2 vMouse = {
            (mouse.x - (GetScreenWidth() - (vWidth * scale)) * 0.5f) / scale,
            (mouse.y - (GetScreenHeight() - (vHeight * scale)) * 0.5f) / scale
        };

        bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        bool enterPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);

        if (sfxLoaded && (enterPressed || mouseClicked)) {
            PlaySound(pressButtonSfx);
        }

        if (activeMusic != NULL && IsMusicStreamPlaying(*activeMusic)) {
            UpdateMusicStream(*activeMusic);
        }

        if (fadeOutMusic && activeMusic != NULL) {
            musicVolume -= GetFrameTime();

            if (musicVolume <= 0.0f) {
                musicVolume = 0.0f;
                StopMusicStream(*activeMusic);
                fadeOutMusic = false;
                currentScreen = nextScreenAfterFade;

                if ((currentScreen >= SCREEN_SCENE1 && currentScreen <= SCREEN_SCENE6) || (currentScreen >= SCREEN_SCENE7 && currentScreen <= SCREEN_SCENE11)) {
                    activeMusic = &storyMusic;
                } else if (currentScreen == SCREEN_GAMEPLAY) {
                    activeMusic = &inGameMusic;
                } else if (currentScreen == SCREEN_MENU) {
                    activeMusic = &menuMusic;
                }

                musicVolume = 1.0f;
                SetMusicVolume(*activeMusic, musicVolume);
                PlayMusicStream(*activeMusic);
            }

            SetMusicVolume(*activeMusic, musicVolume);
            goto render_phase;
        }

        if (currentScreen == SCREEN_LOADING) {
            switch (loadStep) {
                case 0:
                    menu.background = LoadTexture("images/Background/TitleBackground.PNG");
                    menu.settingsBg = LoadTexture("images/GUI/Setting.png");
                    menu.saveBg = LoadTexture("images/GUI/Gamesave.png");
                    menuMusic = LoadMusicStream("music/MainMenu.ogg");
                    storyMusic = LoadMusicStream("music/Story.ogg");
                    inGameMusic = LoadMusicStream("music/ingame.ogg");
                    pressButtonSfx = LoadSound("audio/Sfx/press_button.mp3");
                    loseSfx = LoadSound("audio/Sfx/lose.mp3");
                    winSfx = LoadSound("audio/Sfx/find_objective.mp3");
                    sfxLoaded = true;
                    gameState.key.texture = LoadTexture("images/Elements/key.png");
                    gameState.heartTexture = LoadTexture("images/Elements/heart.png");
                    gameState.speedTexture = LoadTexture("images/Elements/speed.png");
                    loadProgress = 0.10f;
                    loadStep++;
                    break;
                case 1:
                    if (!LoadTilemap(&gameState.map, "maps/map1/map1.json")) goto cleanup;
                    loadProgress = 0.18f;
                    loadStep++;
                    break;
                case 2:
                    InitPlayer(&gameState.player, FindWalkableSpawn(&gameState.map));
                    gameState.camera.offset = (Vector2){ vWidth / 2.0f, vHeight / 2.0f };
                    gameState.camera.rotation = 0.0f;
                    gameState.camera.zoom = 3.0f;
                    gameState.camera.target = gameState.player.pos;
                    for (int i = 0; i < PLAYER_HISTORY_SIZE; i++) gameState.playerHistory[i] = gameState.player.pos;
                    ResetKey(&gameState.key);
                    SpawnPickups(&gameState.map, gameState.hearts, HEART_COUNT, &gameState.heartTexture, gameState.speeds, SPEED_COUNT, &gameState.speedTexture);
                    loadProgress = 0.25f;
                    loadStep++;
                    break;
                case 3:
                    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) {
                        InitBandit(&gameState.regularBandits[i], (Vector2){ 0 });
                        gameState.regularBandits[i].active = false;
                    }
                    InitBossBandit(&gameState.bossBandit, (Vector2){ 0 });
                    gameState.bossBandit.active = false;
                    InitEnemySpawner(&gameState.enemySpawner);
                    loadProgress = 0.32f;
                    loadStep++;
                    break;
                case 4: LoadSceneDialogue("data/scene1.txt", &scene1_data); InitStoryScene(&scene1, &scene1_data); loadProgress = 0.40f; loadStep++; break;
                case 5: LoadSceneDialogue("data/scene2.txt", &scene2_data); InitStoryScene(&scene2, &scene2_data); loadProgress = 0.48f; loadStep++; break;
                case 6: LoadSceneDialogue("data/scene3.txt", &scene3_data); InitStoryScene(&scene3, &scene3_data); loadProgress = 0.55f; loadStep++; break;
                case 7: LoadSceneDialogue("data/scene4.txt", &scene4_data); InitStoryScene(&scene4, &scene4_data); loadProgress = 0.62f; loadStep++; break;
                case 8: LoadSceneDialogue("data/scene5.txt", &scene5_data); InitStoryScene(&scene5, &scene5_data); loadProgress = 0.70f; loadStep++; break;
                case 9: LoadSceneDialogue("data/scene6.txt", &scene6_data); InitStoryScene(&scene6, &scene6_data); loadProgress = 0.75f; loadStep++; break;
                case 10: LoadSceneDialogue("data/scene7.txt", &scene7_data); InitStoryScene(&scene7, &scene7_data); loadProgress = 0.80f; loadStep++; break;
                case 11: LoadSceneDialogue("data/scene8.txt", &scene8_data); InitStoryScene(&scene8, &scene8_data); loadProgress = 0.85f; loadStep++; break;
                case 12: LoadSceneDialogue("data/scene9.txt", &scene9_data); InitStoryScene(&scene9, &scene9_data); loadProgress = 0.90f; loadStep++; break;
                case 13: LoadSceneDialogue("data/scene10.txt", &scene10_data); InitStoryScene(&scene10, &scene10_data); loadProgress = 0.95f; loadStep++; break;
                case 14: LoadSceneDialogue("data/scene11.txt", &scene11_data); InitStoryScene(&scene11, &scene11_data); loadProgress = 1.0f; loadStep++; break;
                case 15:
                    activeMusic = &menuMusic;
                    SetMusicVolume(*activeMusic, 1.0f);
                    PlayMusicStream(*activeMusic);
                    currentScreen = SCREEN_MENU;
                    break;
            }
        }
        else if (currentScreen == SCREEN_MENU) {
            int action = UpdateMenu(&menu, vMouse, vWidth, vHeight);
            if (action == 1) { 
                ResetAllScenes(
                    &scene1, &scene1_data, &scene2, &scene2_data, &scene3, &scene3_data, 
                    &scene4, &scene4_data, &scene5, &scene5_data, &scene6, &scene6_data, 
                    &scene7, &scene7_data, &scene8, &scene8_data, &scene9, &scene9_data, 
                    &scene10, &scene10_data, &scene11, &scene11_data
                );
                fadeOutMusic = true; 
                nextScreenAfterFade = SCREEN_SCENE1; 
            }
            if (action == 2) { RefreshSaveSlots(&menu); currentScreen = SCREEN_LOAD_GAME; menu.subSelected = 0; menu.activeLoadSlot = -1; }
            if (action == 3) { pausedFromScreen = SCREEN_MENU; currentScreen = SCREEN_SETTINGS; menu.subSelected = 0; }
            if (action == 4) { break; }
        }
        else if (currentScreen == SCREEN_LOAD_GAME) {
            int action = UpdateLoadMenu(&menu, vMouse, vWidth, vHeight);
            if (action == 1) currentScreen = SCREEN_MENU;
            if (action >= 10 && action <= 13) {
                int slot = action - 10;
                GameSaveData data;
                if (LoadGameData(slot, &data)) {
                    ResetGameplay(&gameState);
                    gameState.player.pos = data.playerPos;
                    gameState.player.health = data.health;
                    gameState.player.energy = data.energy;
                    gameState.camera.target = gameState.player.pos;
                    gameState.showInstructions = false;
                    fadeOutMusic = true;
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
        else if ((currentScreen >= SCREEN_SCENE1 && currentScreen <= SCREEN_SCENE6) || (currentScreen >= SCREEN_SCENE7 && currentScreen <= SCREEN_SCENE11)) {
            if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) {
                pausedFromScreen = currentScreen;
                currentScreen = SCREEN_PAUSE;
                menu.subSelected = 0;
            } else {
                if (currentScreen == SCREEN_SCENE1) {
                    UpdateStoryScene(&scene1, vMouse, mouseClicked, vWidth);
                    if (scene1.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE2;
                }
                else if (currentScreen == SCREEN_SCENE2) {
                    UpdateStoryScene(&scene2, vMouse, mouseClicked, vWidth);
                    if (scene2.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE3;
                }
                else if (currentScreen == SCREEN_SCENE3) {
                    UpdateStoryScene(&scene3, vMouse, mouseClicked, vWidth);
                    if (scene3.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE4;
                }
                else if (currentScreen == SCREEN_SCENE4) {
                    UpdateStoryScene(&scene4, vMouse, mouseClicked, vWidth);
                    if (scene4.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE5;
                }
                else if (currentScreen == SCREEN_SCENE5) {
                    UpdateStoryScene(&scene5, vMouse, mouseClicked, vWidth);
                    if (scene5.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE6;
                }
                else if (currentScreen == SCREEN_SCENE6) {
                    UpdateStoryScene(&scene6, vMouse, mouseClicked, vWidth);
                    if (scene6.currentState == SCENE_STATE_DONE) {
                        ResetGameplay(&gameState);
                        fadeOutMusic = true;
                        nextScreenAfterFade = SCREEN_GAMEPLAY;
                    }
                }
                else if (currentScreen == SCREEN_SCENE7) { 
                    UpdateStoryScene(&scene7, vMouse, mouseClicked, vWidth);
                    if (scene7.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE8;
                }
                else if (currentScreen == SCREEN_SCENE8) { 
                    UpdateStoryScene(&scene8, vMouse, mouseClicked, vWidth);
                    if (scene8.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE9;
                }
                else if (currentScreen == SCREEN_SCENE9) { 
                    UpdateStoryScene(&scene9, vMouse, mouseClicked, vWidth);
                    if (scene9.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE10;
                }
                else if (currentScreen == SCREEN_SCENE10) { 
                    UpdateStoryScene(&scene10, vMouse, mouseClicked, vWidth);
                    if (scene10.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE11;
                }
                else if (currentScreen == SCREEN_SCENE11) { 
                    UpdateStoryScene(&scene11, vMouse, mouseClicked, vWidth);
                    if (scene11.currentState == SCENE_STATE_DONE) {
                        fadeOutMusic = true;
                        nextScreenAfterFade = SCREEN_MENU;
                    }
                }
            }
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            bool requestPause = false;
            bool requestNextScene = false;

            // Notice how clean this is now!
            UpdateGameplay(&gameState, menu.keys, loseSfx, winSfx, inGameMusic, &requestPause, &requestNextScene, mouseClicked);

            if (requestPause) {
                pausedFromScreen = currentScreen;
                currentScreen = SCREEN_PAUSE;
                menu.subSelected = 0;
            }

            if (requestNextScene) {
                fadeOutMusic = true;
                nextScreenAfterFade = SCREEN_SCENE7; 
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
                GameSaveData data = { 0 };
                strcpy(data.name, menu.saveInput);
                
                // Saving from our new struct
                data.playerPos = gameState.player.pos;
                data.health = gameState.player.health;
                data.energy = gameState.player.energy;
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
                DrawRectangleRec(barBg, DARKGRAY);
                DrawRectangleRec((Rectangle){ barBg.x, barBg.y, 400 * loadProgress, 30 }, RAYWHITE);
                DrawRectangleLinesEx(barBg, 3.0f, LIGHTGRAY);
            }
            else if (currentScreen == SCREEN_MENU) DrawMenu(&menu, vWidth, vHeight);
            else if (currentScreen == SCREEN_LOAD_GAME) DrawLoadMenu(&menu, vWidth, vHeight);
            else if (currentScreen == SCREEN_SETTINGS) DrawSettingsMenu(&menu, vWidth, vHeight);

            if ((bgScreen >= SCREEN_SCENE1 && bgScreen <= SCREEN_SCENE6) || (bgScreen >= SCREEN_SCENE7 && bgScreen <= SCREEN_SCENE11)) {
                if (bgScreen == SCREEN_SCENE1) DrawStoryScene(&scene1, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE2) DrawStoryScene(&scene2, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE3) DrawStoryScene(&scene3, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE4) DrawStoryScene(&scene4, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE5) DrawStoryScene(&scene5, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE6) DrawStoryScene(&scene6, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE7) DrawStoryScene(&scene7, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE8) DrawStoryScene(&scene8, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE9) DrawStoryScene(&scene9, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE10) DrawStoryScene(&scene10, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE11) DrawStoryScene(&scene11, vWidth, vHeight);
            }
            else if (bgScreen == SCREEN_GAMEPLAY) {
                // Notice how clean the rendering is now!
                DrawGameplay(&gameState, vWidth, vHeight);
            }

            if (currentScreen == SCREEN_PAUSE) DrawPauseMenu(&menu, vWidth, vHeight);
            if (currentScreen == SCREEN_SAVE_GAME) DrawSaveMenu(&menu, vWidth, vHeight);

            DrawCircleV(vMouse, 5, RED);
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexturePro(
                target.texture,
                (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height },
                (Rectangle){
                    (GetScreenWidth() - (vWidth * scale)) * 0.5f,
                    (GetScreenHeight() - (vHeight * scale)) * 0.5f,
                    vWidth * scale,
                    vHeight * scale
                },
                (Vector2){ 0, 0 }, 0.0f, WHITE
            );
        EndDrawing();
    }

cleanup:
    UnloadStoryScene(&scene1); UnloadSceneData(&scene1_data);
    UnloadStoryScene(&scene2); UnloadSceneData(&scene2_data);
    UnloadStoryScene(&scene3); UnloadSceneData(&scene3_data);
    UnloadStoryScene(&scene4); UnloadSceneData(&scene4_data);
    UnloadStoryScene(&scene5); UnloadSceneData(&scene5_data);
    UnloadStoryScene(&scene6); UnloadSceneData(&scene6_data);
    UnloadStoryScene(&scene7); UnloadSceneData(&scene7_data);
    UnloadStoryScene(&scene8); UnloadSceneData(&scene8_data);
    UnloadStoryScene(&scene9); UnloadSceneData(&scene9_data);
    UnloadStoryScene(&scene10); UnloadSceneData(&scene10_data);
    UnloadStoryScene(&scene11); UnloadSceneData(&scene11_data);

    // Unloading everything straight from the single struct
    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) UnloadEnemy(&gameState.regularBandits[i]);
    UnloadEnemy(&gameState.bossBandit);
    UnloadPlayer(&gameState.player);
    UnloadTilemap(&gameState.map);

    UnloadTexture(gameState.key.texture);
    UnloadTexture(gameState.heartTexture);
    UnloadTexture(gameState.speedTexture);
    
    UnloadTexture(menu.background);
    UnloadTexture(menu.settingsBg);
    UnloadTexture(menu.saveBg);
    UnloadRenderTexture(target);

    UnloadMusicStream(menuMusic);
    UnloadMusicStream(storyMusic);
    UnloadMusicStream(inGameMusic);

    UnloadEnemyAttackSfx();
    UnloadSound(pressButtonSfx);
    UnloadSound(loseSfx);
    UnloadSound(winSfx);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}