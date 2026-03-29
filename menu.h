#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "save_system.h"
#include <string.h>
#include <stdio.h>

/*
 * Stores configurable player input bindings.
 */
typedef struct {
    int up;
    int down;
    int left;
    int right;
    int run;
} Keybinds;

/*
 * Stores the full menu state used across:
 * - main menu navigation
 * - load/save screens
 * - settings screen
 * - pause menu
 * - save name text input
 */
typedef struct Menu {
    int selected;
    int subSelected;
    bool isAssigningKey;

    /* Save-name input state */
    bool isTypingSave;
    int typingSlot;
    char saveInput[32];
    int letterCount;

    /* Save slot metadata */
    char slotNames[4][32];
    bool slotExists[4];

    /* Load menu sub-action state */
    int activeLoadSlot;

    /* Audio and keybinding settings */
    float masterVolume;
    float musicVolume;
    Keybinds keys;

    /* Menu assets */
    Texture2D background;
    Texture2D settingsBg;
    Texture2D saveBg;
    Font font;
} Menu;

/*
 * Convert a keyboard key code into a readable label for UI display.
 */
static inline const char* GetKeyName(int key) {
    if (key >= KEY_A && key <= KEY_Z) return TextFormat("%c", key);
    if (key == KEY_UP) return "UP ARROW";
    if (key == KEY_DOWN) return "DOWN ARROW";
    if (key == KEY_LEFT) return "LEFT ARROW";
    if (key == KEY_RIGHT) return "RIGHT ARROW";
    if (key == KEY_SPACE) return "SPACE";
    if (key == KEY_LEFT_SHIFT) return "L SHIFT";
    if (key == KEY_RIGHT_SHIFT) return "R SHIFT";
    return "UNKNOWN";
}

/*
 * Draw text with a small shadow behind it to improve readability.
 */
static inline void DrawShadowText(Font font, const char *text, Vector2 pos, float size, float spacing, Color color) {
    DrawTextEx(font, text, (Vector2){ pos.x + 3, pos.y + 3 }, size, spacing, Fade(BLACK, 0.75f));
    DrawTextEx(font, text, pos, size, spacing, color);
}

/*
 * Refresh the display data for all save slots.
 * Existing save files are loaded to read their names;
 * empty slots are marked as unavailable.
 */
static inline void RefreshSaveSlots(Menu *m) {
    for (int i = 0; i < 4; i++) {
        GameSaveData data;
        if (SaveExists(i) && LoadGameData(i, &data)) {
            m->slotExists[i] = true;
            strcpy(m->slotNames[i], data.name);
        } else {
            m->slotExists[i] = false;
            strcpy(m->slotNames[i], "Empty");
        }
    }
}

/*
 * Update the main menu.
 *
 * Returns:
 * 0 = no action
 * 1 = Play
 * 2 = Load Game
 * 3 = Settings
 * 4 = Quit
 */
static inline int UpdateMenu(Menu *m, Vector2 vMouse, int vWidth, int vHeight) {
    if (IsKeyPressed(KEY_DOWN)) { m->selected = (m->selected + 1) % 4; }
    if (IsKeyPressed(KEY_UP)) { m->selected = (m->selected - 1 + 4) % 4; }

    Rectangle btns[4] = {
        { 760, 240, 260, 50 },
        { 760, 300, 260, 50 },
        { 760, 360, 260, 50 },
        { 760, 420, 260, 50 }
    };

    for (int i = 0; i < 4; i++) {
        if (CheckCollisionPointRec(vMouse, btns[i])) {
            m->selected = i;
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER)) {
        return m->selected + 1;
    }

    return 0;
}

/*
 * Draw the main menu background and selectable menu items.
 */
