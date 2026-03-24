#ifndef SCENE1_H
#define SCENE1_H

#include "raylib.h"
#include <string.h>

typedef enum {
    SCENE1_DIALOGUE,
    SCENE1_CHOICE,
    SCENE1_FADE_TO_BLACK,
    SCENE1_NARRATOR,
    SCENE1_DONE
} Scene1State;

typedef struct {
    const char* speakerName;
    const char* text;
    Texture2D* portrait; 
} DialogueLine;


typedef struct {
    Texture2D bgTexture;
    Texture2D commanderPortrait;
    Texture2D reubenPortrait;
    
    // Declare
    float commanderScale; 
    float reubenScale;

    float commanderOffsetX;
    float reubenOffsetX;
    
    Scene1State currentState;
    
    DialogueLine lines[10];
    int totalLines;
    int currentLine;
    
    int charsDrawn;
    float textTimer;
    float textSpeed;
    
    int currentChoice; 
    

    float fadeAlpha;
    

    const char* narratorText;
    int narratorCharsDrawn;

} Scene1;



static inline void InitScene1(Scene1* scene) {
    // Load the image
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene1.png");
    scene->commanderPortrait = LoadTexture("images/Character/Commander/CommanderChat.png");
    scene->reubenPortrait = LoadTexture("images/Character/Reuben/ReubenChat.png");
    
    // Size
    scene->commanderScale = 0.3f; 
    scene->reubenScale = 0.28f;    
    
    scene->commanderOffsetX = 20.0f; 
    scene->reubenOffsetX = 20.0f;    
    
    scene->currentState = SCENE1_DIALOGUE;
    
    // Dialogue
    scene->lines[0] = (DialogueLine){"Commander", "You've got a new mission from the president!", &scene->commanderPortrait};
    scene->lines[1] = (DialogueLine){"Reuben", "What mission commander?", &scene->reubenPortrait};
    scene->lines[2] = (DialogueLine){"Commander", "As usual... You need to eliminate a monster for the government.", &scene->commanderPortrait};
    scene->lines[3] = (DialogueLine){"Reuben", "Could you give me more details about that?", &scene->reubenPortrait};
    scene->lines[4] = (DialogueLine){"Commander", "For the past 2 years, The Bandit Village tribes stole our\narmy's food supplies. But, they move like a shadow.\nNo one can ever touch them.", &scene->commanderPortrait};
    scene->lines[5] = (DialogueLine){"Reuben", "So what's your plan commander?", &scene->reubenPortrait};
    scene->lines[6] = (DialogueLine){"Commander", "You need to find their secret bunker in order to prove that\nthey stole the military supplies. Then we will launch massive\ntroops to capture them. And the media will record it to the world.\nDo you understand your task?", &scene->commanderPortrait};
    
    scene->totalLines = 7;
    scene->currentLine = 0;
    
    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.03f; 
    
    scene->currentChoice = 0;
    
    scene->fadeAlpha = 0.0f;
    
    scene->narratorText = "Before the world called you a hero,\nyou were just a child who believed in order.";
    scene->narratorCharsDrawn = 0;
}

static inline void UpdateScene1(Scene1* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    if (scene->currentState == SCENE1_DIALOGUE) {
        int currentTextLength = TextLength(scene->lines[scene->currentLine].text);
        
        // Effect Typing
        if (scene->charsDrawn < currentTextLength) {
            scene->textTimer += GetFrameTime();
            if (scene->textTimer >= scene->textSpeed) {
                scene->charsDrawn++;
                scene->textTimer = 0.0f;
            }
            if ((IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked)) {
                scene->charsDrawn = currentTextLength;
            }
        } 
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->currentLine++;
                scene->charsDrawn = 0; 
                
                if (scene->currentLine >= scene->totalLines) {
                    scene->currentState = SCENE1_CHOICE;
                }
            }
        }
    } 
    else if (scene->currentState == SCENE1_CHOICE) {
        Rectangle choiceBox1 = { screenWidth / 2.0f - 150, 300, 300, 50 };
        Rectangle choiceBox2 = { screenWidth / 2.0f - 150, 370, 300, 50 };

        if (CheckCollisionPointRec(mousePos, choiceBox1)) {
            scene->currentChoice = 0;
            if (mouseClicked) scene->currentState = SCENE1_FADE_TO_BLACK;
        }
        else if (CheckCollisionPointRec(mousePos, choiceBox2)) {
            scene->currentChoice = 1;
            if (mouseClicked) scene->currentState = SCENE1_FADE_TO_BLACK;
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) scene->currentChoice = 0;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) scene->currentChoice = 1;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            scene->currentState = SCENE1_FADE_TO_BLACK;
        }
    }
    else if (scene->currentState == SCENE1_FADE_TO_BLACK) {
        scene->fadeAlpha += GetFrameTime() * 0.5f; 
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            scene->currentState = SCENE1_NARRATOR;
            scene->charsDrawn = 0; 
        }
    }
    else if (scene->currentState == SCENE1_NARRATOR) {
        int narTextLen = TextLength(scene->narratorText);
        if (scene->narratorCharsDrawn < narTextLen) {
            scene->textTimer += GetFrameTime();
            if (scene->textTimer >= scene->textSpeed) {
                scene->narratorCharsDrawn++;
                scene->textTimer = 0.0f;
            }
        } else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->currentState = SCENE1_DONE; 
            }
        }
    }
}

