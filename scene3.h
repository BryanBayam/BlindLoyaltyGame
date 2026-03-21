#ifndef SCENE3_H
#define SCENE3_H

#include "raylib.h"
#include <string.h>

// Enum to track the sequence of Scene 3
typedef enum {
    SCENE3_TEXT_1,
    SCENE3_FADE_IN,  // Fading to black
    SCENE3_TEXT_2,
    SCENE3_DONE      // We stop here so Scene 4 handles the fade-out!
} Scene3State;

// Structure to hold our scene's assets and data
typedef struct {
    Texture2D bgTexture;
    Scene3State currentState;
    
    const char* name;
    const char* text1;
    const char* text2;

    int text1Length;
    int text2Length;
    
    // Typewriter & Fade Effect Data
    int charsDrawn;
    float textTimer;
    float textSpeed;
    float fadeAlpha;
} Scene3;

// --- IMPLEMENTATIONS ---

static inline void InitScene3(Scene3* scene) {
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene3.jpg");
    scene->currentState = SCENE3_TEXT_1;
    scene->name = "Narrator";
    
    scene->text1 = "Your parents used to say that the only reason people could\nsleep peacefully at night was because someone, somewhere,\nwas willing to make difficult decisions.";
    scene->text2 = "And that idea stayed with you...";

    scene->text1Length = TextLength(scene->text1);
    scene->text2Length = TextLength(scene->text2);
    
    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.04f; 
    scene->fadeAlpha = 0.0f;
}

static inline void UpdateScene3(Scene3* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    if (scene->currentState == SCENE3_TEXT_1) {
        if (scene->charsDrawn < scene->text1Length) {
            scene->textTimer += GetFrameTime();
            if (scene->textTimer >= scene->textSpeed) {
                scene->charsDrawn++;
                scene->textTimer = 0.0f;
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->text1Length;
            }
        } 
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->currentState = SCENE3_FADE_IN;
            }
        }
    } 
    else if (scene->currentState == SCENE3_FADE_IN) {
        scene->fadeAlpha += GetFrameTime() * 0.7f; 
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            scene->currentState = SCENE3_TEXT_2;
            scene->charsDrawn = 0; 
        }
    } 
    else if (scene->currentState == SCENE3_TEXT_2) {
        if (scene->charsDrawn < scene->text2Length) {
            scene->textTimer += GetFrameTime();
            if (scene->textTimer >= scene->textSpeed) {
                scene->charsDrawn++;
                scene->textTimer = 0.0f;
            }
            // ---> CHANGED: Removed the skipping logic here so the player is forced to read it! <---
        } 
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->currentState = SCENE3_DONE; // Hand off to Scene 4 immediately!
            }
        }
    }
}

static inline void DrawScene3(Scene3* scene, int screenWidth, int screenHeight) {
    DrawTexturePro(scene->bgTexture, 
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height }, 
        (Rectangle){ 0, 0, screenWidth, screenHeight }, 
        (Vector2){ 0, 0 }, 0.0f, WHITE);

    if (scene->currentState == SCENE3_TEXT_1 || scene->currentState == SCENE3_FADE_IN) {
        Rectangle chatBox = { 20, 520, (float)(screenWidth - 40), 160 }; 
        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        Rectangle nameBox = { 20, 480, 200, 40 };
        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(scene->name, nameBox.x + 20, nameBox.y + 10, 24, WHITE);

        DrawText(TextSubtext(scene->text1, 0, scene->charsDrawn), chatBox.x + 40, chatBox.y + 35, 28, RAYWHITE);
    }

    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }

    if (scene->currentState == SCENE3_TEXT_2) {
        const char* currentNarText = TextSubtext(scene->text2, 0, scene->charsDrawn);
        int textWidth = MeasureText(currentNarText, 28);
        
        DrawText(currentNarText, (screenWidth / 2) - (textWidth / 2), screenHeight / 2 - 30, 28, RAYWHITE);
        
        if (scene->charsDrawn >= scene->text2Length) {
            DrawText("PRESS ENTER TO CONTINUE", screenWidth / 2 - 150, screenHeight - 60, 20, LIGHTGRAY);
        }
    }
}

static inline void UnloadScene3(Scene3* scene) {
    UnloadTexture(scene->bgTexture);
}

#endif // SCENE3_H