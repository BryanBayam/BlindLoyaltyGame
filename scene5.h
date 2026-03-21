#ifndef SCENE5_H
#define SCENE5_H

#include "raylib.h"
#include <string.h>

// Enum to track the sequence of Scene 5
typedef enum {
    SCENE5_FADE_IN,  // Fades from black to clear
    SCENE5_TEXT,
    SCENE5_FADE_OUT, // Fades to black before gameplay
    SCENE5_DONE
} Scene5State;

typedef struct {
    Texture2D bgTexture;
    Scene5State currentState;
    
    const char* name;
    const char* dialogue;

    int dialogueLength;
    int charsDrawn;
    float textTimer;
    float textSpeed;
    float fadeAlpha;
} Scene5;

// --- IMPLEMENTATIONS ---

static inline void InitScene5(Scene5* scene) {
    // Load the static background image
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene5.jpg");
    
    scene->currentState = SCENE5_FADE_IN; // Start with the fade-in from black
    scene->name = "Narrator";
    
    // Formatted with line breaks (\n) so it safely wraps inside the chatbox
    scene->dialogue = "At the age of nineteen, you were recruited. The Special Force saw\npotential in you... Calm under pressure, loyal, and capable of\nmaking decisions without hesitation.";

    scene->dialogueLength = TextLength(scene->dialogue);
    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.04f; 
    
    // Start completely black to match the end of Scene 4
    scene->fadeAlpha = 1.0f; 
}

static inline void UpdateScene5(Scene5* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    if (scene->currentState == SCENE5_FADE_IN) {
        // Fade the black screen away
        scene->fadeAlpha -= GetFrameTime() * 0.7f; 
        if (scene->fadeAlpha <= 0.0f) {
            scene->fadeAlpha = 0.0f;
            scene->currentState = SCENE5_TEXT; 
        }
    }
    else if (scene->currentState == SCENE5_TEXT) {
        // Typewriter Effect
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
                scene->currentState = SCENE5_FADE_OUT;
            }
        }
    } 
    else if (scene->currentState == SCENE5_FADE_OUT) {
        // Fade to black before handing off to the gameplay loop
        scene->fadeAlpha += GetFrameTime() * 0.7f; 
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            scene->currentState = SCENE5_DONE; 
        }
    }
}

static inline void DrawScene5(Scene5* scene, int screenWidth, int screenHeight) {
    // Draw Background scaled proportionally to fit the screen
    DrawTexturePro(scene->bgTexture, 
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height }, 
        (Rectangle){ 0, 0, screenWidth, screenHeight }, 
        (Vector2){ 0, 0 }, 0.0f, WHITE);

    // Only draw the chatbox when we are actually reading text
    if (scene->currentState == SCENE5_TEXT) {
        Rectangle chatBox = { 20, 520, (float)(screenWidth - 40), 160 }; 
        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        Rectangle nameBox = { 20, 480, 200, 40 };
        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(scene->name, nameBox.x + 20, nameBox.y + 10, 24, WHITE);

        DrawText(TextSubtext(scene->dialogue, 0, scene->charsDrawn), chatBox.x + 40, chatBox.y + 35, 28, RAYWHITE);
    }

    // Draw the Fade Overlay
    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }
}

static inline void UnloadScene5(Scene5* scene) {
    UnloadTexture(scene->bgTexture);
}

#endif // SCENE5_H