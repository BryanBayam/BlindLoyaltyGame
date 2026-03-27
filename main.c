#include "raylib.h"
#include <math.h>
#include <stdbool.h>

#include "tilemap.h"
#include "character.h"
#include "enemy.h"
#include "menu.h"
#include "scene1.h"
#include "scene2.h"
#include "scene3.h"
#include "scene4.h"
#include "scene5.h"
#include "scene6.h"

#define MAX_REGULAR_ENEMIES 5
#define INITIAL_REGULAR_ENEMIES 2
#define PLAYER_HISTORY_SIZE 900

#define KEY_DRAW_SIZE 16.0f
#define KEY_HITBOX_SIZE 20.0f
#define KEY_SPAWN_DELAY 10.0f

#define HEART_COUNT 2
#define SPEED_COUNT 3
#define PICKUP_DRAW_SIZE 16.0f
#define PICKUP_HITBOX_SIZE 18.0f
#define PICKUP_MIN_SEPARATION 80.0f

#define REGULAR_RESPAWN_DELAY 22.0f
#define INITIAL_REGULAR_SPAWN_DELAY 5.0f
#define BOSS_SPAWN_DELAY 7.0f

#define REGULAR_MIN_SPAWN_DISTANCE 120.0f
#define BOSS_MIN_SPAWN_DISTANCE 160.0f

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

typedef struct KeyItem {
    Texture2D texture;
    Vector2 pos;
    Rectangle hitbox;
    bool spawned;
    bool collected;
    float spawnTimer;
} KeyItem;

typedef enum PickupType {
    PICKUP_HEART,
    PICKUP_SPEED
} PickupType;

typedef struct Pickup {
    Texture2D *texture;
    Vector2 pos;
    Rectangle hitbox;
    bool active;
    PickupType type;
} Pickup;