static inline void DrawScene1(Scene1* scene, int screenWidth, int screenHeight) {

    DrawTexturePro(scene->bgTexture, 
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height }, 
        (Rectangle){ 0, 0, screenWidth, screenHeight }, 
        (Vector2){ 0, 0 }, 0.0f, WHITE);

   
    if (scene->currentState == SCENE1_DIALOGUE || scene->currentState == SCENE1_CHOICE) {
        DialogueLine current = scene->lines[scene->currentLine >= scene->totalLines ? scene->totalLines - 1 : scene->currentLine];
        
        Rectangle chatBox = { 280, 520, (float)(screenWidth - 280 - 20), 160 }; 
        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        Rectangle nameBox = { 280, 480, 200, 40 };
        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(current.speakerName, nameBox.x + 20, nameBox.y + 10, 24, WHITE);

        
        if (current.portrait != NULL) {
            float activeScale = 1.0f;
            float activeOffsetX = 0.0f; 
            
           
            if (current.portrait == &scene->commanderPortrait) {
                activeScale = scene->commanderScale;
                activeOffsetX = scene->commanderOffsetX;
            } else if (current.portrait == &scene->reubenPortrait) {
                activeScale = scene->reubenScale;
                activeOffsetX = scene->reubenOffsetX;
            }

           
            Vector2 portraitPos = { activeOffsetX, screenHeight - (current.portrait->height * activeScale) + 20 };
            DrawTextureEx(*current.portrait, portraitPos, 0.0f, activeScale, WHITE);
        }

        
        if (scene->currentState == SCENE1_DIALOGUE) {
            DrawText(TextSubtext(current.text, 0, scene->charsDrawn), chatBox.x + 40, chatBox.y + 35, 28, RAYWHITE);
        } else {
            DrawText(current.text, chatBox.x + 40, chatBox.y + 35, 28, RAYWHITE);
        }
    }

   
    if (scene->currentState == SCENE1_CHOICE) {
        Rectangle choiceBox1 = { screenWidth / 2.0f - 150, 300, 300, 50 };
        Rectangle choiceBox2 = { screenWidth / 2.0f - 150, 370, 300, 50 };
        
        Color color1 = (scene->currentChoice == 0) ? LIGHTGRAY : DARKGRAY;
        Color color2 = (scene->currentChoice == 1) ? LIGHTGRAY : DARKGRAY;

        DrawRectangleRec(choiceBox1, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx(choiceBox1, 2.0f, color1);
        DrawText("Yes Sir!", choiceBox1.x + 100, choiceBox1.y + 15, 24, (scene->currentChoice == 0) ? WHITE : GRAY);

        DrawRectangleRec(choiceBox2, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx(choiceBox2, 2.0f, color2);
        DrawText("Yes Sir!", choiceBox2.x + 100, choiceBox2.y + 15, 24, (scene->currentChoice == 1) ? WHITE : GRAY);
    }

    
    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }

     
    if (scene->currentState == SCENE1_NARRATOR) {
        const char* currentNarText = TextSubtext(scene->narratorText, 0, scene->narratorCharsDrawn);
        DrawText(currentNarText, screenWidth / 2 - 350, screenHeight / 2 - 30, 28, RAYWHITE);
        
        if (scene->narratorCharsDrawn >= TextLength(scene->narratorText)) {
            DrawText("PRESS ENTER TO CONTINUE", screenWidth / 2 - 150, screenHeight - 60, 20, LIGHTGRAY);
        }
    }
}

static inline void UnloadScene1(Scene1* scene) {
    UnloadTexture(scene->bgTexture);
    UnloadTexture(scene->commanderPortrait);
    UnloadTexture(scene->reubenPortrait);
}

#endif 