static inline void DrawMenu(Menu *m, int vWidth, int vHeight) {
    if (m->background.id > 0) {
        DrawTexturePro(
            m->background,
            (Rectangle){ 0, 0, (float)m->background.width, (float)m->background.height },
            (Rectangle){ 0, 0, (float)vWidth, (float)vHeight },
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );
    }

    const char *items[4] = { "PLAY", "LOAD GAME", "SETTINGS", "QUIT" };
    float startY = 240.0f;

    for (int i = 0; i < 4; i++) {
        Color color = (m->selected == i) ? YELLOW : (Color){ 220, 235, 255, 255 };
        DrawShadowText(m->font, items[i], (Vector2){ 780, startY + (i * 60) }, 42.0f, 2.0f, color);

        if (m->selected == i) {
            DrawShadowText(m->font, ">", (Vector2){ 740, startY + (i * 60) }, 42.0f, 2.0f, YELLOW);
        }
    }
}

/*
 * Update the load game menu.
 *
 * Behavior:
 * - allows selecting a save slot
 * - opens a sub-menu for Play / Rename / Delete / Cancel
 * - supports renaming an existing save file
 *
 * Returns:
 * 0        = no action
 * 1        = return to previous screen
 * 10..13   = load save slot 0..3
 */
static inline int UpdateLoadMenu(Menu *m, Vector2 vMouse, int vWidth, int vHeight) {
    /*
     * Rename mode:
     * capture typed input for the currently selected save slot.
     */
    if (m->isTypingSave && m->activeLoadSlot != -1) {
        int key = GetCharPressed();

        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (m->letterCount < 31)) {
                m->saveInput[m->letterCount] = (char)key;
                m->saveInput[m->letterCount + 1] = '\0';
                m->letterCount++;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && m->letterCount > 0) {
            m->letterCount--;
            m->saveInput[m->letterCount] = '\0';
        }

        if (IsKeyPressed(KEY_ENTER) && m->letterCount > 0) {
            GameSaveData data;
            if (LoadGameData(m->activeLoadSlot, &data)) {
                strcpy(data.name, m->saveInput);
                SaveGameData(m->activeLoadSlot, data);
                RefreshSaveSlots(m);
            }
            m->isTypingSave = false;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            m->isTypingSave = false;
        }

        return 0;
    }

    /*
     * Slot action sub-menu:
     * displayed after the user selects an existing save slot.
     */
    if (m->activeLoadSlot != -1) {
        if (IsKeyPressed(KEY_DOWN)) m->subSelected = (m->subSelected + 1) % 4;
        if (IsKeyPressed(KEY_UP)) m->subSelected = (m->subSelected - 1 + 4) % 4;

        float subX = vWidth / 2.0f + 150;
        float subY = vHeight / 2.0f - 100 + (m->activeLoadSlot * 50);

        for (int i = 0; i < 4; i++) {
            Rectangle btn = { subX, subY + (i * 35), 150, 30 };
            if (CheckCollisionPointRec(vMouse, btn)) {
                m->subSelected = i;
            }
        }

        if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (m->subSelected == 0) {
                int slotToLoad = m->activeLoadSlot;
                m->activeLoadSlot = -1;
                return 10 + slotToLoad;
            }
            else if (m->subSelected == 1) {
                m->isTypingSave = true;
                strcpy(m->saveInput, m->slotNames[m->activeLoadSlot]);
                m->letterCount = strlen(m->saveInput);
            }
            else if (m->subSelected == 2) {
                remove(TextFormat("save_slot_%d.dat", m->activeLoadSlot));
                RefreshSaveSlots(m);
                m->activeLoadSlot = -1;
                m->subSelected = 0;
            }
            else if (m->subSelected == 3) {
                m->subSelected = m->activeLoadSlot;
                m->activeLoadSlot = -1;
            }
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            m->activeLoadSlot = -1;
            m->subSelected = 0;
        }

        return 0;
    }

    /*
     * Default load-menu navigation between save slots and the Return option.
     */
    if (IsKeyPressed(KEY_DOWN)) m->subSelected = (m->subSelected + 1) % 5;
    if (IsKeyPressed(KEY_UP)) m->subSelected = (m->subSelected - 1 + 5) % 5;

    float startX = vWidth / 2.0f - 120;
    float startY = vHeight / 2.0f - 100;

    for (int i = 0; i < 4; i++) {
        Rectangle slotRec = { startX, startY + (i * 50), 250, 40 };
        if (CheckCollisionPointRec(vMouse, slotRec)) {
            m->subSelected = i;
        }
    }

    Rectangle returnRec = { startX + 60, startY + 230, 150, 40 };
    if (CheckCollisionPointRec(vMouse, returnRec)) {
        m->subSelected = 4;
    }

    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (m->subSelected == 4) return 1;

        if (m->subSelected >= 0 && m->subSelected < 4 && m->slotExists[m->subSelected]) {
            m->activeLoadSlot = m->subSelected;
            m->subSelected = 0;
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) return 1;
    return 0;
}

