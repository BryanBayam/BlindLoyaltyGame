#ifndef CHARACTER_H
#define CHARACTER_H
#include "raylib.h"
#include "raymath.h" // Butuh ini untuk Vector2Normalize

typedef struct Player {
    Vector2 pos;
    float walkSpeed;
    float runSpeed;
    float currentSpeed;
    
    Texture2D texWalk;
    Texture2D texRun;
    Texture2D *activeTex; 
    
    int frameCount;      
    int currentFrame;
    int currentLine;     // 0:Up, 1:Right, 2:Left, 3:Down (Sesuai gambar)
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
    
    p->texWalk = LoadTexture("images/Character/Reuben/Reuben_walk.png");
    p->texRun = LoadTexture("images/Character/Reuben/Reuben_walk.png");
    
    p->activeTex = &p->texWalk;
    
    p->currentFrame = 0;
    p->currentLine = 3; // start looking down
    p->frameTimer = 0.0f;
    p->frameSpeed = 0.12f; 
    
    p->width = 128.0f;  // Reuben / our MC size
    p->height = 128.0f;
    
    if (p->activeTex->id > 0) {
        p->frameCount = 4; // how many frame ruben walk
    }
}

static inline void UpdatePlayer(Player *p, int mapW, int mapH) {
    Vector2 direction = { 0, 0 };

    // Input walk
    if (IsKeyDown(KEY_RIGHT)) { direction.x += 1; p->currentLine = 1; }
    if (IsKeyDown(KEY_LEFT))  { direction.x -= 1; p->currentLine = 2; }
    if (IsKeyDown(KEY_UP))    { direction.y -= 1; p->currentLine = 0; }
    if (IsKeyDown(KEY_DOWN))  { direction.y += 1; p->currentLine = 3; }

    bool isMoving = (direction.x != 0 || direction.y != 0);

    // 2. This is for diagonal walk problem, I have not complete it yet
    if (isMoving) {
        direction = Vector2Normalize(direction);
        
        // this is the speed for walking and run
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            p->currentSpeed = p->runSpeed;
            p->frameSpeed = 0.08f;
        } else {
            p->currentSpeed = p->walkSpeed;
            p->frameSpeed = 0.14f;
        }

        p->pos.x += direction.x * p->currentSpeed;
        p->pos.y += direction.y * p->currentSpeed;

        // And this one is animation
        p->frameTimer += GetFrameTime();
        if (p->frameTimer >= p->frameSpeed) {
            p->frameTimer = 0.0f;
            p->currentFrame++;
            if (p->currentFrame >= 4) p->currentFrame = 0;
        }
    } else {
        // if you are not moving, we use what frame.
        p->currentFrame = 2; 
        p->frameTimer = 0.0f;
    }

    // 4. This is the map border, still prototype
    float hW = p->width / 2.0f;
    float hH = p->height / 2.0f;
    p->pos.x = Clamp(p->pos.x, hW, mapW - hW);
    p->pos.y = Clamp(p->pos.y, hH, mapH - hH);
}

static inline void DrawPlayer(Player *p) {
    if (p->activeTex->id <= 0) return;

    float frameW = (float)p->activeTex->width / 4.0f;
    float frameH = (float)p->activeTex->height / 4.0f;

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