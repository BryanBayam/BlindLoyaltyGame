#ifndef CHARACTER_H
#define CHARACTER_H

#include "raylib.h"
#include "raymath.h"
#include "tilemap.h"
#include <stdbool.h>

typedef struct Player {
    Vector2 pos;

    float baseWalkSpeed;
    float baseRunSpeed;
    float currentSpeed;
    float speedMultiplier;

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

    float health;
    float maxHealth;
    float energy;
    float maxEnergy;
    float exhaustionTimer;

    float runDrainPerSecond;
    float energyRegenPerSecond;
    float healthRegenPerSecond;

    float damageSlowMultiplier;
    float damageSlowTimer;
    float damageSlowDuration;

    float hurtBlinkTimer;
    float hurtBlinkDuration;

    Texture2D texUI;

    bool isDead;
} Player;

static inline void InitPlayer(Player *p, Vector2 startPos) {
    p->pos = startPos;

    p->baseWalkSpeed = 1.6f;
    p->baseRunSpeed = 3.0f;
    p->currentSpeed = p->baseWalkSpeed;
    p->speedMultiplier = 1.0f;

    p->texWalk = LoadTexture("images/Character/Reuben/Reuben_walk.png");
    p->texRun  = LoadTexture("images/Character/Reuben/Reuben_walk.png");
    p->activeTex = &p->texWalk;

    p->frameCount = 4;
    p->currentFrame = 0;
    p->currentLine = 3;
    p->frameTimer = 0.0f;
    p->frameSpeed = 0.14f;

    p->width = 48.0f;
    p->height = 48.0f;

    p->maxHealth = 100.0f;
    p->health = 100.0f;

    p->maxEnergy = 100.0f;
    p->energy = 100.0f;
    p->exhaustionTimer = 0.0f;

    p->runDrainPerSecond = 40.0f;
    p->energyRegenPerSecond = 6.0f;
    p->healthRegenPerSecond = 0.4f;

    p->damageSlowMultiplier = 0.80f;
    p->damageSlowTimer = 0.0f;
    p->damageSlowDuration = 1.5f;

    p->hurtBlinkTimer = 0.0f;
    p->hurtBlinkDuration = 0.45f;

    p->texUI = LoadTexture("images/GUI/Reuben_Chat.png");

    p->isDead = false;
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

static inline void DamagePlayer(Player *p, float amount, float slowDuration) {
    if (p->isDead) return;

    p->health -= amount;
    if (p->health <= 0.0f) {
        p->health = 0.0f;
        p->isDead = true;
    }

    p->damageSlowDuration = slowDuration;
    p->damageSlowTimer = slowDuration;
    p->hurtBlinkTimer = p->hurtBlinkDuration;
}

static inline void HealPlayer(Player *p, float amount) {
    p->health += amount;
    if (p->health > p->maxHealth) p->health = p->maxHealth;
}

static inline void AddPlayerEnergy(Player *p, float amount) {
    p->energy += amount;
    if (p->energy > p->maxEnergy) p->energy = p->maxEnergy;
}

static inline void UpdatePlayer(Player *p, const Tilemap *map) {
    if (p->isDead) return;

    float dt = GetFrameTime();
    Vector2 direction = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_D)) { direction.x += 1.0f; p->currentLine = 1; }
    if (IsKeyDown(KEY_A)) { direction.x -= 1.0f; p->currentLine = 2; }
    if (IsKeyDown(KEY_W)) { direction.y -= 1.0f; p->currentLine = 0; }
    if (IsKeyDown(KEY_S)) { direction.y += 1.0f; p->currentLine = 3; }

    bool isMoving = (direction.x != 0.0f || direction.y != 0.0f);

    if (p->exhaustionTimer > 0.0f) {
        p->exhaustionTimer -= dt;
        if (p->exhaustionTimer < 0.0f) p->exhaustionTimer = 0.0f;
    }

    if (p->damageSlowTimer > 0.0f) {
        p->damageSlowTimer -= dt;
        if (p->damageSlowTimer < 0.0f) p->damageSlowTimer = 0.0f;
    }

    if (p->hurtBlinkTimer > 0.0f) {
        p->hurtBlinkTimer -= dt;
        if (p->hurtBlinkTimer < 0.0f) p->hurtBlinkTimer = 0.0f;
    }

    bool isRunning = IsKeyDown(KEY_LEFT_SHIFT) &&
                     isMoving &&
                     (p->energy > 0.0f) &&
                     (p->exhaustionTimer <= 0.0f);

    if (isRunning) {
        p->energy -= p->runDrainPerSecond * dt;
        if (p->energy <= 0.0f) {
            p->energy = 0.0f;
            p->exhaustionTimer = 2.0f;
        }
    } else {
        if (p->exhaustionTimer <= 0.0f) {
            p->energy += p->energyRegenPerSecond * dt;
            if (p->energy > p->maxEnergy) p->energy = p->maxEnergy;
        }
    }

    p->health += p->healthRegenPerSecond * dt;
    if (p->health > p->maxHealth) p->health = p->maxHealth;

    if (p->health <= 0.0f) {
        p->health = 0.0f;
        p->isDead = true;
        return;
    }

    float slowFactor = (p->damageSlowTimer > 0.0f) ? p->damageSlowMultiplier : 1.0f;
    float walkSpeed = p->baseWalkSpeed * p->speedMultiplier * slowFactor;
    float runSpeed = p->baseRunSpeed * p->speedMultiplier * slowFactor;

    if (isMoving) {
        direction = Vector2Normalize(direction);

        if (isRunning) {
            p->currentSpeed = runSpeed;
            p->frameSpeed = 0.10f;
            p->activeTex = &p->texRun;
        } else {
            p->currentSpeed = walkSpeed;
            p->frameSpeed = 0.16f;
            p->activeTex = &p->texWalk;
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

        p->frameTimer += dt;
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

    Color tint = WHITE;
    if (p->hurtBlinkTimer > 0.0f) {
        int blinkPhase = ((int)(p->hurtBlinkTimer * 20.0f)) % 2;
        if (blinkPhase == 0) tint = RED;
    }

    DrawTexturePro(*p->activeTex, sourceRec, destRec, origin, 0.0f, tint);
}

static inline void DrawPlayerUI(Player *p) {
    Vector2 uiPos = { 20.0f, 20.0f };
    float uiScale = 2.5f;

    float panelW = p->texUI.width / 2.0f;
    float panelH = p->texUI.height / 5.0f;

    Rectangle uiSource = { 0, 0, panelW, panelH };
    Rectangle uiDest = { uiPos.x, uiPos.y, panelW * uiScale, panelH * uiScale };
    DrawTexturePro(p->texUI, uiSource, uiDest, (Vector2){ 0, 0 }, 0.0f, WHITE);

    float barX = uiPos.x + (panelW * 0.31f * uiScale);
    float barH = panelH * 0.10f * uiScale;

    float redBarW  = panelW * 0.55f * uiScale;
    float blueBarW = panelW * 0.47f * uiScale;

    float redY  = uiPos.y + (panelH * 0.30f * uiScale);
    float blueY = uiPos.y + (panelH * 0.45f * uiScale);

    float healthRatio = p->health / p->maxHealth;
    if (healthRatio < 0.0f) healthRatio = 0.0f;
    if (healthRatio > 1.0f) healthRatio = 1.0f;

    DrawRectangle((int)barX, (int)redY, (int)(redBarW * healthRatio), (int)barH, MAROON);
    DrawRectangle((int)barX, (int)redY, (int)(redBarW * healthRatio), (int)(barH / 2.0f), RED);

    float energyRatio = p->energy / p->maxEnergy;
    if (energyRatio < 0.0f) energyRatio = 0.0f;
    if (energyRatio > 1.0f) energyRatio = 1.0f;

    DrawRectangle((int)barX, (int)blueY, (int)(blueBarW * energyRatio), (int)barH, DARKBLUE);
    DrawRectangle((int)barX, (int)blueY, (int)(blueBarW * energyRatio), (int)(barH / 2.0f), BLUE);
}

static inline void UnloadPlayer(Player *p) {
    UnloadTexture(p->texWalk);
    UnloadTexture(p->texRun);
    UnloadTexture(p->texUI);
}

#endif