/*
 * Draw the load game menu and, when needed,
 * its slot action sub-menu or rename input state.
 */
static inline void DrawLoadMenu(Menu *m, int vWidth, int vHeight) {
    DrawMenu(m, vWidth, vHeight);
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.6f));

    if (m->saveBg.id > 0) {
        float scale = 0.7f;
        float newW = m->saveBg.width * scale;
        float newH = m->saveBg.height * scale;
        Rectangle dest = {
            vWidth / 2.0f - newW / 2.0f,
            vHeight / 2.0f - newH / 2.0f,
            newW,
            newH
        };

        DrawTexturePro(
            m->saveBg,
            (Rectangle){ 0, 0, m->saveBg.width, m->saveBg.height },
            dest,
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );
    }

    DrawText("LOAD GAME", vWidth / 2 - MeasureText("LOAD GAME", 36) / 2, vHeight / 2 - 160, 36, RAYWHITE);

    float startX = vWidth / 2.0f - 120;
    float startY = vHeight / 2.0f - 100;

    for (int i = 0; i < 4; i++) {
        Color c = (m->activeLoadSlot == -1 && m->subSelected == i) ? YELLOW : RAYWHITE;

        if (m->activeLoadSlot == i) {
            c = GREEN;
        }

        if (m->slotExists[i]) {
            DrawText(TextFormat("Save %d: %s", i + 1, m->slotNames[i]), startX, startY + (i * 50), 28, c);
        } else {
            DrawText(TextFormat("Save %d: Empty", i + 1), startX, startY + (i * 50), 28, Fade(c, 0.5f));
        }
    }

    Color backC = (m->activeLoadSlot == -1 && m->subSelected == 4) ? RED : RAYWHITE;
    DrawText("Return", startX + 60, startY + 230, 28, backC);

    /*
     * Draw the action sub-menu for the currently selected save slot.
     */
    if (m->activeLoadSlot != -1) {
        float subX = vWidth / 2.0f + 150;
        float subY = vHeight / 2.0f - 100 + (m->activeLoadSlot * 50);

        DrawRectangle(subX - 10, subY - 10, 180, 160, Fade(BLACK, 0.9f));
        DrawRectangleLines(subX - 10, subY - 10, 180, 160, RAYWHITE);

        if (m->isTypingSave) {
            DrawText("Rename:", subX, subY, 20, RAYWHITE);

            if (((int)(GetTime() * 2)) % 2 == 0) {
                DrawText(TextFormat("%s_", m->saveInput), subX, subY + 30, 20, GREEN);
            } else {
                DrawText(TextFormat("%s", m->saveInput), subX, subY + 30, 20, GREEN);
            }

            DrawText("[ENTER]", subX, subY + 120, 16, GRAY);
        } else {
            const char* subOptions[4] = { "Play", "Rename", "Delete", "Cancel" };

            for (int i = 0; i < 4; i++) {
                Color sc = (m->subSelected == i) ? YELLOW : RAYWHITE;
                DrawText(subOptions[i], subX, subY + (i * 35), 24, sc);

                if (m->subSelected == i) {
                    DrawText(">", subX - 15, subY + (i * 35), 24, YELLOW);
                }
            }
        }
    }
}