static float DistanceSquared(Vector2 a, Vector2 b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

static Rectangle MakeCenteredRect(Vector2 center, float size) {
    Rectangle rect = {
        center.x - size * 0.5f,
        center.y - size * 0.5f,
        size,
        size
    };
    return rect;
}

static Rectangle GetKeyHitbox(Vector2 pos) {
    return MakeCenteredRect(pos, KEY_HITBOX_SIZE);
}

static Rectangle GetPickupHitbox(Vector2 pos) {
    return MakeCenteredRect(pos, PICKUP_HITBOX_SIZE);
}

static bool IsRectFullyInsideMap(const Tilemap *map, Rectangle rect) {
    float mapPixelWidth = (float)map->width * (float)map->tileWidth;
    float mapPixelHeight = (float)map->height * (float)map->tileHeight;

    return rect.x >= 0.0f &&
           rect.y >= 0.0f &&
           rect.x + rect.width <= mapPixelWidth &&
           rect.y + rect.height <= mapPixelHeight;
}

static bool CanPlaceRectAt(const Tilemap *map, Rectangle rect) {
    if (!IsRectFullyInsideMap(map, rect)) return false;
    if (CheckMapCollision(map, rect)) return false;
    return true;
}

static bool CanPlaceKeyAt(Vector2 pos, const Tilemap *map) {
    return CanPlaceRectAt(map, GetKeyHitbox(pos));
}

static bool CanPlacePickupAt(Vector2 pos, const Tilemap *map) {
    return CanPlaceRectAt(map, GetPickupHitbox(pos));
}

static Vector2 FindFurthestKeySpawn(const Tilemap *map, Vector2 playerPos) {
    Vector2 bestPos = playerPos;
    float bestDistSq = -1.0f;

    for (int row = 0; row < map->height; row++) {
        for (int col = 0; col < map->width; col++) {
            Vector2 candidate = {
                col * map->tileWidth + map->tileWidth * 0.5f,
                row * map->tileHeight + map->tileHeight * 0.5f
            };

            if (!CanPlaceKeyAt(candidate, map)) continue;

            float distSq = DistanceSquared(candidate, playerPos);
            if (distSq > bestDistSq) {
                bestDistSq = distSq;
                bestPos = candidate;
            }
        }
    }

    return bestPos;
}

static Vector2 FindRandomWalkablePoint(const Tilemap *map) {
    for (int attempt = 0; attempt < 1000; attempt++) {
        int col = GetRandomValue(0, map->width - 1);
        int row = GetRandomValue(0, map->height - 1);

        Vector2 candidate = {
            col * map->tileWidth + map->tileWidth * 0.5f,
            row * map->tileHeight + map->tileHeight * 0.5f
        };

        if (CanPlacePickupAt(candidate, map)) {
            return candidate;
        }
    }

    return FindWalkableSpawn(map);
}

static bool IsFarEnoughFromOtherPickups(Vector2 pos, Pickup pickups[], int count, float minSeparation) {
    float minSeparationSq = minSeparation * minSeparation;

    for (int i = 0; i < count; i++) {
        if (!pickups[i].active) continue;
        if (DistanceSquared(pos, pickups[i].pos) < minSeparationSq) {
            return false;
        }
    }
    return true;
}

static void SpawnPickups(const Tilemap *map,
                         Pickup hearts[], int heartCount, Texture2D *heartTexture,
                         Pickup speeds[], int speedCount, Texture2D *speedTexture) {
    Pickup all[HEART_COUNT + SPEED_COUNT] = { 0 };
    int totalPlaced = 0;

    for (int i = 0; i < heartCount; i++) {
        hearts[i].active = false;
        hearts[i].type = PICKUP_HEART;
        hearts[i].texture = heartTexture;
    }

    for (int i = 0; i < speedCount; i++) {
        speeds[i].active = false;
        speeds[i].type = PICKUP_SPEED;
        speeds[i].texture = speedTexture;
    }

    for (int i = 0; i < heartCount; i++) {
        for (int attempt = 0; attempt < 1000; attempt++) {
            Vector2 pos = FindRandomWalkablePoint(map);
            if (!IsFarEnoughFromOtherPickups(pos, all, totalPlaced, PICKUP_MIN_SEPARATION)) continue;

            hearts[i].pos = pos;
            hearts[i].hitbox = GetPickupHitbox(pos);
            hearts[i].active = true;
            all[totalPlaced++] = hearts[i];
            break;
        }
    }

    for (int i = 0; i < speedCount; i++) {
        for (int attempt = 0; attempt < 1000; attempt++) {
            Vector2 pos = FindRandomWalkablePoint(map);
            if (!IsFarEnoughFromOtherPickups(pos, all, totalPlaced, PICKUP_MIN_SEPARATION)) continue;

            speeds[i].pos = pos;
            speeds[i].hitbox = GetPickupHitbox(pos);
            speeds[i].active = true;
            all[totalPlaced++] = speeds[i];
            break;
        }
    }
}

static void ResetKey(KeyItem *key) {
    key->pos = (Vector2){ 0.0f, 0.0f };
    key->hitbox = GetKeyHitbox(key->pos);
    key->spawned = false;
    key->collected = false;
    key->spawnTimer = KEY_SPAWN_DELAY;
}

static int CountActiveRegularEnemies(Enemy enemies[], int count) {
    int active = 0;
    for (int i = 0; i < count; i++) {
        if (enemies[i].active && enemies[i].type == ENEMY_REGULAR) active++;
    }
    return active;
}

static int FindFreeRegularEnemySlot(Enemy enemies[], int count) {
    for (int i = 0; i < count; i++) {
        if (!enemies[i].active && enemies[i].type == ENEMY_REGULAR) return i;
    }
    return -1;
}

static Vector2 FindEnemySpawnWithDistance(const Tilemap *map,
                                          Vector2 playerPos,
                                          float minDistance,
                                          Enemy regulars[],
                                          int regularCount,
                                          Enemy *boss) {
    float minDistSq = minDistance * minDistance;

    for (int attempt = 0; attempt < 3000; attempt++) {
        int col = GetRandomValue(1, map->width - 2);
        int row = GetRandomValue(1, map->height - 2);

        Vector2 candidate = {
            col * map->tileWidth + map->tileWidth * 0.5f,
            row * map->tileHeight + map->tileHeight * 0.5f
        };

        if (!CanEnemySpawnAt(candidate, map)) continue;
        if (DistanceSquared(candidate, playerPos) < minDistSq) continue;

        bool overlapsEnemy = false;
        Rectangle candBox = GetEnemyHitbox(candidate);

        for (int i = 0; i < regularCount; i++) {
            if (!regulars[i].active) continue;
            if (CheckCollisionRecs(candBox, GetEnemyHitbox(regulars[i].pos))) {
                overlapsEnemy = true;
                break;
            }
        }

        if (!overlapsEnemy && boss != NULL && boss->active) {
            if (CheckCollisionRecs(candBox, GetEnemyHitbox(boss->pos))) {
                overlapsEnemy = true;
            }
        }

        if (overlapsEnemy) continue;

        return candidate;
    }

    return FindWalkableSpawn(map);
}

static void SpawnInitialRegularBandits(Enemy regulars[],
                                       int regularCount,
                                       Enemy *boss,
                                       const Tilemap *map,
                                       Vector2 playerPos) {
    int spawned = 0;

    for (int i = 0; i < regularCount && spawned < INITIAL_REGULAR_ENEMIES; i++) {
        Vector2 pos = FindEnemySpawnWithDistance(map,
                                                 playerPos,
                                                 REGULAR_MIN_SPAWN_DISTANCE,
                                                 regulars,
                                                 regularCount,
                                                 boss);
        SpawnEnemy(&regulars[i], pos, map);
        if (regulars[i].active) spawned++;
    }
}

static void SpawnBossBandit(Enemy *boss,
                            Enemy regulars[],
                            int regularCount,
                            const Tilemap *map,
                            Vector2 playerPos) {
    Vector2 pos = FindEnemySpawnWithDistance(map,
                                             playerPos,
                                             BOSS_MIN_SPAWN_DISTANCE,
                                             regulars,
                                             regularCount,
                                             boss);
    SpawnEnemy(boss, pos, map);
}

static void DrawDeathOverlay(int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));

    const char *title = "YOU DIED";
    const char *msg = "Press R to Restart";

    int titleSize = 64;
    int msgSize = 28;

    int titleW = MeasureText(title, titleSize);
    int msgW = MeasureText(msg, msgSize);

    DrawText(title, (vWidth - titleW) / 2, vHeight / 2 - 60, titleSize, RED);
    DrawText(msg, (vWidth - msgW) / 2, vHeight / 2 + 20, msgSize, MAROON);
}

