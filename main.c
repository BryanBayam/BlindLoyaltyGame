#include "raylib.h"
#include <math.h>

// Definisi status layar (Game State)
typedef enum GameScreen { SCREEN_MENU, SCREEN_GAMEPLAY } GameScreen;

// Mouse Location
Vector2 GetVirtualMousePosition(int vWidth, int vHeight) {
    Vector2 mouse = GetMousePosition();
    float scale = fminf((float)GetScreenWidth()/vWidth, (float)GetScreenHeight()/vHeight);
    float offsetX = (GetScreenWidth() - ((float)vWidth * scale)) * 0.5f;
    float offsetY = (GetScreenHeight() - ((float)vHeight * scale)) * 0.5f;

    Vector2 virtualMouse = { 0 };
    virtualMouse.x = (mouse.x - offsetX) / scale;
    virtualMouse.y = (mouse.y - offsetY) / scale;

    virtualMouse.x = fmaxf(0.0f, fminf(virtualMouse.x, (float)vWidth));
    virtualMouse.y = fmaxf(0.0f, fminf(virtualMouse.y, (float)vHeight));

    return virtualMouse;
}

int main(void) {
    const int virtualWidth = 1280;
    const int virtualHeight = 720;
    
    // Satukan flags agar Topmost aktif
    SetConfigFlags(FLAG_WINDOW_TOPMOST | FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    
    InitWindow(virtualWidth, virtualHeight, "Blind Loyalty");
    MaximizeWindow(); 
    SetWindowFocused();

    ChangeDirectory(GetApplicationDirectory());
    Texture2D menu = LoadTexture("images/Background/menu.jpg");
    Texture2D presidentsmall = LoadTexture("images/Character/President/PresidentSmallTrans.png");
    
    RenderTexture2D target = LoadRenderTexture(virtualWidth, virtualHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    // Variabel Game State & Player
    GameScreen currentScreen = SCREEN_MENU;
    Vector2 playerPos = { virtualWidth/2, virtualHeight/2 }; // Mulai di tengah
    float playerSpeed = 5.0f;
    int selected = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        Vector2 vMouse = GetVirtualMousePosition(virtualWidth, virtualHeight);

        // --- UPDATE LOGIC ---
        if (currentScreen == SCREEN_MENU) {
            if (IsKeyPressed(KEY_DOWN)) selected = 1;
            if (IsKeyPressed(KEY_UP)) selected = 0;

            Rectangle playBtnArea = { virtualWidth/2 - 100, 350, 200, 40 };
            Rectangle quitBtnArea = { virtualWidth/2 - 100, 420, 200, 40 };

            if (CheckCollisionPointRec(vMouse, playBtnArea)) selected = 0;
            if (CheckCollisionPointRec(vMouse, quitBtnArea)) selected = 1;

            // Klik PLAY untuk masuk ke Game
            if ((IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && selected == 0) {
                currentScreen = SCREEN_GAMEPLAY;
            }
            // Klik QUIT untuk keluar
            if ((IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) && selected == 1) {
                break;
            }
        } 
        else if (currentScreen == SCREEN_GAMEPLAY) {
            // Gerakan ala Pokemon
            if (IsKeyDown(KEY_RIGHT)) playerPos.x += playerSpeed;
            if (IsKeyDown(KEY_LEFT)) playerPos.x -= playerSpeed;
            if (IsKeyDown(KEY_UP)) playerPos.y -= playerSpeed;
            if (IsKeyDown(KEY_DOWN)) playerPos.y += playerSpeed;

            // Tekan ESC untuk kembali ke Menu
            if (IsKeyPressed(KEY_ESCAPE)) currentScreen = SCREEN_MENU;
        }

        // --- DRAWING LOGIC ---
        BeginTextureMode(target);
            ClearBackground(BLACK);

            if (currentScreen == SCREEN_MENU) {
                // Gambar Background Menu
                if (menu.id > 0) {
                    DrawTexturePro(menu, 
                        (Rectangle){ 0, 0, (float)menu.width, (float)menu.height },
                        (Rectangle){ 0, 0, (float)virtualWidth, (float)virtualHeight }, 
                        (Vector2){ 0, 0 }, 0.0f, WHITE);
                }

                DrawText("BLIND LOYALTY", virtualWidth/2 - MeasureText("BLIND LOYALTY", 60)/2, 150, 60, RAYWHITE);
                DrawText("PLAY", virtualWidth/2 - MeasureText("PLAY", 40)/2, 350, 40, (selected == 0) ? YELLOW : WHITE);
                DrawText("QUIT", virtualWidth/2 - MeasureText("QUIT", 40)/2, 420, 40, (selected == 1) ? YELLOW : WHITE);
            } 
            else if (currentScreen == SCREEN_GAMEPLAY) {
                // GAMBAR MAP (Background Hijau/Rumput)
                DrawRectangle(0, 0, virtualWidth, virtualHeight, DARKGREEN);
                DrawText("Playground Testing MODE", 20, 20, 20, LIGHTGRAY);

                // GAMBAR PLAYER (Bisa jalan-jalan)
                Rectangle sourceRec = { 0.0f, 0.0f, (float)presidentsmall.width, (float)presidentsmall.height };
                Rectangle destRec = { playerPos.x, playerPos.y, 80.0f, 80.0f }; // Ukuran char
                Vector2 origin = { 40.0f, 40.0f }; // Center of the character

                DrawTexturePro(presidentsmall, sourceRec, destRec, origin, 0.0f, WHITE);
                
                DrawText("ESC to Menu", 20, virtualHeight - 40, 20, RAYWHITE);
            }
            
            // Kursor Mouse Merah tetap ada di semua screen
            DrawCircleV(vMouse, 5, RED);
        EndTextureMode();

        // --- FINAL SCREEN DRAWING ---
        BeginDrawing();
            ClearBackground(BLACK);
            float scale = fminf((float)GetScreenWidth()/virtualWidth, (float)GetScreenHeight()/virtualHeight);
            DrawTexturePro(target.texture, 
                (Rectangle){ 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
                (Rectangle){ (GetScreenWidth() - ((float)virtualWidth * scale)) * 0.5f, 
                             (GetScreenHeight() - ((float)virtualHeight * scale)) * 0.5f, 
                             (float)virtualWidth * scale, (float)virtualHeight * scale },
                (Vector2){ 0, 0 }, 0.0f, WHITE);
        EndDrawing();
    }

    UnloadTexture(menu);
    UnloadTexture(presidentsmall);
    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}