/*
 * Update the save game screen shown from pause.
 *
 * Returns:
 * 0        = no action
 * 1        = return to pause menu
 * 20..23   = save into slot 0..3
 */
static inline int UpdateSaveMenu(Menu *m, Vector2 vMouse, int vWidth, int vHeight) {
    /*
     * Save-name typing mode.
     */
    if (m->isTypingSave) {
        int key = GetCharPressed();

        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (m->letterCount < 31)) {
                m->saveInput[m->letterCount] = (char)key;
                m->saveInput[m->letterCount + 1] = '\0';
                m->letterCount++;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && m->letterCount > 0) {
            m->letterCount--;
            m->saveInput[m->letterCount] = '\0';
        }

        if (IsKeyPressed(KEY_ENTER) && m->letterCount > 0) {
            m->isTypingSave = false;
            return 20 + m->typingSlot;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            m->isTypingSave = false;
        }

        return 0;
    }

    /*
     * Standard navigation between save slots and Return.
     */
    if (IsKeyPressed(KEY_DOWN)) m->subSelected = (m->subSelected + 1) % 5;
    if (IsKeyPressed(KEY_UP)) m->subSelected = (m->subSelected - 1 + 5) % 5;

    float startX = vWidth / 2.0f - 120;
    float startY = vHeight / 2.0f - 100;

    for (int i = 0; i < 4; i++) {
        Rectangle slotRec = { startX, startY + (i * 50), 250, 40 };
        if (CheckCollisionPointRec(vMouse, slotRec)) {
            m->subSelected = i;
        }
    }

    Rectangle returnRec = { startX + 60, startY + 230, 150, 40 };
    if (CheckCollisionPointRec(vMouse, returnRec)) {
        m->subSelected = 4;
    }

    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (m->subSelected == 4) return 1;

        if (m->subSelected >= 0 && m->subSelected < 4) {
            m->isTypingSave = true;
            m->typingSlot = m->subSelected;
            m->letterCount = 0;
            m->saveInput[0] = '\0';
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) return 1;
    return 0;
}

/*
 * Draw the save menu and active text-entry state.
 */
static inline void DrawSaveMenu(Menu *m, int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.6f));

    if (m->saveBg.id > 0) {
        float scale = 0.7f;
        float newW = m->saveBg.width * scale;
        float newH = m->saveBg.height * scale;
        Rectangle dest = {
            vWidth / 2.0f - newW / 2.0f,
            vHeight / 2.0f - newH / 2.0f,
            newW,
            newH
        };

        DrawTexturePro(
            m->saveBg,
            (Rectangle){ 0, 0, m->saveBg.width, m->saveBg.height },
            dest,
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );
    }

    DrawText("SAVE GAME", vWidth / 2 - MeasureText("SAVE GAME", 36) / 2, vHeight / 2 - 160, 36, RAYWHITE);

    float startX = vWidth / 2.0f - 120;
    float startY = vHeight / 2.0f - 100;

    for (int i = 0; i < 4; i++) {
        Color c = (m->subSelected == i) ? YELLOW : RAYWHITE;

        if (m->isTypingSave && m->typingSlot == i) {
            if (((int)(GetTime() * 2)) % 2 == 0) {
                DrawText(TextFormat("Save %d: %s_", i + 1, m->saveInput), startX, startY + (i * 50), 28, GREEN);
            } else {
                DrawText(TextFormat("Save %d: %s", i + 1, m->saveInput), startX, startY + (i * 50), 28, GREEN);
            }
        } else {
            if (m->slotExists[i]) {
                DrawText(TextFormat("Save %d: %s", i + 1, m->slotNames[i]), startX, startY + (i * 50), 28, c);
            } else {
                DrawText(TextFormat("Save %d: Empty", i + 1), startX, startY + (i * 50), 28, Fade(c, 0.5f));
            }
        }
    }

    if (m->isTypingSave) {
        DrawText("Type name & press ENTER", startX - 25, startY + 230, 24, GREEN);
    } else {
        Color backC = (m->subSelected == 4) ? RED : RAYWHITE;
        DrawText("Return", startX + 60, startY + 230, 28, backC);
    }
}

