#include "raylib.h"
#include <math.h>
#include "character.h"
#include "menu.h"
#include "tilemap.h"

typedef enum { SCREEN_MENU, SCREEN_GAMEPLAY } GameScreen;

int main(void) {
    const int vWidth = 1280;
    const int vHeight = 720;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(vWidth, vHeight, "Blind Loyalty - C Edition");
    SetTargetFPS(60);

    GameScreen currentScreen = SCREEN_MENU;

    Menu menu = { 0 };
    menu.background = LoadTexture("images/Background/TitleBackground.PNG");

    Tilemap map;
    if (!LoadTilemap(&map, "maps/map1/map1.json")) {
        TraceLog(LOG_ERROR, "Failed to load map1");
        UnloadTexture(menu.background);
        CloseWindow();
        return 1;
    }

    Player player;
    InitPlayer(&player, FindWalkableSpawn(&map));

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ vWidth / 2.0f, vHeight / 2.0f };
    camera.target = player.pos;
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

        if (currentScreen == SCREEN_MENU) {
            int action = UpdateMenu(&menu, vMouse, vWidth, vHeight);
            if (action == 1) currentScreen = SCREEN_GAMEPLAY;
            if (action == 2) break;
        }
        else if (currentScreen == SCREEN_GAMEPLAY) {
            UpdatePlayer(&player, &map);
            camera.target = player.pos;

            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MENU;
        }

        BeginTextureMode(target);
            ClearBackground(BLACK);

            if (currentScreen == SCREEN_MENU) {
                DrawMenu(&menu, vWidth, vHeight);
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

    UnloadPlayer(&player);
    UnloadTilemap(&map);
    UnloadTexture(menu.background);
    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}