static void DrawWinOverlay(int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));

    const char *title = "VICTORY";
    const char *msg = "You found the key! Press R to Restart";

    int titleSize = 64;
    int msgSize = 28;

    int titleW = MeasureText(title, titleSize);
    int msgW = MeasureText(msg, msgSize);

    DrawText(title, (vWidth - titleW) / 2, vHeight / 2 - 60, titleSize, GOLD);
    DrawText(msg, (vWidth - msgW) / 2, vHeight / 2 + 20, msgSize, YELLOW);
}

static void DrawInstructionsOverlay(int vWidth, int vHeight) {
    Rectangle panel = {
        vWidth * 0.18f,
        vHeight * 0.14f,
        vWidth * 0.64f,
        vHeight * 0.58f
    };

    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.45f));
    DrawRectangleRounded(panel, 0.08f, 16, Fade(BLACK, 0.82f));
    DrawRectangleRoundedLinesEx(panel, 0.08f, 16, 3.0f, RAYWHITE);

    int x = (int)panel.x + 40;
    int y = (int)panel.y + 30;

    DrawText("INSTRUCTIONS", x, y, 36, RAYWHITE);
    y += 70;
    DrawText("- Find the Key to win.", x, y, 26, YELLOW);
    y += 45;
    DrawText("- Press WASD to Move.", x, y, 26, RAYWHITE);
    y += 45;
    DrawText("- Press Shift + WASD to Run.", x, y, 26, RAYWHITE);
    y += 45;
    DrawText("- Avoid the BANDITS.", x, y, 26, RED);
    y += 80;
    DrawText("Press any key to begin", x, y, 28, GREEN);
}

static void DrawKey(const KeyItem *key) {
    if (!key->spawned || key->collected) return;
    if (key->texture.id <= 0) return;

    Rectangle src = { 0, 0, (float)key->texture.width, (float)key->texture.height };
    Rectangle dst = { key->pos.x, key->pos.y, KEY_DRAW_SIZE, KEY_DRAW_SIZE };
    Vector2 origin = { KEY_DRAW_SIZE / 2.0f, KEY_DRAW_SIZE / 2.0f };
    DrawTexturePro(key->texture, src, dst, origin, 0.0f, WHITE);
}

static void DrawPickup(const Pickup *pickup) {
    if (!pickup->active) return;
    if (pickup->texture == NULL || pickup->texture->id <= 0) return;

    Rectangle src = { 0, 0, (float)pickup->texture->width, (float)pickup->texture->height };
    Rectangle dst = { pickup->pos.x, pickup->pos.y, PICKUP_DRAW_SIZE, PICKUP_DRAW_SIZE };
    Vector2 origin = { PICKUP_DRAW_SIZE / 2.0f, PICKUP_DRAW_SIZE / 2.0f };
    DrawTexturePro(*pickup->texture, src, dst, origin, 0.0f, WHITE);
}