/*
 * Update the settings screen.
 *
 * Supports:
 * - master volume
 * - music volume
 * - rebinding movement/run keys
 *
 * Returns:
 * 0 = no action
 * 1 = return to previous screen
 */
static inline int UpdateSettingsMenu(Menu *m, Vector2 vMouse, int vWidth, int vHeight) {
    /*
     * Key assignment mode:
     * waits for the next pressed key and stores it in the selected binding.
     */
    if (m->isAssigningKey) {
        int key = GetKeyPressed();

        if (key != 0) {
            if (m->subSelected == 2) m->keys.up = key;
            if (m->subSelected == 3) m->keys.down = key;
            if (m->subSelected == 4) m->keys.left = key;
            if (m->subSelected == 5) m->keys.right = key;
            if (m->subSelected == 6) m->keys.run = key;
            m->isAssigningKey = false;
        }

        return 0;
    }

    if (IsKeyPressed(KEY_DOWN)) m->subSelected = (m->subSelected + 1) % 8;
    if (IsKeyPressed(KEY_UP)) m->subSelected = (m->subSelected - 1 + 8) % 8;

    float startX = vWidth / 2.0f - 180;
    float startY = vHeight / 2.0f - 180;

    for (int i = 0; i < 7; i++) {
        if (CheckCollisionPointRec(vMouse, (Rectangle){ startX, startY + (i * 40), 400, 30 })) {
            m->subSelected = i;
        }
    }

    if (CheckCollisionPointRec(vMouse, (Rectangle){ startX + 130, startY + 300, 150, 40 })) {
        m->subSelected = 7;
    }

    /*
     * Volume controls:
     * arrow keys adjust gradually;
     * mouse click cycles through values.
     */
    if (m->subSelected == 0) {
        if (IsKeyPressed(KEY_RIGHT) && m->masterVolume < 1.0f) m->masterVolume += 0.1f;
        if (IsKeyPressed(KEY_LEFT) && m->masterVolume > 0.0f) m->masterVolume -= 0.1f;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            m->masterVolume += 0.1f;
            if (m->masterVolume > 1.05f) m->masterVolume = 0.0f;
        }
    }

    if (m->subSelected == 1) {
        if (IsKeyPressed(KEY_RIGHT) && m->musicVolume < 1.0f) m->musicVolume += 0.1f;
        if (IsKeyPressed(KEY_LEFT) && m->musicVolume > 0.0f) m->musicVolume -= 0.1f;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            m->musicVolume += 0.1f;
            if (m->musicVolume > 1.05f) m->musicVolume = 0.0f;
        }
    }

    if ((IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) &&
        m->subSelected >= 2 && m->subSelected <= 6) {
        m->isAssigningKey = true;
    }

    if ((IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) &&
        m->subSelected == 7) {
        return 1;
    }

    if (IsKeyPressed(KEY_ESCAPE)) return 1;
    return 0;
}

/*
 * Draw the settings screen, including audio controls
 * and keybinding values.
 */
