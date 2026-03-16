#ifndef MENU_H
#define MENU_H
#include "raylib.h"

typedef struct Menu {
    int selected;
    Texture2D background;
} Menu;

// Return 1: Play, 2: Quit, 0: Stay
static inline int UpdateMenu(Menu *m, Vector2 vMouse, int vWidth, int vHeight) {
    if (IsKeyPressed(KEY_DOWN)) m->selected = 1;
    if (IsKeyPressed(KEY_UP)) m->selected = 0;

    Rectangle playBtn = { vWidth/2.0f - 100, 350, 200, 40 };
    Rectangle quitBtn = { vWidth/2.0f - 100, 420, 200, 40 };

    if (CheckCollisionPointRec(vMouse, playBtn)) m->selected = 0;
    if (CheckCollisionPointRec(vMouse, quitBtn)) m->selected = 1;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER)) {
        if (m->selected == 0) return 1;
        if (m->selected == 1) return 2;
    }
    return 0;
}

static inline void DrawMenu(Menu *m, int vWidth, int vHeight) {
    if (m->background.id > 0) {
        DrawTexturePro(m->background, (Rectangle){0,0,(float)m->background.width, (float)m->background.height},
                       (Rectangle){0,0,(float)vWidth, (float)vHeight}, (Vector2){0,0}, 0.0f, WHITE);
    }
    DrawText("BLIND LOYALTY", vWidth/2 - MeasureText("BLIND LOYALTY", 60)/2, 150, 60, RAYWHITE);
    DrawText("PLAY", vWidth/2 - MeasureText("PLAY", 40)/2, 350, 40, (m->selected == 0) ? YELLOW : WHITE);
    DrawText("QUIT", vWidth/2 - MeasureText("QUIT", 40)/2, 420, 40, (m->selected == 1) ? YELLOW : WHITE);
}
#endif