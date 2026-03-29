#ifndef SCENE5_H
#define SCENE5_H

#include "raylib.h"
#include <string.h>
#include <stdbool.h>

/*
 * Timing constants used to synchronize the typewriter text effect
 * with the narrator voice clip.
 *
 * SCENE5_TEXT_LEAD_SECONDS : Number of seconds the text should finish
 *                            before the voice clip ends.
 * SCENE5_MIN_TEXT_DURATION : Minimum allowed total duration for text reveal.
 */
#define SCENE5_TEXT_LEAD_SECONDS 2.0f
#define SCENE5_MIN_TEXT_DURATION 0.15f

/*
 * Runtime states for Scene 5.
 *
 * SCENE5_FADE_IN  : Fade from black into the scene
 * SCENE5_TEXT     : Display narrator dialogue with typewriter effect
 * SCENE5_FADE_OUT : Fade back to black after dialogue completes
 * SCENE5_DONE     : Scene has finished
 */
typedef enum {
    SCENE5_FADE_IN,
    SCENE5_TEXT,
    SCENE5_FADE_OUT,
    SCENE5_DONE
} Scene5State;

/*
 * Runtime data for Scene 5.
 *
 * Fields include:
 * - background texture
 * - current scene state
 * - narrator name and dialogue text
 * - typewriter animation state
 * - fade transition state
 * - narrator voice clip and duration
 * - playback flag to ensure narration starts only once
 */
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

    float narratorDuration;
    bool narratorPlayed;
    Sound narratorVoice;
} Scene5;

/*
 * Load a narrator voice clip and calculate its duration in seconds.
 *
 * Parameters:
 * - path: Audio file path
 * - outSound: Output sound handle
 *
 * Returns:
 * - duration of the loaded clip in seconds
 */
static inline float Scene5LoadVoiceWithDuration(const char* path, Sound* outSound) {
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
 * Calculate the per-character reveal speed for the typewriter effect.
 *
 * The goal is to align text progression with the narrator voice duration,
 * while enforcing a minimum readable text duration.
 */
static inline float Scene5CalcTextSpeed(const char* text, float voiceDuration, float fallbackSpeed) {
    int units = TextLength(text);
    if (units <= 0) return fallbackSpeed;
    if (voiceDuration <= 0.0f) return fallbackSpeed;

    float targetTextDuration = voiceDuration - SCENE5_TEXT_LEAD_SECONDS;
    if (targetTextDuration < SCENE5_MIN_TEXT_DURATION) {
        targetTextDuration = SCENE5_MIN_TEXT_DURATION;
    }

    return targetTextDuration / (float)units;
}

/*
 * Advance the typewriter effect by revealing characters over time.
 *
 * Parameters:
 * - charsDrawn: Number of currently visible characters
 * - maxChars: Total number of characters in the dialogue
 * - timer: Accumulated frame timer
 * - charDelay: Delay between each revealed character
 */
static inline void Scene5RevealTextSync(int* charsDrawn, int maxChars, float* timer, float charDelay) {
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
 * Start narrator playback only once.
 * Safe to call every frame because the flag prevents replay.
 */
static inline void Scene5PlayNarratorIfNeeded(Scene5* scene) {
    if (!scene->narratorPlayed) {
        PlaySound(scene->narratorVoice);
        scene->narratorPlayed = true;
    }
}

/*
 * Initialize Scene 5 with default visuals, text, audio,
 * fade state, and typewriter timing.
 */
static inline void InitScene5(Scene5* scene) {
    scene->bgTexture = LoadTexture("images/Background/Scene/Scene5.jpg");

    scene->currentState = SCENE5_FADE_IN;
    scene->name = "Narrator";

    scene->dialogue =
        "At the age of nineteen, you were recruited. The Special Force saw\n"
        "potential in you... Calm under pressure, loyal, and capable of\n"
        "making decisions without hesitation.";

    scene->dialogueLength = TextLength(scene->dialogue);
    scene->charsDrawn = 0;
    scene->textTimer = 0.0f;
    scene->textSpeed = 0.04f;
    scene->fadeAlpha = 1.0f;

    scene->narratorDuration =
        Scene5LoadVoiceWithDuration("audio/Voice/Scene 5/Narrator.mp3", &scene->narratorVoice);

    scene->narratorPlayed = false;
    scene->textSpeed =
        Scene5CalcTextSpeed(scene->dialogue, scene->narratorDuration, scene->textSpeed);
}

/*
 * Update Scene 5 each frame.
 *
 * Scene flow:
 * 1. Fade in from black
 * 2. Reveal narrator dialogue with synchronized audio
 * 3. Fade out to black
 * 4. Mark the scene as complete
 *
 * Player input can:
 * - instantly complete the current text reveal
 * - advance to the next state once the full dialogue is shown
 */
static inline void UpdateScene5(Scene5* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    (void)mousePos;
    (void)screenWidth;

    if (scene->currentState == SCENE5_FADE_IN) {
        scene->fadeAlpha -= GetFrameTime() * 0.7f;
        if (scene->fadeAlpha <= 0.0f) {
            scene->fadeAlpha = 0.0f;
            scene->currentState = SCENE5_TEXT;
            Scene5PlayNarratorIfNeeded(scene);
        }
    }
    else if (scene->currentState == SCENE5_TEXT) {
        Scene5PlayNarratorIfNeeded(scene);

        if (scene->charsDrawn < scene->dialogueLength) {
            Scene5RevealTextSync(
                &scene->charsDrawn,
                scene->dialogueLength,
                &scene->textTimer,
                scene->textSpeed
            );

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = scene->dialogueLength;
                StopSound(scene->narratorVoice);
            }
        }
        else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                StopSound(scene->narratorVoice);
                scene->currentState = SCENE5_FADE_OUT;
            }
        }
    }
    else if (scene->currentState == SCENE5_FADE_OUT) {
        scene->fadeAlpha += GetFrameTime() * 0.7f;
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            StopSound(scene->narratorVoice);
            scene->currentState = SCENE5_DONE;
        }
    }
}

/*
 * Draw Scene 5 based on the current state.
 *
 * Visual behavior:
 * - always draw the background
 * - show a narrator dialogue box during the text phase
 * - apply a black fade overlay during transitions
 */
static inline void DrawScene5(Scene5* scene, int screenWidth, int screenHeight) {
    DrawTexturePro(
        scene->bgTexture,
        (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height },
        (Rectangle){ 0, 0, screenWidth, screenHeight },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    if (scene->currentState == SCENE5_TEXT) {
        Rectangle chatBox = { 20, 520, (float)(screenWidth - 40), 160 };
        DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
        DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);

        Rectangle nameBox = { 20, 480, 200, 40 };
        DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
        DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
        DrawText(scene->name, (int)nameBox.x + 20, (int)nameBox.y + 10, 24, WHITE);

        DrawText(
            TextSubtext(scene->dialogue, 0, scene->charsDrawn),
            (int)chatBox.x + 40,
            (int)chatBox.y + 35,
            28,
            RAYWHITE
        );
    }

    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }
}

/*
 * Release all resources owned by Scene 5.
 * This should be called when the scene is no longer needed.
 */
static inline void UnloadScene5(Scene5* scene) {
    StopSound(scene->narratorVoice);
    UnloadSound(scene->narratorVoice);
    UnloadTexture(scene->bgTexture);
}

#endif