#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "tilemap.h"
#include "menu.h"
#include "character.h"
#include "story_scene.h"

// INCLUDE BATTLE ENGINE
#include "battle.h"

#define ENEMY_IMPLEMENTATION
#include "enemy.h"

#define BANDIT_IMPLEMENTATION
#include "bandit.h"

#define BOSS_BANDIT_IMPLEMENTATION
#include "boss_bandit.h"

#define SECURITY_GUARD_IMPLEMENTATION
#include "security_guard.h"

#define OASIS_MEDIA_CEO_IMPLEMENTATION
#include "oasis_media_ceo.h"

#define ENEMY_MANAGER_IMPLEMENTATION
#include "enemy_manager.h"

#define PICKUP_IMPLEMENTATION
#include "pickup.h"

#define OBJECTIVE_IMPLEMENTATION
#include "objective.h"

#define GAMEPLAY_IMPLEMENTATION
#include "gameplay.h"

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
    SCREEN_SCENE11,
    SCREEN_GAMEPLAY2,
    SCREEN_SCENE12,
    SCREEN_SCENE13_1,
    SCREEN_SCENE14_1,
    SCREEN_SCENE13_2,
    SCREEN_SCENE14_2,
    SCREEN_SCENE15_2,
    SCREEN_SCENE16_2,
    SCREEN_SCENE17_2,
    SCREEN_SCENE18_2_1,
    SCREEN_SCENE19_2_1,
    SCREEN_SCENE20_2_1,
    SCREEN_SCENE21_2_1,
    SCREEN_SCENE18_2_2,
    SCREEN_TEKKEN_FIGHT
} GameScreen;

#define DEFAULT_PORTRAIT_SCALE 0.28f
#define DEFAULT_PORTRAIT_OFFSET_X 20.0f

#define PORTRAIT_DEFAULT(name, path) \
    { name, path, DEFAULT_PORTRAIT_SCALE, DEFAULT_PORTRAIT_OFFSET_X }

#define PORTRAIT_CUSTOM(name, path, scaleValue, offsetXValue) \
    { name, path, scaleValue, offsetXValue }

static const GameplayConfig GAMEPLAY1_CONFIG = {
    .useSecurityTheme = false,
    .avoidEnemyText = "BANDITS",
    .regularSpawnText = "Bandits arrive in 5 seconds...",
    .bossSpawnText = "Boss arrives in 7 seconds..."
};

static const GameplayConfig GAMEPLAY2_CONFIG = {
    .useSecurityTheme = true,
    .avoidEnemyText = "SECURITY GUARDS",
    .regularSpawnText = "Security guards arrive in 5 seconds...",
    .bossSpawnText = "Oasis Media CEO arrives in 7 seconds..."
};

