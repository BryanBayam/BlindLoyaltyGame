#ifndef SCENE2_H
#define SCENE2_H

#include "raylib.h"
#include <string.h>

// ---> NEW: Enum to track the sequence of Scene 2 <---
typedef enum {
    SCENE2_FADE_IN,
    SCENE2_TEXT,
    SCENE2_FADE_OUT,
    SCENE2_DONE
} Scene2State;

// Structure to hold our scene's assets and data
typedef struct {
    // Background Panning Data
    Texture2D bgTexture;
    float bgScrollX;     
    float bgScrollSpeed; 
    
    // Scene Assets
    Scene2State currentState; // <-- NEW
    const char* name;
    const char* dialogue;

    // Typewriter Effect Data
    int dialogueLength;
    int charsDrawn;
    float textTimer;
    float textSpeed;
    float fadeAlpha; // <-- NEW
} Scene2;

// --- IMPLEMENTATIONS ---

static inline void InitScene2(Scene2* scene) {
    // Load the static PNG background
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene2.jpg");
    
    // Setup Background Panning
    scene->bgScrollX = 0.0f;
    scene->bgScrollSpeed = -15.0f; 
    
    // Start with a fade-in from black to match the end of Scene 1
    scene->currentState = SCENE2_FADE_IN;
    scene->fadeAlpha = 1.0f; 
    
    scene->name = "Narrator";
    
    scene->dialogue = "Your name is Reuben. You grew up in a city that was constantly afraid.\nNews reports spoke about threats, attacks, and instability.";

    scene->dialogueLength = TextLength(scene->dialogue); 
    scene->charsDrawn = 0;                               
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.04f;                            
}

// ---> CHANGED: Added mousePos, mouseClicked, and screenWidth parameters <---
static inline void UpdateScene2(Scene2* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    // --- 1. PAN BACKGROUND (Always moving!) ---
    scene->bgScrollX += scene->bgScrollSpeed * GetFrameTime();

    // --- 2. STATE MACHINE ---
    if (scene->currentState == SCENE2_FADE_IN) {
        scene->fadeAlpha -= GetFrameTime() * 0.7f; 
        if (scene->fadeAlpha <= 0.0f) {
            scene->fadeAlpha = 0.0f;
            scene->currentState = SCENE2_TEXT; 
        }
    }
    else if (scene->currentState == SCENE2_TEXT) {
        if (scene->charsDrawn < scene->dialogueLength) {
            scene->textTimer += GetFrameTime();
            if (scene->textTimer >= scene->textSpeed) {
                scene->charsDrawn++;
                scene->textTimer = 0.0f; 
            }
            // Skip typing if clicked
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->dialogueLength;
            }
        } 
        else {
            // Proceed to fade out
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->currentState = SCENE2_FADE_OUT;
            }
        }
    }
    else if (scene->currentState == SCENE2_FADE_OUT) {
        scene->fadeAlpha += GetFrameTime() * 0.7f; 
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            scene->currentState = SCENE2_DONE; 
        }
    }
}

static inline void DrawScene2(Scene2* scene, int screenWidth, int screenHeight) {
    // 1. Draw Panning Background
    float bgScale = (float)screenHeight / scene->bgTexture.height;
    float scaledWidth = scene->bgTexture.width * bgScale;

    // Handles infinite loop snapping whether panning left or right
    if (scene->bgScrollX <= -scaledWidth || scene->bgScrollX >= scaledWidth) {
        scene->bgScrollX = 0.0f;
    }

    // Draw main image
    DrawTextureEx(scene->bgTexture, (Vector2){ scene->bgScrollX, 0 }, 0.0f, bgScale, WHITE);
    
    // Draw secondary image to fill the gap
    float secondX = (scene->bgScrollSpeed < 0) ? (scene->bgScrollX + scaledWidth) : (scene->bgScrollX - scaledWidth);
    DrawTextureEx(scene->bgTexture, (Vector2){ secondX, 0 }, 0.0f, bgScale, WHITE);


    // 2. Draw Chatbox (Only when text is active)
    if (scene->currentState == SCENE2_TEXT) {
        Rectangle chatBox = { 20, 520, (float)(screenWidth - 40), 160 }; 
        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        Rectangle nameBox = { 20, 480, 200, 40 };
        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(scene->name, nameBox.x + 20, nameBox.y + 10, 24, WHITE);

        DrawText(TextSubtext(scene->dialogue, 0, scene->charsDrawn), chatBox.x + 40, chatBox.y + 35, 28, RAYWHITE);
    }

    // 3. Draw Fade Overlay
    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }
}

static inline void UnloadScene2(Scene2* scene) {
    UnloadTexture(scene->bgTexture); 
}

#endif // SCENE2_H