#ifndef SCENE3_H
#define SCENE3_H

#include "raylib.h"
#include <string.h>
#include <stdbool.h>

/*
 * Timing constants used to synchronize the typewriter effect
 * with the narration audio.
 *
 * SCENE3_TEXT_LEAD_SECONDS       : Target amount of time the text should finish
 *                                  before the voice clip ends.
 * SCENE3_SHORT_VOICE_THRESHOLD   : Voice clips shorter than this use a fixed speed.
 * SCENE3_SHORT_VOICE_TEXT_SPEED  : Text reveal speed for short voice clips.
 * SCENE3_MIN_TEXT_DURATION       : Minimum allowed text reveal duration.
 */
#define SCENE3_TEXT_LEAD_SECONDS 3.0f
#define SCENE3_SHORT_VOICE_THRESHOLD 5.0f
#define SCENE3_SHORT_VOICE_TEXT_SPEED 0.03f
#define SCENE3_MIN_TEXT_DURATION 0.15f

/*
 * Runtime states for Scene 3.
 *
 * SCENE3_TEXT_1 : Show the first narrated text block
 * SCENE3_FADE_IN: Fade to black between text sections
 * SCENE3_TEXT_2 : Show the second narrated text block
 * SCENE3_DONE   : Scene has finished
 */
typedef enum {
    SCENE3_TEXT_1,
    SCENE3_FADE_IN,
    SCENE3_TEXT_2,
    SCENE3_DONE
} Scene3State;

/*
 * Runtime data for Scene 3.
 *
 * Fields include:
 * - background texture
 * - current state
 * - narrator name and text content
 * - typewriter animation state
 * - fade transition state
 * - voice clips and their durations
 * - flags to ensure each voice starts only once
 */
typedef struct {
    Texture2D bgTexture;
    Scene3State currentState;

    const char* name;
    const char* text1;
    const char* text2;

    int text1Length;
    int text2Length;

    int charsDrawn;
    float textTimer;
    float textSpeed;
    float fadeAlpha;
    float defaultTextSpeed;

    Sound text1Voice;
    Sound text2Voice;
    float text1Duration;
    float text2Duration;
    bool text1Started;
    bool text2Started;
} Scene3;

/*
 * Load a voice clip and return its duration in seconds.
 *
 * Parameters:
 * - path: Audio file path
 * - outSound: Output sound object
 *
 * Returns:
 * - duration of the loaded clip in seconds
 */
static inline float Scene3LoadVoiceWithDuration(const char* path, Sound* outSound) {
    Wave wave = LoadWave(path);

    float duration = 0.0f;
    if (wave.frameCount > 0 && wave.sampleRate > 0) {
        duration = (float)wave.frameCount / (float)wave.sampleRate;
    }

    *outSound = LoadSoundFromWave(wave);
    UnloadWave(wave);

    return duration;
}

/*
 * Compute the per-character text speed for the typewriter effect.
 *
 * The goal is to align the text reveal with the voice clip duration.
 * Short clips use a fixed speed to avoid overly fast text changes.
 */
static inline float Scene3CalcTextSpeed(const char* text, float voiceDuration, float fallbackSpeed) {
    int units = TextLength(text);
    if (units <= 0) return fallbackSpeed;
    if (voiceDuration <= 0.0f) return fallbackSpeed;

    if (voiceDuration < SCENE3_SHORT_VOICE_THRESHOLD) {
        return SCENE3_SHORT_VOICE_TEXT_SPEED;
    }

    float targetTextDuration = voiceDuration - SCENE3_TEXT_LEAD_SECONDS;
    if (targetTextDuration < SCENE3_MIN_TEXT_DURATION) {
        targetTextDuration = SCENE3_MIN_TEXT_DURATION;
    }

    return targetTextDuration / (float)units;
}

/*
 * Advance the typewriter effect by revealing characters over time.
 *
 * Parameters:
 * - charsDrawn: Current number of visible characters
 * - maxChars: Total number of characters in the text
 * - timer: Accumulated frame timer
 * - charDelay: Delay between character reveals
 */
static inline void Scene3RevealTextSync(int* charsDrawn, int maxChars, float* timer, float charDelay) {
    if (*charsDrawn >= maxChars) return;

    *timer += GetFrameTime();

    if (charDelay <= 0.0f) {
        *charsDrawn = maxChars;
        *timer = 0.0f;
        return;
    }

    while (*timer >= charDelay && *charsDrawn < maxChars) {
        (*charsDrawn)++;
        *timer -= charDelay;
    }
}

/*
 * Start playback and text animation for the first narration block.
 * This function is safe to call multiple times because it runs only once.
 */
static inline void Scene3StartText1(Scene3* scene) {
    if (scene->text1Started) return;

    scene->textSpeed = Scene3CalcTextSpeed(scene->text1, scene->text1Duration, scene->defaultTextSpeed);
    scene->textTimer = 0.0f;
    scene->charsDrawn = 0;

    PlaySound(scene->text1Voice);
    scene->text1Started = true;
}

