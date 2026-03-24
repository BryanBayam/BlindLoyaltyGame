#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include "character.h"
#include "enemy.h"
#include "menu.h"
#include "tilemap.h"
#include "scene1.h"
#include "scene2.h"
#include "scene3.h"
#include "scene4.h"
#include "scene5.h"
#include "scene6.h"

#define MAX_ENEMIES 5
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

static int CountActiveEnemies(Enemy enemies[], int count) {
    int active = 0;
    for (int i = 0; i < count; i++) {
        if (enemies[i].active) active++;
    }
    return active;
}

static int FindFreeEnemySlot(Enemy enemies[], int count) {
    for (int i = 0; i < count; i++) {
        if (!enemies[i].active) return i;
    }
    return -1;
}

// NEW: restart gameplay state
static void ResetGameplay(Player *player, Tilemap *map, Enemy bandits[], Vector2 playerHistory[], int *historyIndex, float *spawnTimer, Camera2D *camera) {
    UnloadPlayer(player);
    InitPlayer(player, FindWalkableSpawn(map));

    for (int i = 0; i < MAX_ENEMIES; i++) {
        UnloadEnemy(&bandits[i]);
        InitEnemy(&bandits[i], (Vector2){ 0.0f, 0.0f });
    }

    for (int i = 0; i < PLAYER_HISTORY_SIZE; i++) {
        playerHistory[i] = player->pos;
    }

    *historyIndex = 0;
    *spawnTimer = 15.0f;
    camera->target = player->pos;
}

// NEW: death overlay
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

int main(void) {
    const int vWidth = 1280;
    const int vHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(vWidth, vHeight, "Blind Loyalty - C Edition");
    SetTargetFPS(60);

    GameScreen currentScreen = SCREEN_LOADING;
    int loadStep = 0;
    float loadProgress = 0.0f;

    Menu menu = { 0 };
    Tilemap map = { 0 };
    Player player = { 0 };
    Enemy bandits[MAX_ENEMIES] = { 0 };
    Scene1 scene1 = { 0 };
    Scene2 scene2 = { 0 };
    Scene3 scene3 = { 0 };
    Scene4 scene4 = { 0 };
    Scene5 scene5 = { 0 };
    Scene6 scene6 = { 0 };

    Vector2 playerHistory[PLAYER_HISTORY_SIZE] = { 0 };
    int historyIndex = 0;
    float spawnTimer = 15.0f;

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ vWidth / 2.0f, vHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 3.0f;

    RenderTexture2D target = LoadRenderTexture(vWidth, vHeight);

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();

        float scale = fminf((float)GetScreenWidth() / vWidth, (float)GetScreenHeight() / vHeight);
        Vector2 vMouse = {
            (mouse.x - (GetScreenWidth() - (vWidth * scale)) * 0.5f) / scale,
            (mouse.y - (GetScreenHeight() - (vHeight * scale)) * 0.5f) / scale
        };

        bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        if (currentScreen == SCREEN_LOADING) {
            switch (loadStep) {
                case 0:
                    menu.background = LoadTexture("images/Background/TitleBackground.PNG");
                    loadProgress = 0.10f;
                    loadStep++;
                    break;

                case 1:
                    if (!LoadTilemap(&map, "maps/map1/map1.json")) {
                        TraceLog(LOG_ERROR, "Failed to load map1");
                        goto cleanup;
                    }
                    loadProgress = 0.20f;
                    loadStep++;
                    break;

                case 2:
                    InitPlayer(&player, FindWalkableSpawn(&map));
                    camera.target = player.pos;

                    for (int i = 0; i < PLAYER_HISTORY_SIZE; i++) {
                        playerHistory[i] = player.pos;
                    }

                    loadProgress = 0.30f;
                    loadStep++;
                    break;

                case 3:
                    for (int i = 0; i < MAX_ENEMIES; i++) {
                        InitEnemy(&bandits[i], (Vector2){ 0.0f, 0.0f });
                    }
                    loadProgress = 0.40f;
                    loadStep++;
                    break;

                case 4:
                    InitScene1(&scene1);
                    loadProgress = 0.50f;
                    loadStep++;
                    break;

                case 5:
                    InitScene2(&scene2);
                    loadProgress = 0.60f;
                    loadStep++;
                    break;

                case 6:
                    InitScene3(&scene3);
                    loadProgress = 0.70f;
                    loadStep++;
                    break;

                case 7:
                    InitScene4(&scene4);
                    loadProgress = 0.80f;
                    loadStep++;
                    break;

                case 8:
                    InitScene5(&scene5);
                    loadProgress = 0.90f;
                    loadStep++;
                    break;

                case 9:
                    InitScene6(&scene6);
                    loadProgress = 1.0f;
                    loadStep++;
                    break;

                case 10:
                    currentScreen = SCREEN_MENU;
                    break;
            }
        }
        else if (currentScreen == SCREEN_MENU) {
            int action = UpdateMenu(&menu, vMouse, vWidth, vHeight);
            if (action == 1) currentScreen = SCREEN_SCENE1;
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
            if (scene6.currentState == SCENE6_DONE) currentScreen = SCREEN_GAMEPLAY;
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            // ONLY update gameplay if player is alive
            if (!player.isDead) {
                UpdatePlayer(&player, &map);
                camera.target = player.pos;

                playerHistory[historyIndex] = player.pos;
                historyIndex = (historyIndex + 1) % PLAYER_HISTORY_SIZE;

                spawnTimer -= GetFrameTime();
                if (spawnTimer <= 0.0f) {
                    if (CountActiveEnemies(bandits, MAX_ENEMIES) < MAX_ENEMIES) {
                        int slot = FindFreeEnemySlot(bandits, MAX_ENEMIES);

                        if (slot != -1) {
                            Vector2 oldPos = playerHistory[historyIndex];

                            if (CanEnemySpawnAt(oldPos, &map)) {
                                SpawnEnemy(&bandits[slot], oldPos, &map);
                            }
                        }
                    }

                    spawnTimer = 15.0f;
                }

                for (int i = 0; i < MAX_ENEMIES; i++) {
                    UpdateEnemy(&bandits[i], &player, &map);
                }
            }
            else {
                // Restart when dead
                if (IsKeyPressed(KEY_R)) {
                    ResetGameplay(&player, &map, bandits, playerHistory, &historyIndex, &spawnTimer, &camera);
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MENU;
        }

        BeginTextureMode(target);
            ClearBackground(BLACK);

            if (currentScreen == SCREEN_LOADING) {
                const char* loadText = "LOADING ASSETS...";
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

                    for (int i = 0; i < MAX_ENEMIES; i++) {
                        DrawEnemy(&bandits[i]);
                    }

                    DrawPlayer(&player);
                EndMode2D();

                DrawPlayerUI(&player);
                DrawText("ESC TO MENU | LSHIFT TO RUN", 10, vHeight - 30, 20, RAYWHITE);

                // NEW: draw death overlay on top of gameplay
                if (player.isDead) {
                    DrawDeathOverlay(vWidth, vHeight);
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

    for (int i = 0; i < MAX_ENEMIES; i++) {
        UnloadEnemy(&bandits[i]);
    }

    UnloadPlayer(&player);
    UnloadTilemap(&map);
    UnloadTexture(menu.background);
    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}