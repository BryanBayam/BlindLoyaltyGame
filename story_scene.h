#ifndef STORY_SCENE_H
#define STORY_SCENE_H

#include "raylib.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SCENE_LINES 50
#define MAX_PORTRAITS 5
#define SCENE_TEXT_LEAD_SECONDS 3.0f
#define SCENE_MIN_TEXT_DURATION 0.15f

/*
 * Portrait configuration used by scene setup data.
 *
 * Fields:
 * - name: Character name used to match dialogue speaker names
 * - imagePath: Portrait texture path
 * - scale: Render scale for the portrait
 * - offsetX: Horizontal draw offset
 */
typedef struct {
    const char* name;
    const char* imagePath;
    float scale;
    float offsetX;
} PortraitData;

/*
 * One dialogue entry loaded from scene data.
 *
 * Fields:
 * - speakerName: Name shown in the dialogue box
 * - text: Dialogue text content
 * - voicePath: Optional voice-over sound path
 */
typedef struct {
    const char* speakerName;
    const char* text;
    const char* voicePath;
} DialogueData;

/*
 * Static scene configuration used to initialize a runtime story scene.
 *
 * This structure stores background settings, portrait definitions,
 * parsed dialogue lines, one optional confirmation button,
 * narrator text, optional narrator voice, and the raw loaded dialogue buffer.
 */
typedef struct {
    const char* bgPath;
    float bgScrollSpeed;
    bool doFadeIn;
    bool doFadeOut;

    PortraitData portraits[MAX_PORTRAITS];
    int portraitCount;

    DialogueData lines[MAX_SCENE_LINES];
    int lineCount;

    /* Optional single confirmation button shown after dialogue ends */
    const char* choiceText;

    const char* narratorText;
    const char* narratorVoicePath;
    const char* endPromptText;

    char* rawDialogueText;
} SceneData;

/*
 * Convert literal "\n" sequences in a string into actual newline characters.
 *
 * This allows dialogue text loaded from files to contain line breaks
 * without requiring multi-line source formatting.
 */
static inline void ProcessLiteralNewlines(char* str) {
    char *read = str;
    char *write = str;

    while (*read) {
        if (read[0] == '\\' && read[1] == 'n') {
            *write++ = '\n';
            read += 2;
        } else {
            *write++ = *read++;
        }
    }

    *write = '\0';
}

/*
 * Load and parse scene dialogue data from a text file.
 *
 * Expected line format:
 *   speaker|text|voicePath
 *
 * Behavior:
 * - ignores empty lines
 * - ignores comment lines starting with '#' or '//'
 * - converts literal "\n" inside dialogue text to real line breaks
 * - stores parsed results inside SceneData.lines
 */
static inline void LoadSceneDialogue(const char* filepath, SceneData* data) {
    data->rawDialogueText = LoadFileText(filepath);
    if (!data->rawDialogueText) {
        data->lineCount = 0;
        return;
    }

    int count = 0;
    char* line = strtok(data->rawDialogueText, "\n");

    while (line != NULL && count < MAX_SCENE_LINES) {
        char* cr = strchr(line, '\r');
        if (cr) *cr = '\0';

        if (strlen(line) < 3 || line[0] == '#' || (line[0] == '/' && line[1] == '/')) {
            line = strtok(NULL, "\n");
            continue;
        }

        char* speaker = line;
        char* text = strchr(speaker, '|');

        if (text != NULL) {
            *text = '\0';
            text++;

            char* voice = strchr(text, '|');
            if (voice != NULL) {
                *voice = '\0';
                voice++;
                data->lines[count].voicePath = voice;
            } else {
                data->lines[count].voicePath = "";
            }

            ProcessLiteralNewlines(text);

            data->lines[count].speakerName = speaker;
            data->lines[count].text = text;
            count++;
        }

        line = strtok(NULL, "\n");
    }

    data->lineCount = count;
}

/*
 * Release the raw dialogue text buffer loaded for the scene.
 */
static inline void UnloadSceneData(SceneData* data) {
    if (data->rawDialogueText) {
        UnloadFileText(data->rawDialogueText);
        data->rawDialogueText = NULL;
    }
}

/*
 * Runtime state used by the story scene state machine.
 */
