#ifndef CHARACTER_H
#define CHARACTER_H
#include "raylib.h"

typedef struct Player {
    Vector2 pos;
    float walkSpeed;
    float runSpeed;
    float currentSpeed;
    
    Texture2D texWalk;
    Texture2D texRun;
    Texture2D *activeTex; 
    
    int frameCount;      // Akan diupdate otomatis tiap frame
    int currentFrame;
    int currentLine;     // 0:Down, 1:Left, 2:Right, 3:Up
    float frameTimer;
    float frameSpeed;
    
    float width;
    float height;
} Player;

static inline void InitPlayer(Player *p, Vector2 startPos) {
    p->pos = startPos;
    p->walkSpeed = 3.0f;
    p->runSpeed = 6.0f;
    p->currentSpeed = p->walkSpeed;
    
    // Pastikan path folder benar
    p->texWalk = LoadTexture("images/Character/Reuben/Reuben_walk.png");
    p->texRun = LoadTexture("images/Character/Reuben/Reuben_walk.png");
    
    p->activeTex = &p->texWalk;
    
    p->currentFrame = 0;
    p->currentLine = 0;
    p->frameTimer = 0.0f;
    p->frameSpeed = 0.12f; 
    
    p->width = 150.0f;
    p->height = 150.0f;
    
    // Inisialisasi awal frameCount (akan dihitung ulang di Update)
    if (p->activeTex->id > 0) {
        p->frameCount = p->activeTex->width / (p->activeTex->height / 4);
    } else {
        p->frameCount = 1;
    }
}

static inline void UpdatePlayer(Player *p, int mapW, int mapH) {
    bool isMoving = false;

    // 1. Logika Ganti Tekstur (Walk vs Run)
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        p->currentSpeed = p->runSpeed;
        p->activeTex = &p->texRun;
        p->frameSpeed = 0.07f; 
    } else {
        p->currentSpeed = p->walkSpeed;
        p->activeTex = &p->texWalk;
        p->frameSpeed = 0.12f;
    }

    // --- HITUNG JUMLAH FRAME OTOMATIS BERDASARKAN TEKSTUR AKTIF ---
    // Rumus: Lebar total gambar / (Tinggi total / 4 arah)
    if (p->activeTex->id > 0) {
        p->frameCount = p->activeTex->width / (p->activeTex->height / 4);
    }

    // 2. Kontrol Arah & Gerak (Arah baris disesuaikan dengan standar spritesheet kamu)
    if (IsKeyDown(KEY_RIGHT)) { p->pos.x += p->currentSpeed; p->currentLine = 1; isMoving = true; }
    else if (IsKeyDown(KEY_LEFT)) { p->pos.x -= p->currentSpeed; p->currentLine = 2; isMoving = true; }
    else if (IsKeyDown(KEY_UP)) { p->pos.y -= p->currentSpeed; p->currentLine = 0; isMoving = true; }
    else if (IsKeyDown(KEY_DOWN)) { p->pos.y += p->currentSpeed; p->currentLine = 3; isMoving = true; }

    // 3. Update Animasi
    if (isMoving) {
        p->frameTimer += GetFrameTime();
        if (p->frameTimer >= p->frameSpeed) {
            p->frameTimer = 0.0f;
            p->currentFrame++;
            if (p->currentFrame >= p->frameCount) p->currentFrame = 0;
        }
    } else {
        p->currentFrame = 0; 
    }

    // 4. Pembatas Map (Clamping)
    float hW = p->width / 2.0f;
    float hH = p->height / 2.0f;
    if (p->pos.x < hW) p->pos.x = hW;
    if (p->pos.x > mapW - hW) p->pos.x = mapW - hW;
    if (p->pos.y < hH) p->pos.y = hH;
    if (p->pos.y > mapH - hH) p->pos.y = mapH - hH;
}

static inline void DrawPlayer(Player *p) {
    // Jika tekstur gagal load (ID 0), gambar kotak merah
    if (p->activeTex->id <= 0) {
        DrawRectangleV((Vector2){p->pos.x - 25, p->pos.y - 25}, (Vector2){50, 50}, RED);
        return;
    }

    // Hitung ukuran satu kotak frame
    float frameW = (float)p->activeTex->width / p->frameCount;
    float frameH = (float)p->activeTex->height / 4;

    Rectangle sourceRec = { 
        (float)p->currentFrame * frameW, 
        (float)p->currentLine * frameH, 
        frameW, 
        frameH 
    };

    Rectangle destRec = { p->pos.x, p->pos.y, p->width, p->height };
    Vector2 origin = { p->width/2.0f, p->height/2.0f };

    DrawTexturePro(*p->activeTex, sourceRec, destRec, origin, 0.0f, WHITE);
}

static inline void UnloadPlayer(Player *p) {
    UnloadTexture(p->texWalk);
    UnloadTexture(p->texRun);
}

#endif