static inline void DrawSettingsMenu(Menu *m, int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.6f));

    if (m->settingsBg.id > 0) {
        float scale = 1.7f;
        float newW = m->settingsBg.width * scale;
        float newH = m->settingsBg.height * scale;

        DrawTexturePro(
            m->settingsBg,
            (Rectangle){ 0, 0, m->settingsBg.width, m->settingsBg.height },
            (Rectangle){ vWidth / 2.0f - newW / 2.0f, vHeight / 2.0f - newH / 2.0f, newW, newH },
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );
    }

    float startX = vWidth / 2.0f - 180;
    float startY = vHeight / 2.0f - 180;

    DrawText(TextFormat("Master Vol: %d%%", (int)(m->masterVolume * 100)), startX, startY, 24, (m->subSelected == 0) ? YELLOW : RAYWHITE);
    DrawText(TextFormat("Music Vol:  %d%%", (int)(m->musicVolume * 100)), startX, startY + 40, 24, (m->subSelected == 1) ? YELLOW : RAYWHITE);
    DrawText(TextFormat("Move UP:    %s", (m->isAssigningKey && m->subSelected == 2) ? "???" : GetKeyName(m->keys.up)), startX, startY + 80, 24, (m->subSelected == 2) ? YELLOW : RAYWHITE);
    DrawText(TextFormat("Move DOWN:  %s", (m->isAssigningKey && m->subSelected == 3) ? "???" : GetKeyName(m->keys.down)), startX, startY + 120, 24, (m->subSelected == 3) ? YELLOW : RAYWHITE);
    DrawText(TextFormat("Move LEFT:  %s", (m->isAssigningKey && m->subSelected == 4) ? "???" : GetKeyName(m->keys.left)), startX, startY + 160, 24, (m->subSelected == 4) ? YELLOW : RAYWHITE);
    DrawText(TextFormat("Move RIGHT: %s", (m->isAssigningKey && m->subSelected == 5) ? "???" : GetKeyName(m->keys.right)), startX, startY + 200, 24, (m->subSelected == 5) ? YELLOW : RAYWHITE);
    DrawText(TextFormat("Sprint/Run: %s", (m->isAssigningKey && m->subSelected == 6) ? "???" : GetKeyName(m->keys.run)), startX, startY + 240, 24, (m->subSelected == 6) ? YELLOW : RAYWHITE);
    DrawText("Return", startX + 130, startY + 300, 28, (m->subSelected == 7) ? RED : RAYWHITE);

    if (m->isAssigningKey) {
        DrawText("Press any key...", startX + 50, startY + 350, 20, GREEN);
    }
}

/*
 * Update the pause menu.
 *
 * Returns:
 * 0 = no action
 * 1 = Resume
 * 2 = Save Game
 * 3 = Settings
 * 4 = Back to Menu
 */
static inline int UpdatePauseMenu(Menu *m, Vector2 vMouse, int vWidth, int vHeight) {
    if (IsKeyPressed(KEY_DOWN)) m->subSelected = (m->subSelected + 1) % 4;
    if (IsKeyPressed(KEY_UP)) m->subSelected = (m->subSelected - 1 + 4) % 4;

    float startX = vWidth / 2.0f - 110;
    float startY = vHeight / 2.0f - 60;

    for (int i = 0; i < 4; i++) {
        if (CheckCollisionPointRec(vMouse, (Rectangle){ startX, startY + (i * 60), 250, 40 })) {
            m->subSelected = i;
        }
    }

    if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        return m->subSelected + 1;
    }

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_M)) {
        return 1;
    }

    return 0;
}

/*
 * Draw the in-game pause menu.
 */
static inline void DrawPauseMenu(Menu *m, int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.6f));

    if (m->settingsBg.id > 0) {
        float scale = 1.3f;
        float newW = m->settingsBg.width * scale;
        float newH = m->settingsBg.height * scale;

        DrawTexturePro(
            m->settingsBg,
            (Rectangle){ 0, 0, m->settingsBg.width, m->settingsBg.height },
            (Rectangle){ vWidth / 2.0f - newW / 2.0f, vHeight / 2.0f - newH / 2.0f, newW, newH },
            (Vector2){ 0, 0 },
            0.0f,
            WHITE
        );
    }

    DrawText("PAUSED", vWidth / 2 - MeasureText("PAUSED", 40) / 2, vHeight / 2 - 150, 40, RAYWHITE);

    float startX = vWidth / 2.0f - 110;
    float startY = vHeight / 2.0f - 60;
    const char* options[4] = { "Resume", "Save Game", "Settings", "Back to Menu" };

    for (int i = 0; i < 4; i++) {
        Color c = (m->subSelected == i) ? YELLOW : RAYWHITE;
        DrawText(options[i], startX + 20, startY + (i * 60), 32, c);

        if (m->subSelected == i) {
            DrawText(">", startX - 10, startY + (i * 60), 32, YELLOW);
        }
    }
}

#endif