typedef enum {
    SCENE_STATE_FADE_IN,
    SCENE_STATE_DIALOGUE,
    SCENE_STATE_CHOICE,
    SCENE_STATE_FADE_TO_BLACK,
    SCENE_STATE_NARRATOR,
    SCENE_STATE_FADE_OUT,
    SCENE_STATE_DONE
} SceneStateEnum;

/*
 * One dialogue line prepared for runtime use.
 *
 * Fields:
 * - speakerName: Speaker label shown in the UI
 * - text: Dialogue text
 * - portrait: Optional portrait texture matched by speaker name
 */
typedef struct {
    const char* speakerName;
    const char* text;
    Texture2D* portrait;
} RuntimeDialogueLine;

/*
 * Full runtime state for a story scene.
 *
 * This structure stores:
 * - loaded textures and portraits
 * - parsed dialogue ready for display
 * - audio clips and durations
 * - typing effect state
 * - fade and scene progression state
 * - narrator state
 */
typedef struct {
    Texture2D bgTexture;
    float bgScrollX;
    float bgScrollSpeed;

    Texture2D loadedPortraits[MAX_PORTRAITS];
    PortraitData portraitConfigs[MAX_PORTRAITS];
    int portraitCount;

    RuntimeDialogueLine lines[MAX_SCENE_LINES];
    int totalLines;
    int currentLine;

    /* Optional single confirmation button shown after dialogue */
    const char* choiceText;

    int charsDrawn;
    int narratorCharsDrawn;
    float textTimer;
    float textSpeed;
    float defaultTextSpeed;

    float fadeAlpha;
    bool doFadeOut;
    SceneStateEnum currentState;

    const char* narratorText;
    const char* endPromptText;

    Sound dialogueVoice[MAX_SCENE_LINES];
    float dialogueDuration[MAX_SCENE_LINES];

    Sound narratorVoice;
    float narratorDuration;
    float voiceVolume;

    bool firstDialogueStarted;
    bool narratorStarted;
} StoryScene;

/*
 * Count the number of text units used for the typewriter effect.
 * Always returns at least 1 so division logic stays safe.
 */
static inline int StoryScene_CountTextUnits(const char* text) {
    if (text == NULL) return 1;

    int len = TextLength(text);
    return (len > 0) ? len : 1;
}

/*
 * Load a voice clip and calculate its duration in seconds.
 *
 * Returns 0.0f when no path is provided or no duration can be determined.
 */
