#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"

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
    int currentLine;
    float frameTimer;
    float frameSpeed;

    float width;
    float height;

    // ---> Stats & UI Variables <---
    float health;
    float maxHealth;
    float energy;
    float maxEnergy;
    
    // ---> NEW: Tracks the 2-second cooldown when energy hits 0 <---
    float exhaustionTimer; 

    Texture2D texUI;
} Player;

static inline void InitPlayer(Player *p, Vector2 startPos) {
    p->pos = startPos;
    p->walkSpeed = 2.5f;
    p->runSpeed = 5.0f;
    p->currentSpeed = p->walkSpeed;

    p->texWalk = LoadTexture("images/Character/Reuben/Reuben_walk.png");
    p->texRun  = LoadTexture("images/Character/Reuben/Reuben_walk.png");
    p->activeTex = &p->texWalk;

    p->frameCount = 4;
    p->currentFrame = 0;
    p->currentLine = 3;
    p->frameTimer = 0.0f;
    p->frameSpeed = 0.12f;

    p->width = 48.0f;
    p->height = 48.0f;

    // ---> Initialize Stats <---
    p->maxHealth = 100.0f;
    p->health = 100.0f;
    p->maxEnergy = 100.0f;
    p->energy = 100.0f;
    
    p->exhaustionTimer = 0.0f; // Start with no exhaustion

    // ---> Load UI Assets <---
    p->texUI = LoadTexture("images/GUI/Reuben_Chat.png");
}

static inline Rectangle GetPlayerHitbox(Vector2 pos) {
    Rectangle hitbox = {
        pos.x - 7.0f,
        pos.y + 14.0f,
        14.0f,
        8.0f
    };
    return hitbox;
}

static inline void UpdatePlayer(Player *p, const Tilemap *map) {
    Vector2 direction = { 0, 0 };

    if (IsKeyDown(KEY_D)) { direction.x += 1; p->currentLine = 1; }
    if (IsKeyDown(KEY_A)) { direction.x -= 1; p->currentLine = 2; }
    if (IsKeyDown(KEY_W)) { direction.y -= 1; p->currentLine = 0; }
    if (IsKeyDown(KEY_S)) { direction.y += 1; p->currentLine = 3; }

    bool isMoving = (direction.x != 0 || direction.y != 0);
    
    // ---> NEW: Decrease the exhaustion timer if it is active <---
    if (p->exhaustionTimer > 0.0f) {
        p->exhaustionTimer -= GetFrameTime();
        if (p->exhaustionTimer < 0.0f) p->exhaustionTimer = 0.0f;
    }

    // Player can only run if they are pressing Shift, actually moving, have energy, AND are NOT exhausted
    bool isRunning = IsKeyDown(KEY_LEFT_SHIFT) && isMoving && (p->energy > 0.0f) && (p->exhaustionTimer <= 0.0f);

    // ---> Energy & Exhaustion Logic <---
    if (isRunning) {
        p->energy -= 20.0f * GetFrameTime(); // Drain 20 energy per second
        
        // If energy hits 0, trigger the 2-second exhaustion penalty!
        if (p->energy <= 0.0f) {
            p->energy = 0.0f;
            p->exhaustionTimer = 2.0f; 
        }
    } else {
        // Only recover energy if the player is NOT exhausted
        if (p->exhaustionTimer <= 0.0f) {
            p->energy += 4.0f * GetFrameTime();  // Recover 4 energy per second
            if (p->energy > p->maxEnergy) p->energy = p->maxEnergy;
        }
    }

    // Health always recovers 4/s
    p->health += 4.0f * GetFrameTime();
    if (p->health > p->maxHealth) p->health = p->maxHealth;

    // Movement Logic
    if (isMoving) {
        direction = Vector2Normalize(direction);

        if (isRunning) {
            p->currentSpeed = p->runSpeed;
            p->frameSpeed = 0.08f;
        } else {
            p->currentSpeed = p->walkSpeed;
            p->frameSpeed = 0.14f;
        }

        Vector2 nextPos = p->pos;
        nextPos.x += direction.x * p->currentSpeed;

        Rectangle hitboxX = GetPlayerHitbox((Vector2){ nextPos.x, p->pos.y });
        if (!CheckMapCollision(map, hitboxX)) {
            p->pos.x = nextPos.x;
        }

        nextPos = p->pos;
        nextPos.y += direction.y * p->currentSpeed;

        Rectangle hitboxY = GetPlayerHitbox((Vector2){ p->pos.x, nextPos.y });
        if (!CheckMapCollision(map, hitboxY)) {
            p->pos.y = nextPos.y;
        }

        p->frameTimer += GetFrameTime();
        if (p->frameTimer >= p->frameSpeed) {
            p->frameTimer = 0.0f;
            p->currentFrame++;
            if (p->currentFrame >= p->frameCount) p->currentFrame = 0;
        }
    } else {
        p->currentFrame = 2;
        p->frameTimer = 0.0f;
    }
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
    Vector2 origin = { p->width / 2.0f, p->height / 2.0f };

    DrawTexturePro(*p->activeTex, sourceRec, destRec, origin, 0.0f, WHITE);
}

// ---> FOOLPROOF UI (Pushed down and sized to fit the grooves) <---
static inline void DrawPlayerUI(Player *p) {
    Vector2 uiPos = { 20.0f, 20.0f }; 
    float uiScale = 2.5f; 

    float panelW = p->texUI.width / 2.0f;
    float panelH = p->texUI.height / 5.0f; 

    // 1. Draw the Base Frame
    Rectangle uiSource = { 0, 0, panelW, panelH }; 
    Rectangle uiDest = { uiPos.x, uiPos.y, panelW * uiScale, panelH * uiScale };
    DrawTexturePro(p->texUI, uiSource, uiDest, (Vector2){0, 0}, 0.0f, WHITE);

    // 2. Exact coordinates
    float barX = uiPos.x + (panelW * 0.31f * uiScale); 
    float barY = uiPos.x + (panelW * 0.33f * uiScale); 
    float barH = panelH * 0.10f * uiScale; 

    // ---> CHANGED: Separated the widths! The wooden frame is slanted, 
    // so the top groove is actually longer than the bottom groove.
    float redBarW  = panelW * 0.55f * uiScale; // Made Health much longer!
    float blueBarW = panelW * 0.46f * uiScale; // Made Energy slightly longer too

    float redY  = uiPos.y + (panelH * 0.30f * uiScale); 
    float blueY = uiPos.y + (panelH * 0.45f * uiScale); 

    // 3. Draw Health (Red)
    float healthRatio = p->health / p->maxHealth;
    DrawRectangle((int)barX, (int)redY, (int)(redBarW * healthRatio), (int)barH, MAROON);
    DrawRectangle((int)barX, (int)redY, (int)(redBarW * healthRatio), (int)(barH / 2.0f), RED);

    // 4. Draw Energy (Blue)
    float energyRatio = p->energy / p->maxEnergy;
    DrawRectangle((int)barY, (int)blueY, (int)(blueBarW * energyRatio), (int)barH, DARKBLUE);
    DrawRectangle((int)barY, (int)blueY, (int)(blueBarW * energyRatio), (int)(barH / 2.0f), BLUE);
}

static inline void UnloadPlayer(Player *p) {
    UnloadTexture(p->texWalk);
    UnloadTexture(p->texRun);
    UnloadTexture(p->texUI);
}

#endif