// --- SCENE DATA DEFINITIONS ---
SceneData scene1_data = { .bgPath = "images/Background/Scene/Scene1.png", .bgScrollSpeed = 0.0f, .doFadeIn = false, .doFadeOut = false, .portraitCount = 2, .portraits = { PORTRAIT_DEFAULT("Commander", "images/Character/Commander/CommanderChat.png"), PORTRAIT_DEFAULT("Reuben", "images/Character/Reuben/ReubenChat.png") }, .choiceText = "Yes Commander!", .narratorText = "Before the world called you a hero,\nyou were just a child who believed in order.", .narratorVoicePath = "audio/Voice/Scene 1/Narrator.mp3", };
SceneData scene2_data = { .bgPath = "images/Background/Scene/Scene2.jpg", .bgScrollSpeed = -15.0f, .doFadeIn = true, .doFadeOut = true, };
SceneData scene3_data = { .bgPath = "images/Background/Scene/Scene3.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = false, .doFadeOut = false, .narratorText = "And that idea stayed with you...", .narratorVoicePath = "audio/Voice/Scene 3/Narrator part 2.mp3", };
SceneData scene4_data = { .bgPath = "images/Background/Scene/Scene4.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, };
SceneData scene5_data = { .bgPath = "images/Background/Scene/Scene5.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, };
SceneData scene6_data = { .bgPath = "images/Background/Scene/Scene6.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .narratorText = "At least, that's what you were told.", .narratorVoicePath = "audio/Voice/Scene 6/Narrator part 2.mp3" };
SceneData scene7_data = { .bgPath = "images/Background/Scene/Scene7.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .portraitCount = 2, .portraits = { PORTRAIT_DEFAULT("Reuben", "images/Character/Reuben/ReubenChat.png"), PORTRAIT_DEFAULT("Boss Bandit", "images/Character/Boss Bandit/BossBanditChat.png") }, };
SceneData scene8_data = { .bgPath = "images/Background/Scene/Scene8.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .portraitCount = 1, .portraits = { PORTRAIT_DEFAULT("Soldier", "images/Character/Soldier/SoldierChat.png") } };
SceneData scene9_data = { .bgPath = "images/Background/Scene/Scene7.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .portraitCount = 2, .portraits = { PORTRAIT_DEFAULT("Reuben", "images/Character/Reuben/ReubenChat.png"), PORTRAIT_DEFAULT("Commander", "images/Character/Commander/CommanderChat.png") } };
SceneData scene10_data = { .bgPath = "images/Background/Scene/Scene10.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .portraitCount = 1, .portraits = { PORTRAIT_DEFAULT("Commander", "images/Character/Commander/CommanderChat.png") }, .narratorText = "(Next day)\n*Loud knocking sound*", };
SceneData scene11_data = { .bgPath = "images/Background/Scene/Scene11.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .portraitCount = 2, .portraits = { PORTRAIT_DEFAULT("Reuben", "images/Character/Reuben/ReubenChat.png"), PORTRAIT_DEFAULT("Commander", "images/Character/Commander/CommanderChat.png") }, .choiceText = "I'll try my best!", .endPromptText = "PRESS ENTER TO CONTINUE" };
SceneData scene12_data = { .bgPath = "images/Background/Scene/Scene12.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .portraitCount = 2, .portraits = { PORTRAIT_DEFAULT("Reuben", "images/Character/Reuben/ReubenChat.png"), PORTRAIT_DEFAULT("Oasis CEO", "images/Character/Oasis Media CEO/OasisCEOChat.png") }, .choiceText = "Open document", .choiceText2 = "Close document", .endPromptText = "CLICK A BUTTON TO CONTINUE" };
SceneData scene13_1_data = { .bgPath = "images/Background/Scene/Scene13_1.jpg" };
SceneData scene14_1_data = { .bgPath = "images/Background/Scene/Scene14_1.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .portraitCount = 2, .portraits = { PORTRAIT_DEFAULT("Reuben", "images/Character/Reuben/ReubenChat.png"), PORTRAIT_DEFAULT("Oasis CEO", "images/Character/Oasis Media CEO/OasisCEOChat.png") } };
SceneData scene13_2_data = { .bgPath = "images/Background/Scene/Scene1.png", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .portraitCount = 2, .portraits = { PORTRAIT_DEFAULT("Reuben", "images/Character/Reuben/ReubenChat.png"), PORTRAIT_DEFAULT("Commander", "images/Character/Commander/CommanderChat.png") } };
SceneData scene14_2_data = { .bgPath = "images/Background/Scene/Scene14_2.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .portraitCount = 2, .portraits = { PORTRAIT_DEFAULT("Reuben", "images/Character/Reuben/ReubenChat.png"), PORTRAIT_DEFAULT("President", "images/Character/President/PresidentChat.png") } };
SceneData scene15_2_data = { .bgPath = "images/Background/Scene/Scene15_2.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = false, };
SceneData scene16_2_data = { .bgPath = "images/Background/Scene/Scene15_2.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = false, .doFadeOut = true, .portraitCount = 2, .portraits = { PORTRAIT_DEFAULT("Reuben", "images/Character/Reuben/ReubenChat.png"), PORTRAIT_DEFAULT("Ashat Leader", "images/Character/Ashat Leader/AshatLeaderChat.png") } };
SceneData scene17_2_data = { .bgPath = "images/Background/Scene/Scene17_2.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .portraitCount = 2, .portraits = { PORTRAIT_DEFAULT("Reuben", "images/Character/Reuben/ReubenChat.png"), PORTRAIT_DEFAULT("Ashat Leader", "images/Character/Ashat Leader/AshatLeaderChat.png") }, .choiceText = "Trust him", .choiceText2 = "Capture him" };
SceneData scene18_2_1_data = { .bgPath = "images/Background/Scene/Scene18_2_1.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, };
SceneData scene19_2_1_data = { .bgPath = "images/Background/Scene/Scene1.png", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, .portraitCount = 2, .portraits = { PORTRAIT_DEFAULT("Reuben", "images/Character/Reuben/ReubenChat.png"), PORTRAIT_DEFAULT("Commander", "images/Character/Commander/CommanderChat.png") } };
SceneData scene20_2_1_data = { .bgPath = "images/Background/Scene/Scene20_2_1.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, };
SceneData scene21_2_1_data = { .bgPath = "images/Background/Scene/Scene21_2_1.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, };
SceneData scene18_2_2_data = { .bgPath = "images/Background/Scene/Scene18_2_2.jpg", .bgScrollSpeed = 0.0f, .doFadeIn = true, .doFadeOut = true, };

static bool IsGameplayScreen(GameScreen screen) {
    return screen == SCREEN_GAMEPLAY || screen == SCREEN_GAMEPLAY2 || screen == SCREEN_TEKKEN_FIGHT;
}

static bool IsStorySceneScreen(GameScreen screen) {
    switch (screen) {
        case SCREEN_SCENE1: case SCREEN_SCENE2: case SCREEN_SCENE3: case SCREEN_SCENE4:
        case SCREEN_SCENE5: case SCREEN_SCENE6: case SCREEN_SCENE7: case SCREEN_SCENE8:
        case SCREEN_SCENE9: case SCREEN_SCENE10: case SCREEN_SCENE11: case SCREEN_SCENE12:
        case SCREEN_SCENE13_1: case SCREEN_SCENE14_1: case SCREEN_SCENE13_2: case SCREEN_SCENE14_2:
        case SCREEN_SCENE15_2: case SCREEN_SCENE16_2: case SCREEN_SCENE17_2: case SCREEN_SCENE18_2_1:
        case SCREEN_SCENE19_2_1: case SCREEN_SCENE20_2_1: case SCREEN_SCENE21_2_1: case SCREEN_SCENE18_2_2:
            return true;
        default:
            return false;
    }
}

static GameplayState *GetGameplayStateForScreen(GameScreen screen, GameplayState *gameState1, GameplayState *gameState2) {
    return (screen == SCREEN_GAMEPLAY2) ? gameState2 : gameState1;
}

static void SetupGameplayState(GameplayState *state, const GameplayConfig *config, int vWidth, int vHeight) {
    state->config = config;
    state->camera.offset = (Vector2){ vWidth / 2.0f, vHeight / 2.0f };
    state->camera.rotation = 0.0f;
    state->camera.zoom = 3.0f;
    ResetGameplay(state);
}

static void ResetAllScenes(
    StoryScene* s1, SceneData* sd1, StoryScene* s2, SceneData* sd2,
    StoryScene* s3, SceneData* sd3, StoryScene* s4, SceneData* sd4,
    StoryScene* s5, SceneData* sd5, StoryScene* s6, SceneData* sd6,
    StoryScene* s7, SceneData* sd7, StoryScene* s8, SceneData* sd8,
    StoryScene* s9, SceneData* sd9, StoryScene* s10, SceneData* sd10,
    StoryScene* s11, SceneData* sd11, StoryScene* s12, SceneData* sd12,
    StoryScene* s13_1, SceneData* sd13_1, StoryScene* s14_1, SceneData* sd14_1,
    StoryScene* s13_2, SceneData* sd13_2, StoryScene* s14_2, SceneData* sd14_2,
    StoryScene* s15_2, SceneData* sd15_2, StoryScene* s16_2, SceneData* sd16_2,
    StoryScene* s17_2, SceneData* sd17_2, StoryScene* s18_2_1, SceneData* sd18_2_1,
    StoryScene* s19_2_1, SceneData* sd19_2_1, StoryScene* s20_2_1, SceneData* sd20_2_1,
    StoryScene* s21_2_1, SceneData* sd21_2_1, StoryScene* s18_2_2, SceneData* sd18_2_2)
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
    InitStoryScene(s12, sd12);
    InitStoryScene(s13_1, sd13_1);
    InitStoryScene(s14_1, sd14_1);
    InitStoryScene(s13_2, sd13_2);
    InitStoryScene(s14_2, sd14_2);
    InitStoryScene(s15_2, sd15_2);
    InitStoryScene(s16_2, sd16_2);
    InitStoryScene(s17_2, sd17_2);
    InitStoryScene(s18_2_1, sd18_2_1);
    InitStoryScene(s19_2_1, sd19_2_1);
    InitStoryScene(s20_2_1, sd20_2_1);
    InitStoryScene(s21_2_1, sd21_2_1);
    InitStoryScene(s18_2_2, sd18_2_2);
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

    GameplayState gameState1 = { 0 };
    GameplayState gameState2 = { 0 };
    
    // --- TEKKEN MINIGAME VARIABLES ---
    Character playerTekken;
    Character enemyTekken;
    int currentTekkenFight = 1; // 1 = After 16_2, 2 = After 19_2_1
    float tekkenTimer = 99.0f;
    float tekkenEndDelay = 0.0f;
    bool showTekkenControls = false;
    bool tekkenLoseSfxPlayed = false;
    bool tekkenWinSfxPlayed = false;
    bool tekkenShowResultOverlay = false;
    bool tekkenFinalWin = false;
    Tilemap tekkenMap = { 0 };
    Texture2D tekkenBg = { 0 };
    Texture2D tekkenBg2 = { 0 };
    // ---------------------------------

    StoryScene scene1 = { 0 }; StoryScene scene2 = { 0 }; StoryScene scene3 = { 0 };
    StoryScene scene4 = { 0 }; StoryScene scene5 = { 0 }; StoryScene scene6 = { 0 };
    StoryScene scene7 = { 0 }; StoryScene scene8 = { 0 }; StoryScene scene9 = { 0 };
    StoryScene scene10 = { 0 }; StoryScene scene11 = { 0 }; StoryScene scene12 = { 0 };
    StoryScene scene13_1 = { 0 }; StoryScene scene14_1 = { 0 }; StoryScene scene13_2 = { 0 };
    StoryScene scene14_2 = { 0 }; StoryScene scene15_2 = { 0 }; StoryScene scene16_2 = { 0 };
    StoryScene scene17_2 = { 0 }; StoryScene scene18_2_1 = { 0 }; StoryScene scene19_2_1 = { 0 };
    StoryScene scene20_2_1 = { 0 }; StoryScene scene21_2_1 = { 0 }; StoryScene scene18_2_2 = { 0 };

    RenderTexture2D target = LoadRenderTexture(vWidth, vHeight);

    Music menuMusic = { 0 };
    Music storyMusic = { 0 };
    Music inGameMusic = { 0 };
    Music duelMusic = { 0 };
    Music *activeMusic = NULL;

    Sound pressButtonSfx = { 0 };
    Sound loseSfx = { 0 };
    Sound winSfx = { 0 };
    Sound battleSfx = { 0 };
    Sound clappingSfx = { 0 };
    Sound heartPickSfx = { 0 };
    Sound speedSfx = { 0 };

    bool fadeOutMusic = false;
    float musicVolume = 1.0f;
    GameScreen nextScreenAfterFade = SCREEN_MENU;
    bool sfxLoaded = false;
    GameScreen previousScreen = SCREEN_LOADING;

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

        UpdateBattleSfxQueue();

        if (currentScreen != previousScreen) {
            StopSound(battleSfx);
            StopSound(clappingSfx);

            if (currentScreen == SCREEN_SCENE15_2 || currentScreen == SCREEN_SCENE18_2_1) {
                PlaySound(battleSfx);
            }
            else if (currentScreen == SCREEN_SCENE21_2_1 || currentScreen == SCREEN_SCENE18_2_2) {
                PlaySound(clappingSfx);
            }

            // Switch background music when entering screens directly without fade.
            Music *wantedMusic = activeMusic;
            if (currentScreen == SCREEN_TEKKEN_FIGHT) {
                wantedMusic = &duelMusic;
            } else if (currentScreen == SCREEN_GAMEPLAY || currentScreen == SCREEN_GAMEPLAY2) {
                wantedMusic = &inGameMusic;
            } else if (IsStorySceneScreen(currentScreen)) {
                wantedMusic = &storyMusic;
            } else if (currentScreen == SCREEN_MENU) {
                wantedMusic = &menuMusic;
            }

            if (!fadeOutMusic && wantedMusic != NULL && wantedMusic != activeMusic) {
                if (activeMusic != NULL) {
                    StopMusicStream(*activeMusic);
                }
                activeMusic = wantedMusic;
                SetMusicVolume(*activeMusic, musicVolume);
                PlayMusicStream(*activeMusic);
            }

            previousScreen = currentScreen;
        }

        if (fadeOutMusic && activeMusic != NULL) {
            musicVolume -= GetFrameTime();

            if (musicVolume <= 0.0f) {
                musicVolume = 0.0f;
                StopMusicStream(*activeMusic);
                fadeOutMusic = false;
                currentScreen = nextScreenAfterFade;

                if (IsStorySceneScreen(currentScreen)) {
                    activeMusic = &storyMusic;
                } else if (currentScreen == SCREEN_TEKKEN_FIGHT) {
                    activeMusic = &duelMusic;
                } else if (currentScreen == SCREEN_GAMEPLAY || currentScreen == SCREEN_GAMEPLAY2) {
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
                    
                    tekkenBg = LoadTexture("images/Background/Arena/arena1/arena1.png"); 
                    tekkenBg2 = LoadTexture("images/Background/Arena/arena2/arena2.png"); 
                    
                    menuMusic = LoadMusicStream("music/MainMenu.ogg");
                    storyMusic = LoadMusicStream("music/Story.ogg");
                    inGameMusic = LoadMusicStream("music/ingame.ogg");
                    duelMusic = LoadMusicStream("music/duel.mp3");
                    pressButtonSfx = LoadSound("audio/Sfx/press_button.mp3");
                    loseSfx = LoadSound("audio/Sfx/lose.mp3");
                    winSfx = LoadSound("audio/Sfx/find_objective.mp3");
                    battleSfx = LoadSound("audio/Sfx/battle.mp3");
                    clappingSfx = LoadSound("audio/Sfx/clapping.mp3");
                    heartPickSfx = LoadSound("audio/Sfx/heartpick.mp3");
                    speedSfx = LoadSound("audio/Sfx/speedpick.mp3");
                    LoadBattleSfx();
                    sfxLoaded = true;
                    loadProgress = 0.06f;
                    loadStep++;
                    break;

                case 1:
                    gameState1.config = &GAMEPLAY1_CONFIG;
                    gameState2.config = &GAMEPLAY2_CONFIG;

                    gameState1.key.texture = LoadTexture("images/Elements/key.png");
                    gameState1.heartTexture = LoadTexture("images/Elements/heart.png");
                    gameState1.speedTexture = LoadTexture("images/Elements/speed.png");

                    gameState2.key.texture = LoadTexture("images/Elements/key.png");
                    gameState2.heartTexture = LoadTexture("images/Elements/heart.png");
                    gameState2.speedTexture = LoadTexture("images/Elements/speed.png");

                    if (!LoadTilemap(&gameState1.map, "maps/map1/map1.json")) goto cleanup;
                    if (!LoadTilemap(&gameState2.map, "maps/map2/map2.json")) goto cleanup;
                    
                    LoadTilemap(&tekkenMap, "images/Background/Arena/arena1/arena1.json"); 
                    
                    // PRELOAD TEKKEN CHARACTERS (Default for Fight 1)
                    InitCharacter(&playerTekken, (Vector2){ 250, 100 }, 0);
                    InitCharacter(&enemyTekken, (Vector2){ 850, 100 }, 1); // Samurai

                    loadProgress = 0.16f;
                    loadStep++;
                    break;

                case 2:
                    SetupGameplayState(&gameState1, &GAMEPLAY1_CONFIG, vWidth, vHeight);
                    SetupGameplayState(&gameState2, &GAMEPLAY2_CONFIG, vWidth, vHeight);
                    loadProgress = 0.24f;
                    loadStep++;
                    break;

                case 3: LoadSceneDialogue("data/scene1.txt", &scene1_data); InitStoryScene(&scene1, &scene1_data); loadProgress = 0.30f; loadStep++; break;
                case 4: LoadSceneDialogue("data/scene2.txt", &scene2_data); InitStoryScene(&scene2, &scene2_data); loadProgress = 0.34f; loadStep++; break;
                case 5: LoadSceneDialogue("data/scene3.txt", &scene3_data); InitStoryScene(&scene3, &scene3_data); loadProgress = 0.38f; loadStep++; break;
                case 6: LoadSceneDialogue("data/scene4.txt", &scene4_data); InitStoryScene(&scene4, &scene4_data); loadProgress = 0.42f; loadStep++; break;
                case 7: LoadSceneDialogue("data/scene5.txt", &scene5_data); InitStoryScene(&scene5, &scene5_data); loadProgress = 0.46f; loadStep++; break;
                case 8: LoadSceneDialogue("data/scene6.txt", &scene6_data); InitStoryScene(&scene6, &scene6_data); loadProgress = 0.50f; loadStep++; break;
                case 9: LoadSceneDialogue("data/scene7.txt", &scene7_data); InitStoryScene(&scene7, &scene7_data); loadProgress = 0.54f; loadStep++; break;
                case 10: LoadSceneDialogue("data/scene8.txt", &scene8_data); InitStoryScene(&scene8, &scene8_data); loadProgress = 0.58f; loadStep++; break;
                case 11: LoadSceneDialogue("data/scene9.txt", &scene9_data); InitStoryScene(&scene9, &scene9_data); loadProgress = 0.62f; loadStep++; break;
                case 12: LoadSceneDialogue("data/scene10.txt", &scene10_data); InitStoryScene(&scene10, &scene10_data); loadProgress = 0.66f; loadStep++; break;
                case 13: LoadSceneDialogue("data/scene11.txt", &scene11_data); InitStoryScene(&scene11, &scene11_data); loadProgress = 0.70f; loadStep++; break;
                case 14: LoadSceneDialogue("data/scene12.txt", &scene12_data); InitStoryScene(&scene12, &scene12_data); loadProgress = 0.75f; loadStep++; break;
                case 15: InitStoryScene(&scene13_1, &scene13_1_data); loadProgress = 0.80f; loadStep++; break;
                case 16: LoadSceneDialogue("data/scene14_1.txt", &scene14_1_data); InitStoryScene(&scene14_1, &scene14_1_data); loadProgress = 0.84f; loadStep++; break;
                case 17: LoadSceneDialogue("data/scene13_2.txt", &scene13_2_data); InitStoryScene(&scene13_2, &scene13_2_data); loadProgress = 0.88f; loadStep++; break;
                case 18: LoadSceneDialogue("data/scene14_2.txt", &scene14_2_data); InitStoryScene(&scene14_2, &scene14_2_data); loadProgress = 0.92f; loadStep++; break;
                case 19: LoadSceneDialogue("data/scene15_2.txt", &scene15_2_data); InitStoryScene(&scene15_2, &scene15_2_data); loadProgress = 0.88f; loadStep++; break;
                case 20: LoadSceneDialogue("data/scene16_2.txt", &scene16_2_data); InitStoryScene(&scene16_2, &scene16_2_data); loadProgress = 0.90f; loadStep++; break;
                case 21: LoadSceneDialogue("data/scene17_2.txt", &scene17_2_data); InitStoryScene(&scene17_2, &scene17_2_data); loadProgress = 0.92f; loadStep++; break;
                case 22: LoadSceneDialogue("data/scene18_2_1.txt", &scene18_2_1_data); InitStoryScene(&scene18_2_1, &scene18_2_1_data); loadProgress = 0.94f; loadStep++; break;
                case 23: LoadSceneDialogue("data/scene19_2_1.txt", &scene19_2_1_data); InitStoryScene(&scene19_2_1, &scene19_2_1_data); loadProgress = 0.96f; loadStep++; break;
                case 24: LoadSceneDialogue("data/scene20_2_1.txt", &scene20_2_1_data); InitStoryScene(&scene20_2_1, &scene20_2_1_data); loadProgress = 0.98f; loadStep++; break;
                case 25: LoadSceneDialogue("data/scene21_2_1.txt", &scene21_2_1_data); InitStoryScene(&scene21_2_1, &scene21_2_1_data); loadProgress = 0.99f; loadStep++; break;
                case 26: LoadSceneDialogue("data/scene18_2_2.txt", &scene18_2_2_data); InitStoryScene(&scene18_2_2, &scene18_2_2_data); loadProgress = 1.0f; loadStep++; break;

                case 27:
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
                    &scene10, &scene10_data, &scene11, &scene11_data, &scene12, &scene12_data,
                    &scene13_1, &scene13_1_data, &scene14_1, &scene14_1_data,
                    &scene13_2, &scene13_2_data, &scene14_2, &scene14_2_data,
                    &scene15_2, &scene15_2_data, &scene16_2, &scene16_2_data,
                    &scene17_2, &scene17_2_data, &scene18_2_1, &scene18_2_1_data,
                    &scene19_2_1, &scene19_2_1_data, &scene20_2_1, &scene20_2_1_data,
                    &scene21_2_1, &scene21_2_1_data, &scene18_2_2, &scene18_2_2_data
                );
                fadeOutMusic = true;
                nextScreenAfterFade = SCREEN_SCENE1;
            }
            if (action == 2) {
                RefreshSaveSlots(&menu);
                currentScreen = SCREEN_LOAD_GAME;
                menu.subSelected = 0;
                menu.activeLoadSlot = -1;
            }
            if (action == 3) {
                pausedFromScreen = SCREEN_MENU;
                currentScreen = SCREEN_SETTINGS;
                menu.subSelected = 0;
            }
            if (action == 4) {
                break;
            }
        }
        else if (currentScreen == SCREEN_LOAD_GAME) {
            int action = UpdateLoadMenu(&menu, vMouse, vWidth, vHeight);
            if (action == 1) currentScreen = SCREEN_MENU;
            if (action >= 10 && action <= 13) {
                int slot = action - 10;
                GameSaveData data;
                if (LoadGameData(slot, &data)) {
                    GameplayState *targetState = &gameState1;
                    if ((GameScreen)data.savedScreen == SCREEN_GAMEPLAY2) {
                        targetState = &gameState2;
                    }

                    ResetGameplay(targetState);
                    targetState->player.pos = data.playerPos;
                    targetState->player.health = data.health;
                    targetState->player.energy = data.energy;
                    targetState->camera.target = targetState->player.pos;
                    targetState->showInstructions = false;

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
        else if (IsStorySceneScreen(currentScreen)) {
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
                    if (scene6.currentState == SCENE_STATE_DONE) { ResetGameplay(&gameState1); fadeOutMusic = true; nextScreenAfterFade = SCREEN_GAMEPLAY; }
                }
                else if (currentScreen == SCREEN_SCENE7) { UpdateStoryScene(&scene7, vMouse, mouseClicked, vWidth); if (scene7.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE8; }
                else if (currentScreen == SCREEN_SCENE8) { UpdateStoryScene(&scene8, vMouse, mouseClicked, vWidth); if (scene8.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE9; }
                else if (currentScreen == SCREEN_SCENE9) { UpdateStoryScene(&scene9, vMouse, mouseClicked, vWidth); if (scene9.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE10; }
                else if (currentScreen == SCREEN_SCENE10) { UpdateStoryScene(&scene10, vMouse, mouseClicked, vWidth); if (scene10.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE11; }
                else if (currentScreen == SCREEN_SCENE11) { 
                    UpdateStoryScene(&scene11, vMouse, mouseClicked, vWidth); 
                    if (scene11.currentState == SCENE_STATE_DONE) { ResetGameplay(&gameState2); fadeOutMusic = true; nextScreenAfterFade = SCREEN_GAMEPLAY2; }
                }
                else if (currentScreen == SCREEN_SCENE12) {
                    UpdateStoryScene(&scene12, vMouse, mouseClicked, vWidth);
                    if (scene12.currentState == SCENE_STATE_DONE) {
                        if (scene12.selectedChoice == 1) currentScreen = SCREEN_SCENE13_1;
                        else currentScreen = SCREEN_SCENE13_2;
                    }
                }
                else if (currentScreen == SCREEN_SCENE13_1) { if (mouseClicked || enterPressed) { currentScreen = SCREEN_SCENE14_1; } }
                else if (currentScreen == SCREEN_SCENE14_1) { UpdateStoryScene(&scene14_1, vMouse, mouseClicked, vWidth); if (scene14_1.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE18_2_1; }
                else if (currentScreen == SCREEN_SCENE13_2) { UpdateStoryScene(&scene13_2, vMouse, mouseClicked, vWidth); if (scene13_2.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE14_2; }
                else if (currentScreen == SCREEN_SCENE14_2) { UpdateStoryScene(&scene14_2, vMouse, mouseClicked, vWidth); if (scene14_2.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE15_2; }
                else if (currentScreen == SCREEN_SCENE15_2) { UpdateStoryScene(&scene15_2, vMouse, mouseClicked, vWidth); if (scene15_2.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE16_2; }
                
                // --------- LAUNCH TEKKEN FIGHT 1 AFTER SCENE 16_2 ---------
                else if (currentScreen == SCREEN_SCENE16_2) {
                    UpdateStoryScene(&scene16_2, vMouse, mouseClicked, vWidth);
                    if (scene16_2.currentState == SCENE_STATE_DONE) {
                        currentTekkenFight = 1;
                        InitCharacter(&playerTekken, (Vector2){ 250, 100 }, 0); // Player
                        InitCharacter(&enemyTekken, (Vector2){ 850, 100 }, 1); // Samurai
                        tekkenTimer = 99.0f;
                        tekkenEndDelay = 0.0f;
                        showTekkenControls = true;
                        tekkenLoseSfxPlayed = false;
                        tekkenWinSfxPlayed = false;
                        tekkenShowResultOverlay = false;
                        tekkenFinalWin = false;
                        ResetBattleSfxQueue();
                        currentScreen = SCREEN_TEKKEN_FIGHT; 
                    }
                }
                // -----------------------------------------------------------
                
                else if (currentScreen == SCREEN_SCENE17_2) {
                    UpdateStoryScene(&scene17_2, vMouse, mouseClicked, vWidth);
                    if (scene17_2.currentState == SCENE_STATE_DONE) {
                        if (scene17_2.selectedChoice == 1) currentScreen = SCREEN_SCENE18_2_1;
                        else currentScreen = SCREEN_SCENE18_2_2;
                    }
                }
                
                // --- NEW: Scene 18_2_1 just proceeds normally to 19_2_1 ---
                else if (currentScreen == SCREEN_SCENE18_2_1) { 
                    UpdateStoryScene(&scene18_2_1, vMouse, mouseClicked, vWidth); 
                    if (scene18_2_1.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE19_2_1; 
                }
                
                // --------- LAUNCH TEKKEN FIGHT 2 AFTER SCENE 19_2_1 ---------
                else if (currentScreen == SCREEN_SCENE19_2_1) {
                    UpdateStoryScene(&scene19_2_1, vMouse, mouseClicked, vWidth);
                    if (scene19_2_1.currentState == SCENE_STATE_DONE) {
                        currentTekkenFight = 2;
                        InitCharacter(&playerTekken, (Vector2){ 250, 100 }, 0); // Player
                        InitCharacter(&enemyTekken, (Vector2){ 850, 100 }, 2); // Viking
                        tekkenTimer = 99.0f;
                        tekkenEndDelay = 0.0f;
                        showTekkenControls = true;
                        tekkenLoseSfxPlayed = false;
                        tekkenWinSfxPlayed = false;
                        tekkenShowResultOverlay = false;
                        tekkenFinalWin = false;
                        ResetBattleSfxQueue();
                        currentScreen = SCREEN_TEKKEN_FIGHT; 
                    }
                }
                // -----------------------------------------------------------

                else if (currentScreen == SCREEN_SCENE20_2_1) { UpdateStoryScene(&scene20_2_1, vMouse, mouseClicked, vWidth); if (scene20_2_1.currentState == SCENE_STATE_DONE) currentScreen = SCREEN_SCENE21_2_1; }
                else if (currentScreen == SCREEN_SCENE21_2_1) {
                    UpdateStoryScene(&scene21_2_1, vMouse, mouseClicked, vWidth);
                    if (scene21_2_1.currentState == SCENE_STATE_DONE) { fadeOutMusic = true; nextScreenAfterFade = SCREEN_MENU; }
                }
                else if (currentScreen == SCREEN_SCENE18_2_2) {
                    UpdateStoryScene(&scene18_2_2, vMouse, mouseClicked, vWidth);
                    if (scene18_2_2.currentState == SCENE_STATE_DONE) { fadeOutMusic = true; nextScreenAfterFade = SCREEN_MENU; }
                }
            }
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            bool requestPause = false;
            bool requestNextScene = false;
            UpdateGameplay(&gameState1, menu.keys, loseSfx, winSfx, heartPickSfx, speedSfx, inGameMusic, &requestPause, &requestNextScene, mouseClicked);
            if (requestPause) { pausedFromScreen = currentScreen; currentScreen = SCREEN_PAUSE; menu.subSelected = 0; }
            if (requestNextScene) { fadeOutMusic = true; nextScreenAfterFade = SCREEN_SCENE7; }
        }
        else if (currentScreen == SCREEN_GAMEPLAY2) {
            bool requestPause = false;
            bool requestNextScene = false;
            UpdateGameplay(&gameState2, menu.keys, loseSfx, winSfx, heartPickSfx, speedSfx, inGameMusic, &requestPause, &requestNextScene, mouseClicked);
            if (requestPause) { pausedFromScreen = currentScreen; currentScreen = SCREEN_PAUSE; menu.subSelected = 0; }
            if (requestNextScene) { fadeOutMusic = true; nextScreenAfterFade = SCREEN_SCENE12; }
        }
        
        // --------- TEKKEN UPDATE LOOP ---------
        else if (currentScreen == SCREEN_TEKKEN_FIGHT) {
            if (showTekkenControls) {
                // Check if OK button is clicked (updated bounds for the bigger UI)
                Rectangle okButton = { (vWidth / 2.0f) - 60, (vHeight / 2.0f) + 110, 120, 50 };
                if (CheckCollisionPointRec(vMouse, okButton) && mouseClicked) {
                    showTekkenControls = false; // Dismiss pop-up and start fight
                }
            } else {
                bool tekkenTimeOver = tekkenTimer <= 0.0f;
                bool tekkenPlayerWon = (enemyTekken.health <= 0) ||
                    (tekkenTimeOver && playerTekken.health > enemyTekken.health);
                bool tekkenPlayerLost = (playerTekken.health <= 0) ||
                    (tekkenTimeOver && playerTekken.health <= enemyTekken.health);
                bool tekkenFightOver = tekkenPlayerWon || tekkenPlayerLost;

                if (tekkenShowResultOverlay && tekkenPlayerLost && IsKeyPressed(KEY_R)) {
                    int enemyType = (currentTekkenFight == 1) ? 1 : 2;
                    InitCharacter(&playerTekken, (Vector2){ 250, 100 }, 0);
                    InitCharacter(&enemyTekken, (Vector2){ 850, 100 }, enemyType);
                    tekkenTimer = 99.0f;
                    tekkenEndDelay = 0.0f;
                    tekkenLoseSfxPlayed = false;
                    tekkenWinSfxPlayed = false;
                    tekkenShowResultOverlay = false;
                    tekkenFinalWin = false;
                    showTekkenControls = true;
                    ResetBattleSfxQueue();
                    StopSound(loseSfx);
                    StopSound(winSfx);
                    activeMusic = &duelMusic;
                    SetMusicVolume(*activeMusic, musicVolume);
                    PlayMusicStream(*activeMusic);
                } else if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) {
                    pausedFromScreen = currentScreen;
                    currentScreen = SCREEN_PAUSE;
                    menu.subSelected = 0;
                } else if (!tekkenFightOver) {
                    float dt = GetFrameTime();
                    if (tekkenTimer > 0 && playerTekken.health > 0 && enemyTekken.health > 0) {
                        tekkenTimer -= dt;
                        if (tekkenTimer < 0.0f) tekkenTimer = 0.0f;
                    }

                    int enemyTekkenType = (currentTekkenFight == 1) ? 1 : 2;
                    UpdateCharacterWithSfx(&playerTekken, &enemyTekken, dt, true, 0, enemyTekkenType);
                    UpdateCharacterWithSfx(&enemyTekken, &playerTekken, dt, false, enemyTekkenType, 0);
                    ResolveFighterOverlap(&playerTekken, &enemyTekken);
                } else {
                    float dt = GetFrameTime();
                    bool endSequenceFinished = UpdateBattleEndSequence(&playerTekken, &enemyTekken, dt);

                    if (endSequenceFinished && !tekkenShowResultOverlay) {
                        tekkenFinalWin = tekkenPlayerWon;
                        tekkenShowResultOverlay = true;
                        tekkenEndDelay = 0.0f;

                        if (tekkenPlayerLost && !tekkenLoseSfxPlayed) {
                            tekkenLoseSfxPlayed = true;
                            tekkenWinSfxPlayed = false;
                            StopMusicStream(duelMusic);
                            PlaySound(loseSfx);
                        }

                        if (tekkenPlayerWon && !tekkenWinSfxPlayed) {
                            tekkenWinSfxPlayed = true;
                            tekkenLoseSfxPlayed = false;
                            PlaySound(winSfx);
                        }
                    }

                    if (tekkenShowResultOverlay && tekkenFinalWin) {
                        tekkenEndDelay += dt;
                        if (tekkenEndDelay > 3.0f && !fadeOutMusic) {
                            fadeOutMusic = true;
                            if (currentTekkenFight == 1) {
                                nextScreenAfterFade = SCREEN_SCENE17_2;
                            } else {
                                nextScreenAfterFade = SCREEN_SCENE20_2_1;
                            }
                        }
                    }
                }
            }
        }
        // ----------------------------------------
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

                GameplayState *stateToSave = &gameState1;
                if (IsGameplayScreen(pausedFromScreen)) {
                    stateToSave = GetGameplayStateForScreen(pausedFromScreen, &gameState1, &gameState2);
                }

                data.playerPos = stateToSave->player.pos;
                data.health = stateToSave->player.health;
                data.energy = stateToSave->player.energy;
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

            if (IsStorySceneScreen(bgScreen)) {
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
                else if (bgScreen == SCREEN_SCENE12) DrawStoryScene(&scene12, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE13_1) {
                    DrawTexturePro(scene13_1.bgTexture, (Rectangle){ 0, 0, (float)scene13_1.bgTexture.width, (float)scene13_1.bgTexture.height }, (Rectangle){ 0, 0, (float)vWidth, (float)vHeight }, (Vector2){ 0, 0 }, 0.0f, WHITE);
                }
                else if (bgScreen == SCREEN_SCENE14_1) DrawStoryScene(&scene14_1, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE13_2) DrawStoryScene(&scene13_2, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE14_2) DrawStoryScene(&scene14_2, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE15_2) DrawStoryScene(&scene15_2, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE16_2) DrawStoryScene(&scene16_2, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE17_2) DrawStoryScene(&scene17_2, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE18_2_1) DrawStoryScene(&scene18_2_1, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE19_2_1) DrawStoryScene(&scene19_2_1, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE20_2_1) DrawStoryScene(&scene20_2_1, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE21_2_1) DrawStoryScene(&scene21_2_1, vWidth, vHeight);
                else if (bgScreen == SCREEN_SCENE18_2_2) DrawStoryScene(&scene18_2_2, vWidth, vHeight);
            }
            else if (bgScreen == SCREEN_GAMEPLAY) {
                DrawGameplay(&gameState1, vWidth, vHeight);
            }
            else if (bgScreen == SCREEN_GAMEPLAY2) {
                DrawGameplay(&gameState2, vWidth, vHeight);
            }
            
            // --------- TEKKEN RENDER LOGIC ---------
            else if (bgScreen == SCREEN_TEKKEN_FIGHT) {
                ClearBackground(BLACK);
                
                // Draw the Arena Background scaled to fit the 1280x720 window
                Texture2D currentBg = (currentTekkenFight == 2) ? tekkenBg2 : tekkenBg;
                DrawTexturePro(
                    currentBg, 
                    (Rectangle){ 0, 0, currentBg.width, currentBg.height }, 
                    (Rectangle){ 0, 0, vWidth, vHeight }, 
                    (Vector2){ 0, 0 }, 
                    0.0f, 
                    WHITE
                );

                DrawCharacter(&playerTekken);
                DrawCharacter(&enemyTekken);
                DrawGameUI(playerTekken.health, enemyTekken.health, (int)tekkenTimer, vWidth, vHeight);
                
                // Draw Controls UI
                DrawText("A/D to Walk | L-SHIFT to Run | S to Shield | SPACE to Jump", 20, vHeight - 60, 20, RAYWHITE);
                DrawText("Left Click = Atk 1 | Right Click = Atk 2 | E = Atk 3", 20, vHeight - 30, 20, RAYWHITE);

                // Draw Pop-up Overlay
                if (showTekkenControls) {
                    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.7f));
                    
                    // Main Background (Bigger and Black)
                    DrawRectangle((vWidth / 2) - 300, (vHeight / 2) - 200, 600, 400, BLACK);
                    
                    // Left and Right Thick Borders
                    DrawRectangle((vWidth / 2) - 300, (vHeight / 2) - 200, 15, 400, RED); // Left border
                    DrawRectangle((vWidth / 2) + 285, (vHeight / 2) - 200, 15, 400, RED); // Right border
                    
                    // Thin White Outline
                    DrawRectangleLines((vWidth / 2) - 300, (vHeight / 2) - 200, 600, 400, WHITE);
                    
                    DrawText("FIGHT CONTROLS", (vWidth / 2) - MeasureText("FIGHT CONTROLS", 40) / 2, (vHeight / 2) - 150, 40, RAYWHITE);
                    DrawText("Movement: A / D", (vWidth / 2) - 200, (vHeight / 2) - 70, 20, LIGHTGRAY);
                    DrawText("Jump: SPACE   Shield: S   Run: L-SHIFT", (vWidth / 2) - 200, (vHeight / 2) - 20, 20, LIGHTGRAY);
                    DrawText("Attacks: L-Click / R-Click / E", (vWidth / 2) - 200, (vHeight / 2) + 30, 20, LIGHTGRAY);
                    
                    Rectangle okButton = { (vWidth / 2) - 60, (vHeight / 2) + 110, 120, 50 };
                    Color btnColor = CheckCollisionPointRec(vMouse, okButton) ? LIGHTGRAY : GRAY; // Hover effect
                    DrawRectangleRec(okButton, btnColor);
                    DrawRectangleLinesEx(okButton, 2, WHITE);
                    DrawText("FIGHT", (int)okButton.x + 30, (int)okButton.y + 15, 20, BLACK);
                }

                if (!showTekkenControls) {
                    if (tekkenShowResultOverlay && tekkenFinalWin) {
                        DrawBattleWinOverlay(vWidth, vHeight);
                    } else if (tekkenShowResultOverlay && !tekkenFinalWin) {
                        DrawBattleDeathOverlay(vWidth, vHeight);
                    }
                }
            }
            // ---------------------------------------

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
    UnloadStoryScene(&scene1);   UnloadSceneData(&scene1_data);
    UnloadStoryScene(&scene2);   UnloadSceneData(&scene2_data);
    UnloadStoryScene(&scene3);   UnloadSceneData(&scene3_data);
    UnloadStoryScene(&scene4);   UnloadSceneData(&scene4_data);
    UnloadStoryScene(&scene5);   UnloadSceneData(&scene5_data);
    UnloadStoryScene(&scene6);   UnloadSceneData(&scene6_data);
    UnloadStoryScene(&scene7);   UnloadSceneData(&scene7_data);
    UnloadStoryScene(&scene8);   UnloadSceneData(&scene8_data);
    UnloadStoryScene(&scene9);   UnloadSceneData(&scene9_data);
    UnloadStoryScene(&scene10);  UnloadSceneData(&scene10_data);
    UnloadStoryScene(&scene11);  UnloadSceneData(&scene11_data);
    UnloadStoryScene(&scene12);  UnloadSceneData(&scene12_data);
    UnloadStoryScene(&scene13_1); UnloadSceneData(&scene13_1_data);
    UnloadStoryScene(&scene14_1); UnloadSceneData(&scene14_1_data);
    UnloadStoryScene(&scene13_2); UnloadSceneData(&scene13_2_data);
    UnloadStoryScene(&scene14_2); UnloadSceneData(&scene14_2_data);
    UnloadStoryScene(&scene15_2); UnloadSceneData(&scene15_2_data);
    UnloadStoryScene(&scene16_2); UnloadSceneData(&scene16_2_data);
    UnloadStoryScene(&scene17_2); UnloadSceneData(&scene17_2_data);
    UnloadStoryScene(&scene18_2_1); UnloadSceneData(&scene18_2_1_data);
    UnloadStoryScene(&scene19_2_1); UnloadSceneData(&scene19_2_1_data);
    UnloadStoryScene(&scene20_2_1); UnloadSceneData(&scene20_2_1_data);
    UnloadStoryScene(&scene21_2_1); UnloadSceneData(&scene21_2_1_data);
    UnloadStoryScene(&scene18_2_2); UnloadSceneData(&scene18_2_2_data);

    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) UnloadEnemy(&gameState1.regularBandits[i]);
    UnloadEnemy(&gameState1.bossBandit);
    UnloadPlayer(&gameState1.player);
    UnloadTilemap(&gameState1.map);

    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) UnloadEnemy(&gameState2.regularBandits[i]);
    UnloadEnemy(&gameState2.bossBandit);
    UnloadPlayer(&gameState2.player);
    UnloadTilemap(&gameState2.map);

    UnloadTexture(gameState1.key.texture);
    UnloadTexture(gameState1.heartTexture);
    UnloadTexture(gameState1.speedTexture);

    UnloadTexture(gameState2.key.texture);
    UnloadTexture(gameState2.heartTexture);
    UnloadTexture(gameState2.speedTexture);

    UnloadTexture(menu.background);
    UnloadTexture(menu.settingsBg);
    UnloadTexture(menu.saveBg);
    UnloadRenderTexture(target);

    // --- TEKKEN CLEANUP ---
    for(int i = 0; i < NUM_STATES; i++) {
        UnloadTexture(playerTekken.textures[i]);
        UnloadTexture(enemyTekken.textures[i]);
    }
    UnloadTexture(tekkenBg);
    UnloadTexture(tekkenBg2);
    UnloadTilemap(&tekkenMap);
    // ----------------------

    UnloadMusicStream(menuMusic);
    UnloadMusicStream(storyMusic);
    UnloadMusicStream(inGameMusic);
    UnloadMusicStream(duelMusic);

    UnloadBattleSfx();
    UnloadEnemyAttackSfx();
    UnloadSound(pressButtonSfx);
    UnloadSound(loseSfx);
    UnloadSound(winSfx);
    UnloadSound(battleSfx);
    UnloadSound(clappingSfx);
    UnloadSound(heartPickSfx);
    UnloadSound(speedSfx);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