/*
 * Start playback and text animation for the second narration block.
 * This function is safe to call multiple times because it runs only once.
 */
static inline void Scene3StartText2(Scene3* scene) {
    if (scene->text2Started) return;

    scene->textSpeed = Scene3CalcTextSpeed(scene->text2, scene->text2Duration, scene->defaultTextSpeed);
    scene->textTimer = 0.0f;
    scene->charsDrawn = 0;

    PlaySound(scene->text2Voice);
    scene->text2Started = true;
}

/*
 * Initialize Scene 3 with default assets, text content, timing values,
 * and voice-over data.
 */
static inline void InitScene3(Scene3* scene) {
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene3.jpg");
    scene->currentState = SCENE3_TEXT_1;
    scene->name = "Narrator";

    scene->text1 =
        "Your parents used to say that the only reason people could\n"
        "sleep peacefully at night was because someone, somewhere,\n"
        "was willing to make difficult decisions.";
    scene->text2 = "And that idea stayed with you...";

    scene->text1Length = TextLength(scene->text1);
    scene->text2Length = TextLength(scene->text2);

    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.04f;
    scene->defaultTextSpeed = 0.04f;
    scene->fadeAlpha = 0.0f;

    scene->text1Duration = Scene3LoadVoiceWithDuration("audio/Voice/Scene 3/Narrator part 1.mp3", &scene->text1Voice);
    scene->text2Duration = Scene3LoadVoiceWithDuration("audio/Voice/Scene 3/Narrator part 2.mp3", &scene->text2Voice);
    scene->text1Started = false;
    scene->text2Started = false;
}

/*
 * Update Scene 3 each frame.
 *
 * Scene flow:
 * 1. Reveal the first narration text with synchronized voice
 * 2. Fade to black
 * 3. Reveal the second narration text
 * 4. Mark the scene as complete
 *
 * Player input can:
 * - instantly finish the current text reveal
 * - advance to the next scene state once text is fully shown
 */
static inline void UpdateScene3(Scene3* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    (void)mousePos;
    (void)screenWidth;

    if (scene->currentState == SCENE3_TEXT_1) {
        Scene3StartText1(scene);

        if (scene->charsDrawn < scene->text1Length) {
            Scene3RevealTextSync(&scene->charsDrawn, scene->text1Length, &scene->textTimer, scene->textSpeed);

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->text1Length;
                StopSound(scene->text1Voice);
            }
        } else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                StopSound(scene->text1Voice);
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
            scene->textTimer = 0.0f;
        }
    }
    else if (scene->currentState == SCENE3_TEXT_2) {
        Scene3StartText2(scene);

        if (scene->charsDrawn < scene->text2Length) {
            Scene3RevealTextSync(&scene->charsDrawn, scene->text2Length, &scene->textTimer, scene->textSpeed);

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->text2Length;
                StopSound(scene->text2Voice);
            }
        } else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                StopSound(scene->text2Voice);
                scene->currentState = SCENE3_DONE;
            }
        }
    }
}

/*
 * Draw Scene 3 based on the current state.
 *
 * Visual behavior:
 * - always draw the background
 * - show a dialogue box during the first text section
 * - apply fade overlay during transitions
 * - center the second narration text on screen
 * - show a continue prompt once the second text finishes
 */
static inline void DrawScene3(Scene3* scene, int screenWidth, int screenHeight) {
    DrawTexturePro(
        scene->bgTexture,
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height },
        (Rectangle){ 0, 0, screenWidth, screenHeight },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    if (scene->currentState == SCENE3_TEXT_1 || scene->currentState == SCENE3_FADE_IN) {
        Rectangle chatBox = { 20, 520, (float)(screenWidth - 40), 160 };
        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        Rectangle nameBox = { 20, 480, 200, 40 };
        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(scene->name, (int)nameBox.x + 20, (int)nameBox.y + 10, 24, WHITE);

        DrawText(
            TextSubtext(scene->text1, 0, scene->charsDrawn),
            (int)chatBox.x + 40,
            (int)chatBox.y + 35,
            28,
            RAYWHITE
        );
    }

    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }

    if (scene->currentState == SCENE3_TEXT_2) {
        const char* currentNarText = TextSubtext(scene->text2, 0, scene->charsDrawn);
        int textWidth = MeasureText(currentNarText, 28);

        DrawText(
            currentNarText,
            (screenWidth / 2) - (textWidth / 2),
            screenHeight / 2 - 30,
            28,
            RAYWHITE
        );

        if (scene->charsDrawn >= scene->text2Length) {
            DrawText("PRESS ENTER TO CONTINUE", screenWidth / 2 - 150, screenHeight - 60, 20, LIGHTGRAY);
        }
    }
}

/*
 * Release all audio and texture resources owned by Scene 3.
 * This should be called when the scene is no longer needed.
 */
static inline void UnloadScene3(Scene3* scene) {
    StopSound(scene->text1Voice);
    StopSound(scene->text2Voice);
    UnloadSound(scene->text1Voice);
    UnloadSound(scene->text2Voice);
    UnloadTexture(scene->bgTexture);
}

#endif