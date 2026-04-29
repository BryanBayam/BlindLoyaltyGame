#ifndef BATTLE_H
#define BATTLE_H

#include "raylib.h"
#include <stdbool.h>
#include "tekkenplayer.h"

// Set up a fighter (Reuben, Ashat Leader, or Commander)
void InitCharacter(Character* c, Vector2 startPos, int charType);
// Audio management for the fight
void LoadBattleSfx(void);
void ResetBattleSfxQueue(void);
void UpdateBattleSfxQueue(void);
bool IsBattleSfxBusy(void);
void UnloadBattleSfx(void);

// Logic and Movement
void UpdateCharacter(Character* c, Character* opponent, float dt, bool isPlayer);
void UpdateCharacterWithSfx(Character* c, Character* opponent, float dt, bool isPlayer, int charType, int opponentType);
bool UpdateBattleEndSequence(Character *player, Character *enemy, float dt);
void ResolveFighterOverlap(Character *a, Character *b); // Keeps fighters from walking through each other

// Visuals and UI
void DrawCharacter(Character* c);
void DrawGameUI(int playerHealth, int enemyHealth, int timeRemaining, int screenWidth, int screenHeight);
void DrawBattleDeathOverlay(int vWidth, int vHeight);
void DrawBattleWinOverlay(int vWidth, int vHeight);

#endif