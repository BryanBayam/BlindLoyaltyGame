#include "raylib.h"
#include <math.h>
#include "character.h"
#include "menu.h"
#include "gameplay.h"

typedef enum { SCREEN_MENU, SCREEN_GAMEPLAY } GameScreen;

int main(void) {
    // Resolution
    const int vWidth = 1280;
    const int vHeight = 720;
    const int mapWidth = 2500; 
    const int mapHeight = 2500;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(vWidth, vHeight, "Blind Loyalty - C Edition");
    SetTargetFPS(60);

    // When game opened, first thing open is menu screen
    GameScreen currentScreen = SCREEN_MENU;
    
    Player player;
    InitPlayer(&player, (Vector2){ mapWidth/2.0f, mapHeight/2.0f });

    Menu menu = { 0 };
    // Loading the background
    menu.background = LoadTexture("images/Background/menu.jpg");

    Camera2D camera = { 0 };
    camera.offset = (Vector2){ vWidth/2.0f, vHeight/2.0f };
    camera.zoom = 1.0f;

    RenderTexture2D target = LoadRenderTexture(vWidth, vHeight);

    // main Loop
    while (!WindowShouldClose()) {
        // Update Mouse Virtual
       Vector2 mouse = GetMousePosition(); 

    float scale = fminf((float)GetScreenWidth()/vWidth, (float)GetScreenHeight()/vHeight);
    Vector2 vMouse = { (mouse.x - (GetScreenWidth() - (vWidth*scale))*0.5f)/scale, 
                       (mouse.y - (GetScreenHeight() - (vHeight*scale))*0.5f)/scale };

        if (currentScreen == SCREEN_MENU) {
            int action = UpdateMenu(&menu, vMouse, vWidth, vHeight);
            if (action == 1) currentScreen = SCREEN_GAMEPLAY;
            if (action == 2) break;
        } 
        else if (currentScreen == SCREEN_GAMEPLAY) {
            
            //  UpdatePlayer
            UpdatePlayer(&player, mapWidth, mapHeight);
            
            // Camera follow player
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
                    DrawMap1(mapWidth, mapHeight);
                    
                    // DrawPlayer
                    DrawPlayer(&player);
                EndMode2D();
                
                // UI doesn't follow camera
                DrawText("ESC TO MENU | LSHIFT TO RUN", 10, vHeight - 30, 20, RAYWHITE);
            }
            DrawCircleV(vMouse, 5, RED); 
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);
            // Auto scalling
            DrawTexturePro(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, (float)-target.texture.height },
                           (Rectangle){ (GetScreenWidth() - (vWidth*scale))*0.5f, (GetScreenHeight() - (vHeight*scale))*0.5f, vWidth*scale, vHeight*scale },
                           (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

    // Cleanup
    UnloadPlayer(&player); 
    
    UnloadTexture(menu.background);
    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}