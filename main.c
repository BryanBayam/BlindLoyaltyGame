#include "raylib.h"
#include <math.h>
#include "character.h"
#include "menu.h"
#include "tilemap.h"
#include "scene1.h" 
#include "scene2.h" 
#include "scene3.h" 
#include "scene4.h" 
#include "scene5.h" 
#include "scene6.h" // <-- 1. Include Scene 6

// 2. Added SCREEN_SCENE6 to the enum
typedef enum { SCREEN_LOADING, SCREEN_MENU, SCREEN_SCENE1, SCREEN_SCENE2, SCREEN_SCENE3, SCREEN_SCENE4, SCREEN_SCENE5, SCREEN_SCENE6, SCREEN_GAMEPLAY } GameScreen;

int main(void) {
    const int vWidth = 1280;
    const int vHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(vWidth, vHeight, "Blind Loyalty - C Edition");
    SetTargetFPS(60);

    GameScreen currentScreen = SCREEN_LOADING;
    int loadStep = 0;
    float loadProgress = 0.0f;

    // 3. Declare Scene 6 alongside the others
    Menu menu = { 0 };
    Tilemap map = { 0 };
    Player player = { 0 };
    Scene1 scene1 = { 0 };
    Scene2 scene2 = { 0 };
    Scene3 scene3 = { 0 };
    Scene4 scene4 = { 0 }; 
    Scene5 scene5 = { 0 }; 
    Scene6 scene6 = { 0 }; // Initialize Scene 6

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

        // --- UPDATE LOGIC ---
        if (currentScreen == SCREEN_LOADING) {
            // 4. Update the switch statement to include loading Scene 6
            switch (loadStep) {
                case 0:
                    menu.background = LoadTexture("images/Background/TitleBackground.PNG");
                    loadProgress = 0.11f; 
                    loadStep++;
                    break;
                case 1:
                    if (!LoadTilemap(&map, "maps/map1/map1.json")) {
                        TraceLog(LOG_ERROR, "Failed to load map1");
                        CloseWindow();
                        return 1;
                    }
                    loadProgress = 0.22f; 
                    loadStep++;
                    break;
                case 2:
                    InitPlayer(&player, FindWalkableSpawn(&map));
                    camera.target = player.pos; 
                    loadProgress = 0.33f; 
                    loadStep++;
                    break;
                case 3:
                    InitScene1(&scene1); 
                    loadProgress = 0.44f; 
                    loadStep++;
                    break;
                case 4:
                    InitScene2(&scene2); 
                    loadProgress = 0.55f; 
                    loadStep++;
                    break;
                case 5:
                    InitScene3(&scene3); 
                    loadProgress = 0.66f; 
                    loadStep++;
                    break;
                case 6:
                    InitScene4(&scene4); 
                    loadProgress = 0.77f; 
                    loadStep++;
                    break;
                case 7:
                    InitScene5(&scene5); 
                    loadProgress = 0.88f; 
                    loadStep++;
                    break;
                case 8:
                    InitScene6(&scene6); // Load Scene 6
                    loadProgress = 1.0f; 
                    loadStep++;
                    break;
                case 9:
                    // Done loading!
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
            if (scene1.currentState == SCENE1_DONE) {
                currentScreen = SCREEN_SCENE2; 
            }
        }
        else if (currentScreen == SCREEN_SCENE2) {
            UpdateScene2(&scene2, vMouse, mouseClicked, vWidth);
            if (scene2.currentState == SCENE2_DONE) {
                currentScreen = SCREEN_SCENE3; 
            }
        }
        else if (currentScreen == SCREEN_SCENE3) {
            UpdateScene3(&scene3, vMouse, mouseClicked, vWidth);
            if (scene3.currentState == SCENE3_DONE) {
                currentScreen = SCREEN_SCENE4; 
            }
        }
        else if (currentScreen == SCREEN_SCENE4) {
            UpdateScene4(&scene4, vMouse, mouseClicked, vWidth);
            if (scene4.currentState == SCENE4_DONE) {
                currentScreen = SCREEN_SCENE5; 
            }
        }
        else if (currentScreen == SCREEN_SCENE5) {
            UpdateScene5(&scene5, vMouse, mouseClicked, vWidth);
            if (scene5.currentState == SCENE5_DONE) {
                currentScreen = SCREEN_SCENE6; // 5. Transition to Scene 6
            }
        }
        else if (currentScreen == SCREEN_SCENE6) {
            // 6. Update Scene 6
            UpdateScene6(&scene6, vMouse, mouseClicked, vWidth);
            if (scene6.currentState == SCENE6_DONE) {
                currentScreen = SCREEN_GAMEPLAY; // 7. Finally, enter the game!
            }
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            UpdatePlayer(&player, &map);
            camera.target = player.pos;

            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MENU;
        }

        // --- DRAWING LOGIC ---
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
                // 8. Draw Scene 6
                DrawScene6(&scene6, vWidth, vHeight);
            }
            else {
                BeginMode2D(camera);
                    DrawTilemapAll(&map);
                    DrawPlayer(&player);
                EndMode2D();

                DrawText("ESC TO MENU | LSHIFT TO RUN", 10, vHeight - 30, 20, RAYWHITE);
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

    // --- Cleanup Memory ---
    UnloadScene1(&scene1);
    UnloadScene2(&scene2);
    UnloadScene3(&scene3); 
    UnloadScene4(&scene4); 
    UnloadScene5(&scene5); 
    UnloadScene6(&scene6); // 9. Clean up Scene 6
    UnloadPlayer(&player);
    UnloadTilemap(&map);
    UnloadTexture(menu.background);
    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}