static inline float StoryScene_LoadVoiceWithDuration(const char* path, Sound* outSound) {
    if (!path || path[0] == '\0') {
        *outSound = (Sound){ 0 };
        return 0.0f;
    }

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
 * Calculate typewriter speed based on text length and voice duration.
 *
 * The text is timed so that it usually finishes slightly before the
 * voice clip ends, while still respecting a minimum readable duration.
 */
static inline float StoryScene_CalcTextSpeed(const char* text, float voiceDuration, float fallbackSpeed) {
    int units = StoryScene_CountTextUnits(text);
    if (units <= 0 || voiceDuration <= 0.0f) return fallbackSpeed;

    float targetTextDuration = voiceDuration - SCENE_TEXT_LEAD_SECONDS;
    if (targetTextDuration < SCENE_MIN_TEXT_DURATION) {
        targetTextDuration = SCENE_MIN_TEXT_DURATION;
    }

    return targetTextDuration / (float)units;
}

/*
 * Stop all dialogue and narrator voice playback for the scene.
 */
static inline void StoryScene_StopAllVoices(StoryScene* scene) {
    for (int i = 0; i < scene->totalLines; i++) {
        StopSound(scene->dialogueVoice[i]);
    }

    StopSound(scene->narratorVoice);
}

/*
 * Initialize a runtime story scene from static scene configuration data.
 *
 * Responsibilities:
 * - load background and portrait textures
 * - copy dialogue and portrait configuration
 * - match dialogue speakers to portraits
 * - load voice clips and durations
 * - configure fade and text timing defaults
 */
static inline void InitStoryScene(StoryScene* scene, const SceneData* data) {
    memset(scene, 0, sizeof(StoryScene));

    scene->bgTexture = LoadTexture(data->bgPath);
    scene->bgScrollSpeed = data->bgScrollSpeed;
    scene->bgScrollX = 0.0f;

    scene->portraitCount = data->portraitCount;
    for (int i = 0; i < scene->portraitCount; i++) {
        scene->loadedPortraits[i] = LoadTexture(data->portraits[i].imagePath);
        scene->portraitConfigs[i] = data->portraits[i];
    }

    scene->totalLines = data->lineCount;
    scene->voiceVolume = 1.0f;

    for (int i = 0; i < scene->totalLines; i++) {
        scene->lines[i].speakerName = data->lines[i].speakerName;
        scene->lines[i].text = data->lines[i].text;
        scene->lines[i].portrait = NULL;

        for (int p = 0; p < scene->portraitCount; p++) {
            if (strcmp(scene->lines[i].speakerName, scene->portraitConfigs[p].name) == 0) {
                scene->lines[i].portrait = &scene->loadedPortraits[p];
                break;
            }
        }

        scene->dialogueDuration[i] =
            StoryScene_LoadVoiceWithDuration(data->lines[i].voicePath, &scene->dialogueVoice[i]);
        SetSoundVolume(scene->dialogueVoice[i], scene->voiceVolume);
    }

    scene->choiceText = data->choiceText;
    scene->narratorText = data->narratorText;
    scene->endPromptText = data->endPromptText ? data->endPromptText : "PRESS ENTER TO CONTINUE";

    if (data->narratorVoicePath) {
        scene->narratorDuration =
            StoryScene_LoadVoiceWithDuration(data->narratorVoicePath, &scene->narratorVoice);
        SetSoundVolume(scene->narratorVoice, scene->voiceVolume);
    }

    scene->doFadeOut = data->doFadeOut;
    scene->fadeAlpha = data->doFadeIn ? 1.0f : 0.0f;
    scene->currentState = data->doFadeIn ? SCENE_STATE_FADE_IN : SCENE_STATE_DIALOGUE;

    scene->currentLine = 0;
    scene->defaultTextSpeed = 0.03f;
    scene->textSpeed = scene->defaultTextSpeed;
}

/*
 * Update the story scene state machine each frame.
 *
 * Main flow:
 * - fade in
 * - dialogue typing and progression
 * - optional single confirmation button
 * - fade to black
 * - narrator text
 * - optional fade out
 * - done
 */
static inline void UpdateStoryScene(StoryScene* scene, Vector2 mousePos, bool mouseClicked, int screenWidth) {
    if (scene->bgScrollSpeed != 0.0f) {
        scene->bgScrollX += scene->bgScrollSpeed * GetFrameTime();
    }

    if (scene->currentState == SCENE_STATE_FADE_IN) {
        scene->fadeAlpha -= GetFrameTime() * 0.7f;
        if (scene->fadeAlpha <= 0.0f) {
            scene->fadeAlpha = 0.0f;
            scene->currentState = SCENE_STATE_DIALOGUE;
        }
    }
    else if (scene->currentState == SCENE_STATE_DIALOGUE) {
        if (!scene->firstDialogueStarted && scene->totalLines > 0) {
            scene->textTimer = 0.0f;
            scene->textSpeed = StoryScene_CalcTextSpeed(
                scene->lines[0].text,
                scene->dialogueDuration[0],
                scene->defaultTextSpeed
            );
            PlaySound(scene->dialogueVoice[0]);
            scene->firstDialogueStarted = true;
        }

        if (scene->totalLines == 0) {
            scene->currentState = scene->doFadeOut ? SCENE_STATE_FADE_OUT : SCENE_STATE_DONE;
            return;
        }

        int currentTextLength = TextLength(scene->lines[scene->currentLine].text);

        if (scene->charsDrawn < currentTextLength) {
            scene->textTimer += GetFrameTime();

            while (scene->textTimer >= scene->textSpeed && scene->charsDrawn < currentTextLength) {
                scene->charsDrawn++;
                scene->textTimer -= scene->textSpeed;
            }

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->charsDrawn = currentTextLength;
                StopSound(scene->dialogueVoice[scene->currentLine]);
            }
        } else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                int nextLine = scene->currentLine + 1;
                StoryScene_StopAllVoices(scene);

                if (nextLine >= scene->totalLines) {
                    if (scene->choiceText) {
                        scene->currentState = SCENE_STATE_CHOICE;
                    } else if (scene->doFadeOut) {
                        scene->currentState = SCENE_STATE_FADE_OUT;
                    } else if (scene->narratorText) {
                        scene->currentState = SCENE_STATE_FADE_TO_BLACK;
                    } else {
                        scene->currentState = SCENE_STATE_DONE;
                    }
                } else {
                    scene->currentLine = nextLine;
                    scene->charsDrawn = 0;
                    scene->textTimer = 0.0f;
                    scene->textSpeed = StoryScene_CalcTextSpeed(
                        scene->lines[nextLine].text,
                        scene->dialogueDuration[nextLine],
                        scene->defaultTextSpeed
                    );
                    PlaySound(scene->dialogueVoice[nextLine]);
                }
            }
        }
    }
    else if (scene->currentState == SCENE_STATE_CHOICE) {
        Rectangle choiceBox = {
            screenWidth / 2.0f - 150.0f,
            300.0f,
            300.0f,
            50.0f
        };

        if (CheckCollisionPointRec(mousePos, choiceBox) && mouseClicked) {
            StoryScene_StopAllVoices(scene);
            scene->currentState = scene->doFadeOut ? SCENE_STATE_FADE_OUT : SCENE_STATE_FADE_TO_BLACK;
        }

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            StoryScene_StopAllVoices(scene);
            scene->currentState = scene->doFadeOut ? SCENE_STATE_FADE_OUT : SCENE_STATE_FADE_TO_BLACK;
        }
    }
    else if (scene->currentState == SCENE_STATE_FADE_TO_BLACK) {
        scene->fadeAlpha += GetFrameTime() * 0.5f;
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            scene->currentState = SCENE_STATE_NARRATOR;
            scene->charsDrawn = 0;
            scene->narratorCharsDrawn = 0;
            scene->textTimer = 0.0f;
        }
    }
    else if (scene->currentState == SCENE_STATE_FADE_OUT) {
        scene->fadeAlpha += GetFrameTime() * 0.7f;
        if (scene->fadeAlpha >= 1.0f) {
            scene->fadeAlpha = 1.0f;
            scene->currentState = SCENE_STATE_DONE;
        }
    }
    else if (scene->currentState == SCENE_STATE_NARRATOR) {
        if (!scene->narratorStarted) {
            scene->textSpeed = StoryScene_CalcTextSpeed(
                scene->narratorText,
                scene->narratorDuration,
                scene->defaultTextSpeed
            );
            PlaySound(scene->narratorVoice);
            scene->narratorStarted = true;
        }

        int narTextLen = TextLength(scene->narratorText);

        if (scene->narratorCharsDrawn < narTextLen) {
            scene->textTimer += GetFrameTime();

            while (scene->textTimer >= scene->textSpeed && scene->narratorCharsDrawn < narTextLen) {
                scene->narratorCharsDrawn++;
                scene->textTimer -= scene->textSpeed;
            }

            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                scene->narratorCharsDrawn = narTextLen;
                StopSound(scene->narratorVoice);
            }
        } else {
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || mouseClicked) {
                StoryScene_StopAllVoices(scene);
                scene->currentState = scene->doFadeOut ? SCENE_STATE_FADE_OUT : SCENE_STATE_DONE;
            }
        }
    }
}

