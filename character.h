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

    if (IsKeyDown(KEY_RIGHT)) { direction.x += 1; p->currentLine = 1; }
    if (IsKeyDown(KEY_LEFT))  { direction.x -= 1; p->currentLine = 2; }
    if (IsKeyDown(KEY_UP))    { direction.y -= 1; p->currentLine = 0; }
    if (IsKeyDown(KEY_DOWN))  { direction.y += 1; p->currentLine = 3; }

    bool isMoving = (direction.x != 0 || direction.y != 0);

    if (isMoving) {
        direction = Vector2Normalize(direction);

        if (IsKeyDown(KEY_LEFT_SHIFT)) {
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

    // Debug hitbox:
    // Rectangle hb = GetPlayerHitbox(p->pos);
    // DrawRectangleLinesEx(hb, 1, RED);
}

static inline void UnloadPlayer(Player *p) {
    UnloadTexture(p->texWalk);
    UnloadTexture(p->texRun);
}

#endif