static void ResetGameplay(Player *player,
                          Tilemap *map,
                          Enemy regulars[],
                          int regularCount,
                          Enemy *boss,
                          Vector2 playerHistory[],
                          int *historyIndex,
                          float *regularRespawnTimer,
                          float *initialRegularSpawnTimer,
                          float *bossSpawnTimer,
                          bool *initialRegularsSpawned,
                          bool *bossSpawned,
                          Camera2D *camera,
                          KeyItem *key,
                          Pickup hearts[],
                          Pickup speeds[],
                          Texture2D *heartTexture,
                          Texture2D *speedTexture,
                          bool *gameWon,
                          bool *showInstructions) {
    UnloadPlayer(player);
    InitPlayer(player, FindWalkableSpawn(map));

    for (int i = 0; i < regularCount; i++) {
        UnloadEnemy(&regulars[i]);
        InitEnemy(&regulars[i], (Vector2){ 0.0f, 0.0f });
    }

    UnloadEnemy(boss);
    InitBossEnemy(boss, (Vector2){ 0.0f, 0.0f });

    for (int i = 0; i < PLAYER_HISTORY_SIZE; i++) {
        playerHistory[i] = player->pos;
    }

    *historyIndex = 0;
    *regularRespawnTimer = REGULAR_RESPAWN_DELAY;
    *initialRegularSpawnTimer = INITIAL_REGULAR_SPAWN_DELAY;
    *bossSpawnTimer = BOSS_SPAWN_DELAY;
    *initialRegularsSpawned = false;
    *bossSpawned = false;
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

    Scene1 scene1 = { 0 };
    Scene2 scene2 = { 0 };
    Scene3 scene3 = { 0 };
    Scene4 scene4 = { 0 };
    Scene5 scene5 = { 0 };
    Scene6 scene6 = { 0 };

    Vector2 playerHistory[PLAYER_HISTORY_SIZE] = { 0 };
    int historyIndex = 0;
    float regularRespawnTimer = REGULAR_RESPAWN_DELAY;
    float initialRegularSpawnTimer = INITIAL_REGULAR_SPAWN_DELAY;
    float bossSpawnTimer = BOSS_SPAWN_DELAY;
    bool initialRegularsSpawned = false;
    bool bossSpawned = false;

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ vWidth / 2.0f, vHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 3.0f;

    RenderTexture2D target = LoadRenderTexture(vWidth, vHeight);

    Music menuMusic = { 0 };
    Music storyMusic = { 0 };
    Music inGameMusic = { 0 };
    Music *activeMusic = NULL;

    bool fadeOutMusic = false;
    float musicVolume = 1.0f;
    GameScreen nextScreenAfterFade = SCREEN_MENU;

    KeyItem key = { 0 };
    Texture2D heartTexture = { 0 };
    Texture2D speedTexture = { 0 };
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

        if (activeMusic != NULL) {
            UpdateMusicStream(*activeMusic);
        }

        if (fadeOutMusic && activeMusic != NULL) {
            musicVolume -= GetFrameTime();

            if (musicVolume <= 0.0f) {
                musicVolume = 0.0f;
                StopMusicStream(*activeMusic);
                fadeOutMusic = false;
                currentScreen = nextScreenAfterFade;

                if (currentScreen == SCREEN_SCENE1) {
                    activeMusic = &storyMusic;
                    musicVolume = 1.0f;
                    SetMusicVolume(*activeMusic, musicVolume);
                    PlayMusicStream(*activeMusic);
                } else if (currentScreen == SCREEN_GAMEPLAY) {
                    activeMusic = &inGameMusic;
                    musicVolume = 1.0f;
                    SetMusicVolume(*activeMusic, musicVolume);
                    PlayMusicStream(*activeMusic);
                } else if (currentScreen == SCREEN_MENU) {
                    activeMusic = &menuMusic;
                    musicVolume = 1.0f;
                    SetMusicVolume(*activeMusic, musicVolume);
                    PlayMusicStream(*activeMusic);
                }
            }

            SetMusicVolume(*activeMusic, musicVolume);
            goto render_phase;
        }

        if (currentScreen == SCREEN_LOADING) {
            switch (loadStep) {
                case 0:
                    menu.background = LoadTexture("images/Background/TitleBackground.PNG");
                    menuMusic = LoadMusicStream("Music/MainMenu.ogg");
                    storyMusic = LoadMusicStream("Music/Story.ogg");
                    inGameMusic = LoadMusicStream("Music/ingame.ogg");

                    key.texture = LoadTexture("images/Elements/key.png");
                    heartTexture = LoadTexture("images/Elements/heart.png");
                    speedTexture = LoadTexture("images/Elements/speed.png");

                    loadProgress = 0.12f;
                    loadStep++;
                    break;

                case 1:
                    if (!LoadTilemap(&map, "maps/map1/map1.json")) {
                        TraceLog(LOG_ERROR, "Failed to load map1");
                        goto cleanup;
                    }
                    loadProgress = 0.24f;
                    loadStep++;
                    break;

                case 2:
                    InitPlayer(&player, FindWalkableSpawn(&map));
                    camera.target = player.pos;

                    for (int i = 0; i < PLAYER_HISTORY_SIZE; i++) {
                        playerHistory[i] = player.pos;
                    }

                    ResetKey(&key);
                    SpawnPickups(&map, hearts, HEART_COUNT, &heartTexture, speeds, SPEED_COUNT, &speedTexture);

                    loadProgress = 0.36f;
                    loadStep++;
                    break;

                case 3:
                    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) {
                        InitEnemy(&regularBandits[i], (Vector2){ 0.0f, 0.0f });
                    }
                    InitBossEnemy(&bossBandit, (Vector2){ 0.0f, 0.0f });

                    loadProgress = 0.48f;
                    loadStep++;
                    break;

                case 4:
                    InitScene1(&scene1);
                    loadProgress = 0.58f;
                    loadStep++;
                    break;

                case 5:
                    InitScene2(&scene2);
                    loadProgress = 0.68f;
                    loadStep++;
                    break;

                case 6:
                    InitScene3(&scene3);
                    loadProgress = 0.78f;
                    loadStep++;
                    break;

                case 7:
                    InitScene4(&scene4);
                    loadProgress = 0.86f;
                    loadStep++;
                    break;

                case 8:
                    InitScene5(&scene5);
                    loadProgress = 0.94f;
                    loadStep++;
                    break;

                case 9:
                    InitScene6(&scene6);
                    loadProgress = 1.0f;
                    loadStep++;
                    break;

                case 10:
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
                fadeOutMusic = true;
                nextScreenAfterFade = SCREEN_SCENE1;
            }
            if (action == 2) break;
        }
        else if (currentScreen == SCREEN_SCENE1) {
            UpdateScene1(&scene1, vMouse, mouseClicked, vWidth);
            if (scene1.currentState == SCENE1_DONE) currentScreen = SCREEN_SCENE2;
        }
        else if (currentScreen == SCREEN_SCENE2) {
            UpdateScene2(&scene2, vMouse, mouseClicked, vWidth);
            if (scene2.currentState == SCENE2_DONE) currentScreen = SCREEN_SCENE3;
        }
        else if (currentScreen == SCREEN_SCENE3) {
            UpdateScene3(&scene3, vMouse, mouseClicked, vWidth);
            if (scene3.currentState == SCENE3_DONE) currentScreen = SCREEN_SCENE4;
        }
        else if (currentScreen == SCREEN_SCENE4) {
            UpdateScene4(&scene4, vMouse, mouseClicked, vWidth);
            if (scene4.currentState == SCENE4_DONE) currentScreen = SCREEN_SCENE5;
        }
        else if (currentScreen == SCREEN_SCENE5) {
            UpdateScene5(&scene5, vMouse, mouseClicked, vWidth);
            if (scene5.currentState == SCENE5_DONE) currentScreen = SCREEN_SCENE6;
        }
        else if (currentScreen == SCREEN_SCENE6) {
            UpdateScene6(&scene6, vMouse, mouseClicked, vWidth);
            if (scene6.currentState == SCENE6_DONE) {
                ResetGameplay(&player, &map,
                              regularBandits, MAX_REGULAR_ENEMIES, &bossBandit,
                              playerHistory, &historyIndex,
                              &regularRespawnTimer,
                              &initialRegularSpawnTimer,
                              &bossSpawnTimer,
                              &initialRegularsSpawned,
                              &bossSpawned,
                              &camera, &key, hearts, speeds,
                              &heartTexture, &speedTexture,
                              &gameWon, &showInstructions);

                fadeOutMusic = true;
                nextScreenAfterFade = SCREEN_GAMEPLAY;
            }
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            bool gameplayPaused = showInstructions || player.isDead || gameWon;

            if (showInstructions && GetKeyPressed() != 0) {
                showInstructions = false;
            }

            if (!gameplayPaused) {
                float dt = GetFrameTime();

                UpdatePlayer(&player, &map);
                camera.target = player.pos;

                playerHistory[historyIndex] = player.pos;
                historyIndex = (historyIndex + 1) % PLAYER_HISTORY_SIZE;

                initialRegularSpawnTimer -= dt;
                if (!initialRegularsSpawned && initialRegularSpawnTimer <= 0.0f) {
                    SpawnInitialRegularBandits(regularBandits,
                                               MAX_REGULAR_ENEMIES,
                                               &bossBandit,
                                               &map,
                                               player.pos);
                    initialRegularsSpawned = true;
                }

                bossSpawnTimer -= dt;
                if (!bossSpawned && bossSpawnTimer <= 0.0f) {
                    SpawnBossBandit(&bossBandit,
                                    regularBandits,
                                    MAX_REGULAR_ENEMIES,
                                    &map,
                                    player.pos);
                    bossSpawned = true;
                }

                key.spawnTimer -= dt;
                if (!key.spawned && key.spawnTimer <= 0.0f) {
                    key.pos = FindFurthestKeySpawn(&map, player.pos);
                    key.hitbox = GetKeyHitbox(key.pos);
                    key.spawned = true;
                }

                if (key.spawned && !key.collected &&
                    CheckCollisionRecs(GetPlayerHitbox(player.pos), key.hitbox)) {
                    key.collected = true;
                    gameWon = true;
                }

                for (int i = 0; i < HEART_COUNT; i++) {
                    if (hearts[i].active &&
                        CheckCollisionRecs(GetPlayerHitbox(player.pos), hearts[i].hitbox)) {
                        HealPlayer(&player, 25.0f);
                        hearts[i].active = false;
                    }
                }

                for (int i = 0; i < SPEED_COUNT; i++) {
                    if (speeds[i].active &&
                        CheckCollisionRecs(GetPlayerHitbox(player.pos), speeds[i].hitbox)) {
                        player.speedMultiplier = 1.25f;
                        AddPlayerEnergy(&player, 20.0f);
                        speeds[i].active = false;
                    }
                }

                if (initialRegularsSpawned) {
                    regularRespawnTimer -= dt;
                    if (regularRespawnTimer <= 0.0f) {
                        if (CountActiveRegularEnemies(regularBandits, MAX_REGULAR_ENEMIES) < MAX_REGULAR_ENEMIES) {
                            int slot = FindFreeRegularEnemySlot(regularBandits, MAX_REGULAR_ENEMIES);
                            if (slot != -1) {
                                Vector2 spawnPos = FindEnemySpawnWithDistance(&map,
                                                                              player.pos,
                                                                              REGULAR_MIN_SPAWN_DISTANCE,
                                                                              regularBandits,
                                                                              MAX_REGULAR_ENEMIES,
                                                                              &bossBandit);

                                if (CanEnemySpawnAt(spawnPos, &map)) {
                                    SpawnEnemy(&regularBandits[slot], spawnPos, &map);
                                }
                            }
                        }
                        regularRespawnTimer = REGULAR_RESPAWN_DELAY;
                    }
                }

                for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) {
                    UpdateEnemy(&regularBandits[i],
                                regularBandits,
                                MAX_REGULAR_ENEMIES,
                                &bossBandit,
                                &player,
                                &map);
                }

                UpdateEnemy(&bossBandit,
                            regularBandits,
                            MAX_REGULAR_ENEMIES,
                            &bossBandit,
                            &player,
                            &map);
            }

            if ((player.isDead || gameWon) && IsKeyPressed(KEY_R)) {
                ResetGameplay(&player, &map,
                              regularBandits, MAX_REGULAR_ENEMIES, &bossBandit,
                              playerHistory, &historyIndex,
                              &regularRespawnTimer,
                              &initialRegularSpawnTimer,
                              &bossSpawnTimer,
                              &initialRegularsSpawned,
                              &bossSpawned,
                              &camera, &key, hearts, speeds,
                              &heartTexture, &speedTexture,
                              &gameWon, &showInstructions);
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                fadeOutMusic = true;
                nextScreenAfterFade = SCREEN_MENU;
            }
        }

