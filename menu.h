#ifndef MENU_H
#define MENU_H

#include "raylib.h"

typedef struct Menu {
    int selected;
    Texture2D background;
    Font font;
} Menu;

// Return 1: Play, 2: Settings, 3: Quit, 0: Stay
static inline int UpdateMenu(Menu *m, Vector2 vMouse, int vWidth, int vHeight) {
    if (IsKeyPressed(KEY_DOWN)) {
        m->selected++;
        if (m->selected > 2) m->selected = 0;
    }

    if (IsKeyPressed(KEY_UP)) {
        m->selected--;
        if (m->selected < 0) m->selected = 2;
    }

    Rectangle playBtn     = { 760, 240, 260, 50 };
    Rectangle settingsBtn = { 760, 300, 260, 50 };
    Rectangle quitBtn     = { 760, 360, 260, 50 };

    if (CheckCollisionPointRec(vMouse, playBtn)) m->selected = 0;
    if (CheckCollisionPointRec(vMouse, settingsBtn)) m->selected = 1;
    if (CheckCollisionPointRec(vMouse, quitBtn)) m->selected = 2;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER)) {
        if (m->selected == 0) return 1;
        if (m->selected == 1) return 2;
        if (m->selected == 2) return 3;
    }

    return 0;
}

static inline void DrawShadowText(Font font, const char *text, Vector2 pos, float size, float spacing, Color color) {
    DrawTextEx(font, text, (Vector2){ pos.x + 3, pos.y + 3 }, size, spacing, Fade(BLACK, 0.75f));
    DrawTextEx(font, text, pos, size, spacing, color);
}

static inline void DrawMenu(Menu *m, int vWidth, int vHeight) {
    (void)vWidth;
    (void)vHeight;

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

    const char *playText = "PLAY";
    const char *settingsText = "SETTINGS";
    const char *quitText = "QUIT";

    float fontSize = 42.0f;
    float spacing = 2.0f;

    Vector2 playPos = { 780, 240 };
    Vector2 settingsPos = { 780, 300 };
    Vector2 quitPos = { 780, 360 };

    Color normalColor = (Color){ 220, 235, 255, 255 };
    Color selectedColor = YELLOW;

    DrawShadowText(m->font, playText, playPos, fontSize, spacing,
                   (m->selected == 0) ? selectedColor : normalColor);

    DrawShadowText(m->font, settingsText, settingsPos, fontSize, spacing,
                   (m->selected == 1) ? selectedColor : normalColor);

    DrawShadowText(m->font, quitText, quitPos, fontSize, spacing,
                   (m->selected == 2) ? selectedColor : normalColor);

    if (m->selected == 0) {
        DrawShadowText(m->font, ">", (Vector2){ 740, 240 }, fontSize, spacing, YELLOW);
    } else if (m->selected == 1) {
        DrawShadowText(m->font, ">", (Vector2){ 740, 300 }, fontSize, spacing, YELLOW);
    } else if (m->selected == 2) {
        DrawShadowText(m->font, ">", (Vector2){ 740, 360 }, fontSize, spacing, YELLOW);
    }
}

#endif