/*
 * Draw the current visual state of the story scene.
 *
 * Responsibilities:
 * - draw scrolling or static background
 * - draw dialogue UI and active portrait
 * - draw one centered confirmation button when applicable
 * - draw fade overlay
 * - draw narrator text centered on screen
 */
static inline void DrawStoryScene(StoryScene* scene, int screenWidth, int screenHeight) {
    if (scene->bgScrollSpeed != 0.0f) {
        float bgScale = (float)screenHeight / scene->bgTexture.height;
        float scaledWidth = scene->bgTexture.width * bgScale;

        if (scene->bgScrollX <= -scaledWidth || scene->bgScrollX >= scaledWidth) {
            scene->bgScrollX = 0.0f;
        }

        DrawTextureEx(scene->bgTexture, (Vector2){ scene->bgScrollX, 0 }, 0.0f, bgScale, WHITE);

        float secondX = (scene->bgScrollSpeed < 0)
            ? (scene->bgScrollX + scaledWidth)
            : (scene->bgScrollX - scaledWidth);

        DrawTextureEx(scene->bgTexture, (Vector2){ secondX, 0 }, 0.0f, bgScale, WHITE);
    } else {
        DrawTexturePro(
            scene->bgTexture,
            (Rectangle){ 0, 0, (float)scene->bgTexture.width, (float)scene->bgTexture.height },
            (Rectangle){ 0, 0, (float)screenWidth, (float)screenHeight },
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );
    }

    if (scene->currentState == SCENE_STATE_DIALOGUE || scene->currentState == SCENE_STATE_CHOICE) {
        if (scene->totalLines > 0) {
            int safeLineIndex =
                (scene->currentLine >= scene->totalLines) ? (scene->totalLines - 1) : scene->currentLine;

            RuntimeDialogueLine current = scene->lines[safeLineIndex];

            float chatStartX = (scene->portraitCount > 0) ? 280.0f : 20.0f;
            Rectangle chatBox = { chatStartX, 520, (float)(screenWidth - chatStartX - 20), 160 };
            Rectangle nameBox = { chatStartX, 480, 200, 40 };

            DrawRectangleRec(chatBox, Fade(BLACK, 0.7f));
            DrawRectangleLinesEx(chatBox, 4.0f, LIGHTGRAY);
            DrawRectangleRec(nameBox, Fade(BLACK, 0.85f));
            DrawRectangleLinesEx(nameBox, 3.0f, LIGHTGRAY);
            DrawText(current.speakerName, (int)nameBox.x + 20, (int)nameBox.y + 10, 24, WHITE);

            if (current.portrait != NULL) {
                float activeScale = 1.0f;
                float activeOffsetX = 0.0f;

                for (int p = 0; p < scene->portraitCount; p++) {
                    if (current.portrait == &scene->loadedPortraits[p]) {
                        activeScale = scene->portraitConfigs[p].scale;
                        activeOffsetX = scene->portraitConfigs[p].offsetX;
                        break;
                    }
                }

                Vector2 portraitPos = {
                    activeOffsetX,
                    screenHeight - (current.portrait->height * activeScale) + 20
                };

                DrawTextureEx(*current.portrait, portraitPos, 0.0f, activeScale, WHITE);
            }

            const char* textToDraw =
                (scene->currentState == SCENE_STATE_DIALOGUE)
                    ? TextSubtext(current.text, 0, scene->charsDrawn)
                    : current.text;

            DrawText(textToDraw, (int)chatBox.x + 40, (int)chatBox.y + 35, 28, RAYWHITE);
        }
    }

    if (scene->currentState == SCENE_STATE_CHOICE && scene->choiceText) {
        Rectangle choiceBox = {
            screenWidth / 2.0f - 150.0f,
            300.0f,
            300.0f,
            50.0f
        };

        const int fontSize = 24;
        const int textWidth = MeasureText(scene->choiceText, fontSize);
        const int textX = (int)(choiceBox.x + (choiceBox.width - textWidth) * 0.5f);
        const int textY = (int)(choiceBox.y + (choiceBox.height - fontSize) * 0.5f);

        DrawRectangleRec(choiceBox, Fade(BLACK, 0.8f));
        DrawRectangleLinesEx(choiceBox, 2.0f, LIGHTGRAY);
        DrawText(scene->choiceText, textX, textY, fontSize, WHITE);
    }

    if (scene->fadeAlpha > 0.0f) {
        DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, scene->fadeAlpha));
    }

    if (scene->currentState == SCENE_STATE_NARRATOR) {
        const char* textToDraw = TextSubtext(scene->narratorText, 0, scene->narratorCharsDrawn);
        int textWidth = MeasureText(textToDraw, 28);

        DrawText(
            textToDraw,
            (screenWidth / 2) - (textWidth / 2),
            screenHeight / 2 - 30,
            28,
            RAYWHITE
        );

        if (scene->narratorCharsDrawn >= TextLength(scene->narratorText)) {
            int promptWidth = MeasureText(scene->endPromptText, 20);
            DrawText(
                scene->endPromptText,
                (screenWidth / 2) - (promptWidth / 2),
                screenHeight - 60,
                20,
                LIGHTGRAY
            );
        }
    }
}

/*
 * Release all textures and sounds used by the story scene.
 */
static inline void UnloadStoryScene(StoryScene* scene) {
    StoryScene_StopAllVoices(scene);

    UnloadTexture(scene->bgTexture);

    for (int i = 0; i < scene->portraitCount; i++) {
        UnloadTexture(scene->loadedPortraits[i]);
    }

    for (int i = 0; i < scene->totalLines; i++) {
        UnloadSound(scene->dialogueVoice[i]);
    }

    if (scene->narratorVoice.stream.buffer != NULL) {
        UnloadSound(scene->narratorVoice);
    }
}

#endif