render_phase:
        BeginTextureMode(target);
            ClearBackground(BLACK);

            if (currentScreen == SCREEN_LOADING) {
                const char *loadText = "LOADING ASSETS...";
                int textWidth = MeasureText(loadText, 40);
                DrawText(loadText, (vWidth / 2) - (textWidth / 2), (vHeight / 2) - 60, 40, RAYWHITE);

                Rectangle barBg = { (vWidth / 2.0f) - 200, (vHeight / 2.0f) + 10, 400, 30 };
                DrawRectangleRec(barBg, DARKGRAY);
                Rectangle barFill = { barBg.x, barBg.y, 400 * loadProgress, 30 };
                DrawRectangleRec(barFill, RAYWHITE);
                DrawRectangleLinesEx(barBg, 3.0f, LIGHTGRAY);
            }
            else if (currentScreen == SCREEN_MENU) {
                DrawMenu(&menu, vWidth, vHeight);
            }
            else if (currentScreen == SCREEN_SCENE1) {
                DrawScene1(&scene1, vWidth, vHeight);
            }
            else if (currentScreen == SCREEN_SCENE2) {
                DrawScene2(&scene2, vWidth, vHeight);
            }
            else if (currentScreen == SCREEN_SCENE3) {
                DrawScene3(&scene3, vWidth, vHeight);
            }
            else if (currentScreen == SCREEN_SCENE4) {
                DrawScene4(&scene4, vWidth, vHeight);
            }
            else if (currentScreen == SCREEN_SCENE5) {
                DrawScene5(&scene5, vWidth, vHeight);
            }
            else if (currentScreen == SCREEN_SCENE6) {
                DrawScene6(&scene6, vWidth, vHeight);
            }
            else if (currentScreen == SCREEN_GAMEPLAY) {
                BeginMode2D(camera);
                    DrawTilemapAll(&map);

                    DrawKey(&key);

                    for (int i = 0; i < HEART_COUNT; i++) DrawPickup(&hearts[i]);
                    for (int i = 0; i < SPEED_COUNT; i++) DrawPickup(&speeds[i]);

                    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) {
                        DrawEnemy(&regularBandits[i]);
                    }
                    DrawEnemy(&bossBandit);

                    DrawPlayer(&player);
                EndMode2D();

                DrawPlayerUI(&player);
                DrawText("ESC TO MENU | SHIFT TO RUN", 10, vHeight - 30, 20, RAYWHITE);

                if (!showInstructions && !player.isDead && !gameWon) {
                    if (!initialRegularsSpawned) {
                        DrawText("Bandits arrive in 5 seconds...", 10, vHeight - 90, 20, RED);
                    } else if (!bossSpawned) {
                        DrawText("Boss arrives in 7 seconds...", 10, vHeight - 90, 20, ORANGE);
                    }

                    if (!key.spawned) {
                        DrawText("Key will appear in 10 seconds...", 10, vHeight - 60, 20, YELLOW);
                    }
                }

                if (showInstructions) {
                    DrawInstructionsOverlay(vWidth, vHeight);
                }

                if (player.isDead) {
                    DrawDeathOverlay(vWidth, vHeight);
                }

                if (gameWon) {
                    DrawWinOverlay(vWidth, vHeight);
                }
            }

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
                (Vector2){ 0, 0 },
                0.0f,
                WHITE
            );
        EndDrawing();
    }

cleanup:
    UnloadScene1(&scene1);
    UnloadScene2(&scene2);
    UnloadScene3(&scene3);
    UnloadScene4(&scene4);
    UnloadScene5(&scene5);
    UnloadScene6(&scene6);

    for (int i = 0; i < MAX_REGULAR_ENEMIES; i++) {
        UnloadEnemy(&regularBandits[i]);
    }
    UnloadEnemy(&bossBandit);

    UnloadPlayer(&player);
    UnloadTilemap(&map);
    UnloadTexture(menu.background);
    UnloadTexture(key.texture);
    UnloadTexture(heartTexture);
    UnloadTexture(speedTexture);
    UnloadRenderTexture(target);

    UnloadMusicStream(menuMusic);
    UnloadMusicStream(storyMusic);
    UnloadMusicStream(inGameMusic);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}