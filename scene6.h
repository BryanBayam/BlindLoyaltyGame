#ifndef SCENE6_H
#define SCENE6_H

#include "raylib.h"
#include <string.h>

// Enum to track the sequence of Scene 6
typedef enum {
    SCENE6_FADE_IN,       // Fades in from Scene 5's black screen
    SCENE6_TEXT_1,        // Normal text
    SCENE6_FADE_TO_BLACK, // Fades out to black
    SCENE6_TEXT_2,        // Unskippable text on black screen
    SCENE6_WAIT,          // Waits for input to start the game
    SCENE6_DONE
} Scene6State;

typedef struct {
    Texture2D bgTexture;
    Scene6State currentState;
    
    const char* name;
    const char* text1;
    const char* text2;

    int text1Length;
    int text2Length;

    int charsDrawn;
    float textTimer;
    float textSpeed;
    float fadeAlpha;
} Scene6;

// --- IMPLEMENTATIONS ---

static inline void InitScene6(Scene6* scene) {
    // Load the static background image
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene6.jpg");
    
    scene->currentState = SCENE6_FADE_IN;
    scene->name = "Narrator";
    
    // Line breaks (\n) for the chatbox
    scene->text1 = "Over the years, you became one of their trusted operatives.\nThe missions were clean, efficient, and necessary.";
    scene->text2 = "At least, that's what you were told.";

    scene->text1Length = TextLength(scene->text1);
    scene->text2Length = TextLength(scene->text2);
    
    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.04f; 
    
    // Start completely black to catch the hand-off from Scene 5
    scene->fadeAlpha = 1.0f; 
}

static inline void UpdateScene6(Scene6* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    if (scene->currentState == SCENE6_FADE_IN) {
        scene->fadeAlpha -= GetFrameTime() * 0.7f; 
        if (scene->fadeAlpha <= 0.0f) {
            scene->fadeAlpha = 0.0f;
            scene->currentState = SCENE6_TEXT_1; 
        }
    }
    else if (scene->currentState == SCENE6_TEXT_1) {
        if (scene->charsDrawn < scene->text1Length) {
            scene->textTimer += GetFrameTime();
            if (scene->textTimer >= scene->textSpeed) {
                scene->charsDrawn++;
                scene->textTimer = 0.0f;
            }
            // Skip typing if clicked
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->text1Length;
            }
        } 
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->currentState = SCENE6_FADE_TO_BLACK;
            }
        }
    } 
    else if (scene->currentState == SCENE6_FADE_TO_BLACK) {
        scene->fadeAlpha += GetFrameTime() * 0.7f; 
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            scene->currentState = SCENE6_TEXT_2;
            scene->charsDrawn = 0; 
        }
    }
    else if (scene->currentState == SCENE6_TEXT_2) {
        if (scene->charsDrawn < scene->text2Length) {
            scene->textTimer += GetFrameTime();
            if (scene->textTimer >= scene->textSpeed) {
                scene->charsDrawn++;
                scene->textTimer = 0.0f;
            }
            // ---> UNSKIPPABLE: No Enter/Click logic here! <---
        } 
        else {
            scene->currentState = SCENE6_WAIT; 
        }
    }
    else if (scene->currentState == SCENE6_WAIT) {
        // Now they can press enter to start the game!
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
            scene->currentState = SCENE6_DONE; 
        }
    }
}

static inline void DrawScene6(Scene6* scene, int screenWidth, int screenHeight) {
    // 1. Draw Background
    DrawTexturePro(scene->bgTexture, 
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height }, 
        (Rectangle){ 0, 0, screenWidth, screenHeight }, 
        (Vector2){ 0, 0 }, 0.0f, WHITE);

    // 2. Draw Chatbox (Only during the first text phase)
    if (scene->currentState == SCENE6_TEXT_1 || scene->currentState == SCENE6_FADE_IN || scene->currentState == SCENE6_FADE_TO_BLACK) {
        Rectangle chatBox = { 20, 520, (float)(screenWidth - 40), 160 }; 
        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        Rectangle nameBox = { 20, 480, 200, 40 };
        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(scene->name, nameBox.x + 20, nameBox.y + 10, 24, WHITE);

        DrawText(TextSubtext(scene->text1, 0, scene->charsDrawn), chatBox.x + 40, chatBox.y + 35, 28, RAYWHITE);
    }

    // 3. Draw Black Overlay
    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }

    // 4. Draw Final Text on Black Screen
    if (scene->currentState == SCENE6_TEXT_2 || scene->currentState == SCENE6_WAIT) {
        const char* currentNarText = TextSubtext(scene->text2, 0, scene->charsDrawn);
        int textWidth = MeasureText(currentNarText, 28);
        
        DrawText(currentNarText, (screenWidth / 2) - (textWidth / 2), screenHeight / 2 - 30, 28, RAYWHITE);
        
        if (scene->currentState == SCENE6_WAIT) {
            DrawText("PRESS ENTER TO START MISSION", screenWidth / 2 - 160, screenHeight - 60, 20, LIGHTGRAY);
        }
    }
}

static inline void UnloadScene6(Scene6* scene) {
    UnloadTexture(scene->bgTexture);
}

#endif // SCENE6_H