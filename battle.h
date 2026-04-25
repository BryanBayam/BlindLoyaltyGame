#ifndef BATTLE_H
#define BATTLE_H

#include "raylib.h"
#include <stdbool.h>
#include "tekkenplayer.h"

void InitCharacter(Character* c, Vector2 startPos, int charType);
void LoadBattleSfx(void);
void ResetBattleSfxQueue(void);
void UpdateBattleSfxQueue(void);
bool IsBattleSfxBusy(void);
void UnloadBattleSfx(void);
void UpdateCharacter(Character* c, Character* opponent, float dt, bool isPlayer);
void UpdateCharacterWithSfx(Character* c, Character* opponent, float dt, bool isPlayer, int charType, int opponentType);
bool UpdateBattleEndSequence(Character *player, Character *enemy, float dt);
void ResolveFighterOverlap(Character *a, Character *b);
void DrawCharacter(Character* c);
void DrawGameUI(int playerHealth, int enemyHealth, int timeRemaining, int screenWidth, int screenHeight);
void DrawBattleDeathOverlay(int vWidth, int vHeight);
void DrawBattleWinOverlay(int vWidth, int vHeight);

#endif // BATTLE_H
