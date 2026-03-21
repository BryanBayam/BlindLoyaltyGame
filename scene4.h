#ifndef SCENE4_H
#define SCENE4_H

#include "raylib.h"
#include <string.h>

// Enum to track the sequence of Scene 4
typedef enum {
    SCENE4_FADE_IN,  // <-- NEW: Fades from black to clear
    SCENE4_TEXT,
    SCENE4_FADE_OUT, // Fades to black at the end of the scene
    SCENE4_DONE
} Scene4State;

typedef struct {
    Texture2D bgTexture;
    Scene4State currentState;
    
    const char* name;
    const char* dialogue;

    int dialogueLength;
    int charsDrawn;
    float textTimer;
    float textSpeed;
    float fadeAlpha;
} Scene4;

// --- IMPLEMENTATIONS ---

static inline void InitScene4(Scene4* scene) {
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene4.jpg");
    
    scene->currentState = SCENE4_FADE_IN; // Start with the fade-in
    scene->name = "Narrator";
    
    scene->dialogue = "You were not the strongest student, but you were disciplined. Focused.\nYou believed rules existed for a reason. While others questioned authority,\nyou believed in protecting people, even if it meant doing things that\nothers wouldn't understand.";

    scene->dialogueLength = TextLength(scene->dialogue);
    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.04f; 
    
    // --> IMPORTANT: Start completely black to match the end of Scene 3 <--
    scene->fadeAlpha = 1.0f; 
}

static inline void UpdateScene4(Scene4* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    if (scene->currentState == SCENE4_FADE_IN) {
        // Fade the black screen away to reveal the library!
        scene->fadeAlpha -= GetFrameTime() * 0.7f; 
        if (scene->fadeAlpha <= 0.0f) {
            scene->fadeAlpha = 0.0f;
            scene->currentState = SCENE4_TEXT; 
        }
    }
    else if (scene->currentState == SCENE4_TEXT) {
        if (scene->charsDrawn < scene->dialogueLength) {
            scene->textTimer += GetFrameTime();
            if (scene->textTimer >= scene->textSpeed) {
                scene->charsDrawn++;
                scene->textTimer = 0.0f;
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->dialogueLength;
            }
        } 
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->currentState = SCENE4_FADE_OUT;
            }
        }
    } 
    else if (scene->currentState == SCENE4_FADE_OUT) {
        scene->fadeAlpha += GetFrameTime() * 0.7f; 
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            scene->currentState = SCENE4_DONE; 
        }
    }
}

static inline void DrawScene4(Scene4* scene, int screenWidth, int screenHeight) {
    DrawTexturePro(scene->bgTexture, 
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height }, 
        (Rectangle){ 0, 0, screenWidth, screenHeight }, 
        (Vector2){ 0, 0 }, 0.0f, WHITE);

    // Only draw the chatbox when we are actually reading text
    if (scene->currentState == SCENE4_TEXT) {
        Rectangle chatBox = { 20, 520, (float)(screenWidth - 40), 160 }; 
        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        Rectangle nameBox = { 20, 480, 200, 40 };
        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(scene->name, nameBox.x + 20, nameBox.y + 10, 24, WHITE);

        DrawText(TextSubtext(scene->dialogue, 0, scene->charsDrawn), chatBox.x + 40, chatBox.y + 35, 28, RAYWHITE);
    }

    // Draw the Fade Overlay (handles both the starting fade-in and ending fade-out)
    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }
}

static inline void UnloadScene4(Scene4* scene) {
    UnloadTexture(scene->bgTexture);
}

#endif // SCENE4_H