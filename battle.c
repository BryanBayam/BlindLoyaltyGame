#include "battle.h"
#include <math.h>
#include <stdio.h>

const float GRAVITY = 1200.0f;
const float JUMP_FORCE = -850.0f;
const float GROUND_Y = 680.0f;

#define PLAYER_CROSSUP_JUMP_SPEED 360.0f
#define ENEMY_DODGE_JUMP_SPEED 420.0f
#define AIR_DRIFT_SPEED 220.0f
#define CROSSUP_DISTANCE 180.0f 


#define BATTLE_SFX_VARIANTS 2
#define BATTLE_SFX_QUEUE_MAX 32

#define PLAYER_LIGHT_COOLDOWN 0.42f
#define PLAYER_MEDIUM_COOLDOWN 0.62f
#define PLAYER_HEAVY_COOLDOWN 0.85f
#define PLAYER_DEFENCE_COOLDOWN 0.60f
#define PLAYER_SHIELD_MAX_HOLD 1.35f
#define PLAYER_JUMP_COOLDOWN 0.48f
#define PLAYER_ATTACK_PRESSURE_TIME 1.0f
#define ENEMY_SPACING_CHANCE 24
#define ENEMY_PROACTIVE_JUMP_CHANCE 8

typedef struct CharacterBattleSfx {
    Sound attack1[BATTLE_SFX_VARIANTS];
    Sound attack2[BATTLE_SFX_VARIANTS];
    Sound attack3[BATTLE_SFX_VARIANTS];
    Sound hurt[BATTLE_SFX_VARIANTS];
    Sound defence[BATTLE_SFX_VARIANTS];
    Sound death;
    Sound jump;
    int nextAttack1;
    int nextAttack2;
    int nextAttack3;
    int nextHurt;
    int nextDefence;
} CharacterBattleSfx;

typedef struct BattleSfxEvent {
    Sound *sound;
    Character *target;
    CharState stateToStart;
    bool startAnimation;
} BattleSfxEvent;

static CharacterBattleSfx gBattleSfx[3];
static Sound gReubenWalkSfx;
static BattleSfxEvent gBattleSfxQueue[BATTLE_SFX_QUEUE_MAX];
static int gBattleSfxQueueHead = 0;
static int gBattleSfxQueueCount = 0;
static Sound *gCurrentBattleSfx = NULL;
static bool gBattleSfxLoaded = false;
static float gPlayerActionCooldown = 0.0f;
static float gPlayerShieldHoldTimer = 0.0f;
static float gPlayerAttackPressureTimer = 0.0f;

static Character *gPendingDeathCharacter = NULL;
static int gPendingDeathCharType = 0;
static bool gPendingDeathStarted = false;

static void LoadCharacterBattleSfx(CharacterBattleSfx *sfx, const char *folder) {
    sfx->attack1[0] = LoadSound(TextFormat("%s/attack1_1.mp3", folder));
    sfx->attack1[1] = LoadSound(TextFormat("%s/attack1_2.mp3", folder));
    sfx->attack2[0] = LoadSound(TextFormat("%s/attack2_1.mp3", folder));
    sfx->attack2[1] = LoadSound(TextFormat("%s/attack2_2.mp3", folder));
    sfx->attack3[0] = LoadSound(TextFormat("%s/attack3_1.mp3", folder));
    sfx->attack3[1] = LoadSound(TextFormat("%s/attack3_2.mp3", folder));
    sfx->hurt[0] = LoadSound(TextFormat("%s/hurt_1.mp3", folder));
    sfx->hurt[1] = LoadSound(TextFormat("%s/hurt_2.mp3", folder));
    sfx->defence[0] = LoadSound(TextFormat("%s/defence_1.mp3", folder));
    sfx->defence[1] = LoadSound(TextFormat("%s/defence_2.mp3", folder));
    sfx->death = LoadSound(TextFormat("%s/death.mp3", folder));
    sfx->jump = LoadSound(TextFormat("%s/jump.mp3", folder));
    sfx->nextAttack1 = 0;
    sfx->nextAttack2 = 0;
    sfx->nextAttack3 = 0;
    sfx->nextHurt = 0;
    sfx->nextDefence = 0;
}

static void UnloadCharacterBattleSfx(CharacterBattleSfx *sfx) {
    for (int i = 0; i < BATTLE_SFX_VARIANTS; i++) {
        UnloadSound(sfx->attack1[i]);
        UnloadSound(sfx->attack2[i]);
        UnloadSound(sfx->attack3[i]);
        UnloadSound(sfx->hurt[i]);
        UnloadSound(sfx->defence[i]);
    }
    UnloadSound(sfx->death);
    UnloadSound(sfx->jump);
}

void LoadBattleSfx(void) {
    if (gBattleSfxLoaded) return;
    LoadCharacterBattleSfx(&gBattleSfx[0], "audio/Sfx/Reuben");
    LoadCharacterBattleSfx(&gBattleSfx[1], "audio/Sfx/Ashat Leader");
    LoadCharacterBattleSfx(&gBattleSfx[2], "audio/Sfx/Commander");
    gReubenWalkSfx = LoadSound("audio/Sfx/walk.mp3");
    gBattleSfxLoaded = true;
}

bool IsBattleSfxBusy(void) {
    // Battle voice SFX are allowed to overlap lightly now, so this no longer blocks actions.
    // Cooldowns and animation states are what prevent attack spam.
    (void)gBattleSfxLoaded;
    return false;
}

void ResetBattleSfxQueue(void) {
    if (gCurrentBattleSfx != NULL) StopSound(*gCurrentBattleSfx);
    for (int c = 0; c < 3; c++) {
        for (int i = 0; i < BATTLE_SFX_VARIANTS; i++) {
            StopSound(gBattleSfx[c].attack1[i]);
            StopSound(gBattleSfx[c].attack2[i]);
            StopSound(gBattleSfx[c].attack3[i]);
            StopSound(gBattleSfx[c].hurt[i]);
            StopSound(gBattleSfx[c].defence[i]);
        }
        StopSound(gBattleSfx[c].death);
        StopSound(gBattleSfx[c].jump);
    }
    StopSound(gReubenWalkSfx);
    gBattleSfxQueueHead = 0;
    gBattleSfxQueueCount = 0;
    gCurrentBattleSfx = NULL;
    gPlayerActionCooldown = 0.0f;
    gPlayerShieldHoldTimer = 0.0f;
    gPlayerAttackPressureTimer = 0.0f;
    gPendingDeathCharacter = NULL;
    gPendingDeathCharType = 0;
    gPendingDeathStarted = false;
}

void UnloadBattleSfx(void) {
    if (!gBattleSfxLoaded) return;
    ResetBattleSfxQueue();
    for (int c = 0; c < 3; c++) UnloadCharacterBattleSfx(&gBattleSfx[c]);
    UnloadSound(gReubenWalkSfx);
    gBattleSfxLoaded = false;
}

static int ClampBattleCharType(int charType) {
    return (charType < 0 || charType > 2) ? 0 : charType;
}

static void StartBattleAnimation(Character *target, CharState state) {
    if (target == NULL) return;
    target->state = state;
    target->currentFrame = 0;
    target->frameTimer = 0.0f;
}

static void PlayBattleSfxNow(Sound *sound) {
    if (!gBattleSfxLoaded || sound == NULL || sound->frameCount == 0) return;
    StopSound(gReubenWalkSfx);
    PlaySound(*sound);
    gCurrentBattleSfx = sound;
}

void UpdateBattleSfxQueue(void) {
    if (!gBattleSfxLoaded) return;

    // Play any queued reaction immediately. This keeps animation and SFX responsive,
    // while still allowing attack and hurt/defence sounds to start in a predictable order.
    while (gBattleSfxQueueCount > 0) {
        BattleSfxEvent event = gBattleSfxQueue[gBattleSfxQueueHead];
        gBattleSfxQueueHead = (gBattleSfxQueueHead + 1) % BATTLE_SFX_QUEUE_MAX;
        gBattleSfxQueueCount--;

        if (event.startAnimation) {
            StartBattleAnimation(event.target, event.stateToStart);
        }

        if (event.sound != NULL && event.sound->frameCount > 0) {
            PlaySound(*event.sound);
            gCurrentBattleSfx = event.sound;
        }
    }
}

static bool HasQueuedAnimationForTarget(Character *target) {
    if (target == NULL) return false;
    for (int i = 0; i < gBattleSfxQueueCount; i++) {
        int index = (gBattleSfxQueueHead + i) % BATTLE_SFX_QUEUE_MAX;
        if (gBattleSfxQueue[index].startAnimation && gBattleSfxQueue[index].target == target) return true;
    }
    return false;
}

static Sound *GetAttackSfx(int charType, int attackNumber) {
    CharacterBattleSfx *sfx = &gBattleSfx[ClampBattleCharType(charType)];
    if (attackNumber == 1) {
        Sound *sound = &sfx->attack1[sfx->nextAttack1];
        sfx->nextAttack1 = (sfx->nextAttack1 + 1) % BATTLE_SFX_VARIANTS;
        return sound;
    }
    if (attackNumber == 2) {
        Sound *sound = &sfx->attack2[sfx->nextAttack2];
        sfx->nextAttack2 = (sfx->nextAttack2 + 1) % BATTLE_SFX_VARIANTS;
        return sound;
    }
    Sound *sound = &sfx->attack3[sfx->nextAttack3];
    sfx->nextAttack3 = (sfx->nextAttack3 + 1) % BATTLE_SFX_VARIANTS;
    return sound;
}

static Sound *GetHurtSfxOnly(int charType) {
    CharacterBattleSfx *sfx = &gBattleSfx[ClampBattleCharType(charType)];
    Sound *sound = &sfx->hurt[sfx->nextHurt];
    sfx->nextHurt = (sfx->nextHurt + 1) % BATTLE_SFX_VARIANTS;
    return sound;
}

static Sound *GetDeathSfxOnly(int charType) {
    return &gBattleSfx[ClampBattleCharType(charType)].death;
}

static void ScheduleDeathAfterHit(Character *character, int charType) {
    gPendingDeathCharacter = character;
    gPendingDeathCharType = ClampBattleCharType(charType);
    gPendingDeathStarted = false;
}

static bool IsPendingDeathCharacter(Character *character) {
    return character != NULL && character == gPendingDeathCharacter && !gPendingDeathStarted;
}

static bool IsNonDeathBattleSfxPlaying(void) {
    if (!gBattleSfxLoaded) return false;
    for (int c = 0; c < 3; c++) {
        for (int i = 0; i < BATTLE_SFX_VARIANTS; i++) {
            if (IsSoundPlaying(gBattleSfx[c].attack1[i])) return true;
            if (IsSoundPlaying(gBattleSfx[c].attack2[i])) return true;
            if (IsSoundPlaying(gBattleSfx[c].attack3[i])) return true;
            if (IsSoundPlaying(gBattleSfx[c].hurt[i])) return true;
            if (IsSoundPlaying(gBattleSfx[c].defence[i])) return true;
        }
        if (IsSoundPlaying(gBattleSfx[c].jump)) return true;
    }
    if (IsSoundPlaying(gReubenWalkSfx)) return true;
    return false;
}

static bool IsActionAnimationActive(Character *character) {
    if (character == NULL) return false;
    bool actionState = character->state == STATE_ATTACK_1 || character->state == STATE_ATTACK_2 ||
                       character->state == STATE_ATTACK_3 || character->state == STATE_HURT;
    if (!actionState) return false;
    int lastFrame = character->frameCounts[character->state] - 1;
    return character->currentFrame < lastFrame;
}

static Sound *GetDefenceSfx(int charType) {
    CharacterBattleSfx *sfx = &gBattleSfx[ClampBattleCharType(charType)];
    Sound *sound = &sfx->defence[sfx->nextDefence];
    sfx->nextDefence = (sfx->nextDefence + 1) % BATTLE_SFX_VARIANTS;
    return sound;
}

static Sound *GetJumpSfx(int charType) {
    return &gBattleSfx[ClampBattleCharType(charType)].jump;
}

static bool CanStartNewBattleAction(bool isPlayer) {
    if (isPlayer && gPlayerActionCooldown > 0.0f) return false;
    return true;
}

static int ApplyBattleDamageScaling(int damage, int attackerType, int defenderType) {
    // Commander is a heavier boss, so Reuben damages him slightly slower.
    // Keep a minimum of 1 damage so every clean hit still matters.
    if (attackerType == 0 && defenderType == 2) {
        int scaledDamage = (int)roundf((float)damage * 0.75f);
        return scaledDamage < 1 ? 1 : scaledDamage;
    }
    return damage;
}

static void MoveAwayFromOpponent(Character *character, Character *opponent, float speed, float dt) {
    if (character == NULL || opponent == NULL) return;
    float dir = (character->position.x < opponent->position.x) ? -1.0f : 1.0f;
    character->position.x += dir * speed * dt;
}

static void FaceOpponent(Character *character, Character *opponent) {
    if (character == NULL || opponent == NULL) return;
    character->facingRight = (opponent->position.x >= character->position.x);
}

static float DirectionToward(Character *from, Character *to) {
    if (from == NULL || to == NULL) return 1.0f;
    return (to->position.x >= from->position.x) ? 1.0f : -1.0f;
}

static void StartCrossupJump(Character *jumper, Character *opponent, float horizontalSpeed) {
    if (jumper == NULL) return;
    jumper->velocity.y = JUMP_FORCE;
    jumper->velocity.x = 0.0f;

    if (opponent != NULL) {
        float distance = fabs(jumper->position.x - opponent->position.x);
        if (distance < CROSSUP_DISTANCE) {
            // Jump toward and over the opponent, so the jumper can land behind them.
            jumper->velocity.x = DirectionToward(jumper, opponent) * horizontalSpeed;
        }
    }
}

static void StartImmediateActionWithSfx(Character *character, CharState state, Sound *sound) {
    StartBattleAnimation(character, state);
    PlayBattleSfxNow(sound);
}

static void QueueReactionAnimationWithSfx(Character *character, CharState state, Sound *sound) {
    // Reactions now start immediately so hurt/defence/death animation stays synced with its MP3.
    StartBattleAnimation(character, state);
    PlayBattleSfxNow(sound);
}

static void PlayWalkSfxIfNeeded(CharState previousState, CharState currentState, int charType) {
    if (charType != 0) return;
    bool wasWalking = previousState == STATE_WALK || previousState == STATE_RUN;
    bool isWalking = currentState == STATE_WALK || currentState == STATE_RUN;
    if (!wasWalking && isWalking && !IsSoundPlaying(gReubenWalkSfx)) {
        PlaySound(gReubenWalkSfx);
    }
}

// charType: 0 = Player, 1 = Samurai, 2 = Viking
// charType: 0 = Player, 1 = Samurai, 2 = Viking
void InitCharacter(Character* c, Vector2 startPos, int charType) {
    c->position = startPos;
    c->velocity = (Vector2){ 0, 0 };
    c->state = STATE_IDLE;
    c->facingRight = (charType == 0);
    c->isGrounded = false;
    c->health = 100;
    c->aiTimer = 0.0f;
    c->hasHealed = false; // <-- Added this to reset the heal ability
    
    // Switch load paths
    const char* basePath = "";
    if (charType == 0) basePath = "images/Character/Reuben";
    else if (charType == 2) basePath = "images/Character/Commander";
    else if (charType == 1) basePath = "images/Character/Ashat Leader";

    c->textures[STATE_IDLE] = LoadTexture(TextFormat("%s/Idle.png", basePath));
    c->textures[STATE_WALK] = LoadTexture(TextFormat("%s/Walk.png", basePath));
    c->textures[STATE_RUN] = LoadTexture(TextFormat("%s/Walk.png", basePath));
    c->textures[STATE_JUMP] = LoadTexture(TextFormat("%s/Jump.png", basePath));
    c->textures[STATE_ATTACK_1] = LoadTexture(TextFormat("%s/Attack_1.png", basePath));
    c->textures[STATE_ATTACK_2] = LoadTexture(TextFormat("%s/Attack_2.png", basePath));
    c->textures[STATE_HURT] = LoadTexture(TextFormat("%s/Hurt.png", basePath));
    c->textures[STATE_DEAD] = LoadTexture(TextFormat("%s/Dead.png", basePath));

    if (charType ==1) { // Specific names for the new character set
        c->textures[STATE_SHIELD] = LoadTexture(TextFormat("%s/Defence.png", basePath));
        c->textures[STATE_ATTACK_3] = LoadTexture(TextFormat("%s/Attack_2.png", basePath));
    } else {
        c->textures[STATE_SHIELD] = LoadTexture(TextFormat("%s/Shield.png", basePath));
        c->textures[STATE_ATTACK_3] = LoadTexture(TextFormat("%s/Attack_3.png", basePath));
    }
    
    // Frame Counts mapping
    if (charType == 0) {
        c->frameCounts[STATE_IDLE] = 6;
        c->frameCounts[STATE_WALK] = 6;
        c->frameCounts[STATE_RUN] = 6;
        c->frameCounts[STATE_JUMP] = 10;
        c->frameCounts[STATE_SHIELD] = 2;
        c->frameCounts[STATE_ATTACK_1] = 4;
        c->frameCounts[STATE_ATTACK_2] = 3;
        c->frameCounts[STATE_ATTACK_3] = 4;
        c->frameCounts[STATE_HURT] = 3;
        c->frameCounts[STATE_DEAD] = 3;
    } else if (charType == 1) {
        c->frameCounts[STATE_IDLE] = 5;
        c->frameCounts[STATE_WALK] = 8;
        c->frameCounts[STATE_RUN] = 8;
        c->frameCounts[STATE_JUMP] = 8;
        c->frameCounts[STATE_SHIELD] = 3;
        c->frameCounts[STATE_ATTACK_1] = 4;
        c->frameCounts[STATE_ATTACK_2] = 3;
        c->frameCounts[STATE_ATTACK_3] = 3;
        c->frameCounts[STATE_HURT] = 2;
        c->frameCounts[STATE_DEAD] = 4;
    } else if (charType == 2) {
        c->frameCounts[STATE_IDLE] = 6;
        c->frameCounts[STATE_WALK] = 8;
        c->frameCounts[STATE_RUN] = 8;
        c->frameCounts[STATE_JUMP] = 12;
        c->frameCounts[STATE_SHIELD] = 2;
        c->frameCounts[STATE_ATTACK_1] = 6;
        c->frameCounts[STATE_ATTACK_2] = 4;
        c->frameCounts[STATE_ATTACK_3] = 3;
        c->frameCounts[STATE_HURT] = 2;
        c->frameCounts[STATE_DEAD] = 3;
    }
    
    c->currentFrame = 0;
    c->frameTimer = 0.0f;
    c->frameDuration = 0.1f;
    c->hitBox = (Rectangle){ c->position.x, c->position.y, 50, 100 }; 
}

void UpdateCharacterWithSfx(Character* c, Character* opponent, float dt, bool isPlayer, int charType, int opponentType) {
    CharState previousState = c->state;

    if (isPlayer) {
        if (gPlayerActionCooldown > 0.0f) {
            gPlayerActionCooldown -= dt;
            if (gPlayerActionCooldown < 0.0f) gPlayerActionCooldown = 0.0f;
        }
        if (gPlayerAttackPressureTimer > 0.0f) {
            gPlayerAttackPressureTimer -= dt;
            if (gPlayerAttackPressureTimer < 0.0f) gPlayerAttackPressureTimer = 0.0f;
        }
    }

    if (c->state != STATE_DEAD) {
        c->velocity.y += GRAVITY * dt;
        c->position.y += c->velocity.y * dt;

        if (!c->isGrounded) {
            c->position.x += c->velocity.x * dt;
        }

        if (c->position.y >= GROUND_Y) {
            c->position.y = GROUND_Y;
            c->velocity.y = 0.0f;
            c->velocity.x = 0.0f;
            c->isGrounded = true;
            if (c->state == STATE_JUMP) c->state = STATE_IDLE;
        } else {
            c->isGrounded = false;
        }
    }

    bool waitingForQueuedReaction = HasQueuedAnimationForTarget(c);

    // Let Reuben hold defence longer, then force a short cooldown after release or timeout.
    if (isPlayer && c->state == STATE_SHIELD) {
        if (IsKeyDown(KEY_S) && c->isGrounded && gPlayerShieldHoldTimer > 0.0f) {
            gPlayerShieldHoldTimer -= dt;
            if (gPlayerShieldHoldTimer < 0.0f) gPlayerShieldHoldTimer = 0.0f;
        } else {
            c->state = STATE_IDLE;
            c->currentFrame = 0;
            c->frameTimer = 0.0f;
            gPlayerShieldHoldTimer = 0.0f;
            if (gPlayerActionCooldown < PLAYER_DEFENCE_COOLDOWN) {
                gPlayerActionCooldown = PLAYER_DEFENCE_COOLDOWN;
            }
        }
    }

    // Player Input Logic
    if (!waitingForQueuedReaction && isPlayer && c->state != STATE_DEAD && c->state != STATE_HURT &&
        c->state != STATE_SHIELD &&
        c->state != STATE_ATTACK_1 && c->state != STATE_ATTACK_2 && c->state != STATE_ATTACK_3)
    {
        bool isMoving = false;
        float moveSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 500.0f : 250.0f;
        CharState moveState = IsKeyDown(KEY_LEFT_SHIFT) ? STATE_RUN : STATE_WALK;

        if (IsKeyDown(KEY_D)) {
            c->position.x += moveSpeed * dt;
            if (c->isGrounded) c->state = moveState;
            isMoving = true;
        } else if (IsKeyDown(KEY_A)) {
            c->position.x -= moveSpeed * dt;
            if (c->isGrounded) c->state = moveState;
            isMoving = true;
        }

        if (IsKeyDown(KEY_S) && c->isGrounded && !isMoving && CanStartNewBattleAction(true)) {
            StartImmediateActionWithSfx(c, STATE_SHIELD, GetDefenceSfx(charType));
            gPlayerShieldHoldTimer = PLAYER_SHIELD_MAX_HOLD;
        } else if (!isMoving && c->isGrounded && c->state != STATE_SHIELD) {
            c->state = STATE_IDLE;
        }

        if (IsKeyPressed(KEY_SPACE) && c->isGrounded && CanStartNewBattleAction(true)) {
            StartImmediateActionWithSfx(c, STATE_JUMP, GetJumpSfx(charType));
            StartCrossupJump(c, opponent, PLAYER_CROSSUP_JUMP_SPEED);
            if (fabs(c->position.x - opponent->position.x) >= CROSSUP_DISTANCE) {
                if (IsKeyDown(KEY_D)) c->velocity.x = AIR_DRIFT_SPEED;
                else if (IsKeyDown(KEY_A)) c->velocity.x = -AIR_DRIFT_SPEED;
            }
            gPlayerActionCooldown = PLAYER_JUMP_COOLDOWN;
        }

        bool wantsAttack1 = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        bool wantsAttack2 = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
        bool wantsAttack3 = IsKeyPressed(KEY_E);

        if ((wantsAttack1 || wantsAttack2 || wantsAttack3) && c->isGrounded && CanStartNewBattleAction(true)) {
            int damage = 10;
            int attackNumber = 1;
            CharState attackState = STATE_ATTACK_1;
            float cooldown = PLAYER_LIGHT_COOLDOWN;

            if (wantsAttack1) {
                attackState = STATE_ATTACK_1;
                attackNumber = 1;
                damage = 10;
                cooldown = PLAYER_LIGHT_COOLDOWN;
            } else if (wantsAttack2) {
                attackState = STATE_ATTACK_2;
                attackNumber = 2;
                damage = 15;
                cooldown = PLAYER_MEDIUM_COOLDOWN;
            } else {
                attackState = STATE_ATTACK_3;
                attackNumber = 3;
                damage = 25;
                cooldown = PLAYER_HEAVY_COOLDOWN;
            }

            StartImmediateActionWithSfx(c, attackState, GetAttackSfx(charType, attackNumber));
            gPlayerActionCooldown = cooldown;
            gPlayerAttackPressureTimer = PLAYER_ATTACK_PRESSURE_TIME;
            damage = ApplyBattleDamageScaling(damage, charType, opponentType);

            // Hit detection against Enemy. The reaction animation is queued so it begins with its own SFX.
            if (fabs(c->position.x - opponent->position.x) < 140.0f && opponent->state != STATE_DEAD && opponent->isGrounded && opponent->state != STATE_JUMP && !HasQueuedAnimationForTarget(opponent)) {
                bool blocked = false;

                bool canDefendOrDodge = (opponent->state == STATE_IDLE || opponent->state == STATE_WALK || opponent->state == STATE_RUN || opponent->state == STATE_SHIELD) && opponent->isGrounded;
                bool dodgedByJump = false;
                blocked = opponent->state == STATE_SHIELD;

                if (canDefendOrDodge) {
                    int defenseRoll = GetRandomValue(1, 100);
                    if (defenseRoll <= 32) {
                        dodgedByJump = true;
                    } else if (defenseRoll <= 68) {
                        blocked = true;
                    }
                }

                if (dodgedByJump) {
                    StartImmediateActionWithSfx(opponent, STATE_JUMP, GetJumpSfx(opponentType));
                    StartCrossupJump(opponent, c, ENEMY_DODGE_JUMP_SPEED);
                    opponent->aiTimer = GetRandomValue(10, 17) / 10.0f;
                } else if (blocked) {
                    QueueReactionAnimationWithSfx(opponent, STATE_SHIELD, GetDefenceSfx(opponentType));
                } else {
                    opponent->health -= damage;
                    if (opponent->health <= 0) {
                        opponent->health = 0;
                        QueueReactionAnimationWithSfx(opponent, STATE_HURT, GetHurtSfxOnly(opponentType));
                        ScheduleDeathAfterHit(opponent, opponentType);
                    } else {
                        QueueReactionAnimationWithSfx(opponent, STATE_HURT, GetHurtSfxOnly(opponentType));
                    }
                }
            }
        }
    }
    // ENEMY AI LOGIC (CHASING, RANDOM ATTACKS, FLEEING)
    else if (!waitingForQueuedReaction && !isPlayer && c->state != STATE_DEAD && c->state != STATE_HURT &&
             c->state != STATE_ATTACK_1 && c->state != STATE_ATTACK_2 && c->state != STATE_ATTACK_3)
    {
        c->facingRight = (opponent->position.x > c->position.x);
        float distanceToPlayer = fabs(c->position.x - opponent->position.x);

        c->aiTimer -= dt;

        // --- AI FEATURE: Flee under 20% health, hit border, and heal once ---
        if (c->health < 20 && !c->hasHealed) {
            c->facingRight = (opponent->position.x < c->position.x); // Face away from player
            float moveDir = (opponent->position.x > c->position.x) ? -1.0f : 1.0f;
            c->position.x += moveDir * 320.0f * dt; // Run away quickly
            if (c->isGrounded) c->state = STATE_RUN;

            if (c->position.x <= 55.0f || c->position.x >= 1225.0f) {
                c->health += 40;
                if (c->health > 100) c->health = 100;
                c->hasHealed = true;
            }
        }
        // Enemy spacing is less frequent now: it only backs away sometimes during Reuben's pressure.
        // If Reuben does not attack for about 1 second, this timer reaches 0 and the enemy chases again.
        else if (gPlayerAttackPressureTimer > 0.0f && distanceToPlayer < 220.0f &&
                 c->aiTimer <= 0.0f && GetRandomValue(1, 100) <= ENEMY_SPACING_CHANCE) {
            MoveAwayFromOpponent(c, opponent, 185.0f, dt);
            if (c->isGrounded) c->state = STATE_WALK;
            c->aiTimer = GetRandomValue(4, 7) / 10.0f;
        }
        // --- AI FEATURE: Chase player dynamically ---
        else if (distanceToPlayer > 120.0f) {
            if (distanceToPlayer > 300.0f) {
                c->position.x += (c->facingRight ? 320.0f : -320.0f) * dt;
                if (c->isGrounded) c->state = STATE_RUN;
            } else {
                c->position.x += (c->facingRight ? 220.0f : -220.0f) * dt;
                if (c->isGrounded) c->state = STATE_WALK;
            }
        }
        // If close enough, stop walking, sometimes jump-dodge, then try to attack.
        else {
            if (c->isGrounded && c->state != STATE_IDLE && c->state != STATE_SHIELD) c->state = STATE_IDLE;

            if (c->aiTimer <= 0.0f && c->isGrounded && distanceToPlayer < 170.0f &&
                GetRandomValue(1, 100) <= ENEMY_PROACTIVE_JUMP_CHANCE) {
                StartImmediateActionWithSfx(c, STATE_JUMP, GetJumpSfx(charType));
                StartCrossupJump(c, opponent, ENEMY_DODGE_JUMP_SPEED);
                c->aiTimer = GetRandomValue(7, 11) / 10.0f;
            } else if (c->aiTimer <= 0.0f && opponent->state != STATE_DEAD && opponent->isGrounded && opponent->state != STATE_JUMP && !HasQueuedAnimationForTarget(opponent) && CanStartNewBattleAction(false)) {
                int randAttack = GetRandomValue(1, 3);
                int damage = 10;
                CharState attackState = STATE_ATTACK_1;

                if (randAttack == 1) {
                    attackState = STATE_ATTACK_1;
                    damage = 10;
                } else if (randAttack == 2) {
                    attackState = STATE_ATTACK_2;
                    damage = 15;
                } else {
                    attackState = STATE_ATTACK_3;
                    damage = 25;
                }

                StartImmediateActionWithSfx(c, attackState, GetAttackSfx(charType, randAttack));

                // AI recovery window: shorter than the previous sync update, but still prevents spam.
                c->aiTimer = GetRandomValue(10, 17) / 10.0f;

                if (opponent->state == STATE_SHIELD) {
                    QueueReactionAnimationWithSfx(opponent, STATE_SHIELD, GetDefenceSfx(opponentType));
                } else {
                    opponent->health -= damage;
                    if (opponent->health <= 0) {
                        opponent->health = 0;
                        QueueReactionAnimationWithSfx(opponent, STATE_HURT, GetHurtSfxOnly(opponentType));
                        ScheduleDeathAfterHit(opponent, opponentType);
                    } else {
                        QueueReactionAnimationWithSfx(opponent, STATE_HURT, GetHurtSfxOnly(opponentType));
                    }
                }
            }
        }
    }

    // Screen Boundary Limit
    if (c->position.x < 50.0f) c->position.x = 50.0f;
    if (c->position.x > 1230.0f) c->position.x = 1230.0f;

    // Fighting-game style auto-facing: both fighters keep looking at each other after
    // jumps, attacks, blocks, hurt reactions, and movement.
    FaceOpponent(c, opponent);

    // Animation Loop
    c->frameTimer += dt;
    if (c->frameTimer >= c->frameDuration) {
        c->frameTimer = 0.0f;
        c->currentFrame++;

        if (c->currentFrame >= c->frameCounts[c->state]) {
            if (c->state == STATE_ATTACK_1 || c->state == STATE_ATTACK_2 || c->state == STATE_ATTACK_3 || c->state == STATE_HURT) {
                if (c->health <= 0) {
                    if (IsPendingDeathCharacter(c)) {
                        c->currentFrame = c->frameCounts[c->state] - 1;
                    } else {
                        c->state = STATE_DEAD;
                        c->currentFrame = 0;
                    }
                } else {
                    c->state = STATE_IDLE;
                }
            } else if (c->state == STATE_JUMP) {
                c->currentFrame = c->frameCounts[STATE_JUMP] - 1;
            } else if (c->state == STATE_DEAD) {
                c->currentFrame = c->frameCounts[STATE_DEAD] - 1;
            } else if (c->state == STATE_SHIELD) {
                c->state = STATE_IDLE;
                c->currentFrame = 0;
            } else {
                c->currentFrame = 0;
            }
        }
    }

    PlayWalkSfxIfNeeded(previousState, c->state, charType);
}

static void AdvanceBattleEndAnimation(Character *c, float dt) {
    if (c == NULL) return;
    c->frameTimer += dt;
    if (c->frameTimer < c->frameDuration) return;

    c->frameTimer = 0.0f;
    c->currentFrame++;

    if (c->currentFrame >= c->frameCounts[c->state]) {
        if (c->state == STATE_ATTACK_1 || c->state == STATE_ATTACK_2 || c->state == STATE_ATTACK_3) {
            c->state = STATE_IDLE;
            c->currentFrame = 0;
        } else if (c->state == STATE_HURT) {
            if (IsPendingDeathCharacter(c)) {
                c->currentFrame = c->frameCounts[c->state] - 1;
            } else {
                c->state = STATE_IDLE;
                c->currentFrame = 0;
            }
        } else if (c->state == STATE_DEAD) {
            c->currentFrame = c->frameCounts[STATE_DEAD] - 1;
        } else {
            c->currentFrame = 0;
        }
    }
}

bool UpdateBattleEndSequence(Character *player, Character *enemy, float dt) {
    AdvanceBattleEndAnimation(player, dt);
    AdvanceBattleEndAnimation(enemy, dt);

    if (gPendingDeathCharacter == NULL) {
        return true;
    }

    bool animationsFinished = !IsActionAnimationActive(player) && !IsActionAnimationActive(enemy);

    if (!gPendingDeathStarted && animationsFinished && !IsNonDeathBattleSfxPlaying()) {
        StartBattleAnimation(gPendingDeathCharacter, STATE_DEAD);
        PlayBattleSfxNow(GetDeathSfxOnly(gPendingDeathCharType));
        gPendingDeathStarted = true;
    }

    if (gPendingDeathStarted) {
        bool deathAnimDone = gPendingDeathCharacter->state == STATE_DEAD &&
                             gPendingDeathCharacter->currentFrame >= gPendingDeathCharacter->frameCounts[STATE_DEAD] - 1;
        bool deathSfxDone = !IsSoundPlaying(*GetDeathSfxOnly(gPendingDeathCharType));
        if (deathAnimDone && deathSfxDone) {
            gPendingDeathCharacter = NULL;
            gPendingDeathStarted = false;
            return true;
        }
    }

    return false;
}

void UpdateCharacter(Character* c, Character* opponent, float dt, bool isPlayer) {
    UpdateCharacterWithSfx(c, opponent, dt, isPlayer, isPlayer ? 0 : 1, isPlayer ? 1 : 0);
}

void ResolveFighterOverlap(Character *a, Character *b) {
    if (a == NULL || b == NULL) return;

    const float minDistance = 95.0f;
    float dx = b->position.x - a->position.x;
    float distance = fabsf(dx);

    if (a->isGrounded && b->isGrounded && distance < minDistance) {
        float push = (minDistance - distance) * 0.5f;

        if (dx >= 0.0f) {
            a->position.x -= push;
            b->position.x += push;
        } else {
            a->position.x += push;
            b->position.x -= push;
        }

        if (a->position.x < 50.0f) a->position.x = 50.0f;
        if (a->position.x > 1230.0f) a->position.x = 1230.0f;
        if (b->position.x < 50.0f) b->position.x = 50.0f;
        if (b->position.x > 1230.0f) b->position.x = 1230.0f;
    }

    a->facingRight = (b->position.x >= a->position.x);
    b->facingRight = (a->position.x >= b->position.x);
}

void DrawCharacter(Character* c) {
    Texture2D tex = c->textures[c->state];
    if (tex.id == 0) return; 

    int numFrames = c->frameCounts[c->state];
    float frameWidth = (float)tex.width / numFrames;
    float frameHeight = (float)tex.height;

    Rectangle sourceRec = { 
        (float)c->currentFrame * frameWidth, 0.0f, 
        c->facingRight ? frameWidth : -frameWidth, frameHeight 
    };

    Rectangle destRec = { 
        c->position.x, c->position.y, 
        frameWidth * 3.5f, frameHeight * 3.5f 
    };

    Vector2 origin = { (frameWidth * 3.5f) / 2.0f, frameHeight * 3.5f };

    Color tint = WHITE;
    if (c->state == STATE_HURT) {
        // Red flicker while taking damage, similar to the previous gameplay feedback.
        tint = (((int)(GetTime() * 24.0)) % 2 == 0) ? RED : WHITE;
    }

    DrawTexturePro(tex, sourceRec, destRec, origin, 0.0f, tint);
}

void DrawGameUI(int playerHealth, int enemyHealth, int timeRemaining, int screenWidth, int screenHeight) {
    (void)screenHeight;
    int barWidth = (screenWidth / 2) - 80;
    int barHeight = 40;
    int padding = 30;

    DrawRectangle((screenWidth / 2) - 30, padding, 60, 50, BLACK);
    DrawRectangleLines((screenWidth / 2) - 30, padding, 60, 50, WHITE);
    
    char timeText[5];
    sprintf(timeText, "%02d", timeRemaining);
    DrawText(timeText, (screenWidth / 2) - 15, padding + 15, 20, WHITE);

    float p1HealthPct = (float)playerHealth / 100.0f;
    DrawRectangle(padding, padding + 5, barWidth, barHeight, RED); 
    DrawRectangle(padding + (barWidth * (1.0f - p1HealthPct)), padding + 5, barWidth * p1HealthPct, barHeight, BLUE);
    DrawRectangleLines(padding, padding + 5, barWidth, barHeight, WHITE);

    float p2HealthPct = (float)enemyHealth / 100.0f;
    DrawRectangle(screenWidth / 2 + 50, padding + 5, barWidth, barHeight, RED); 
    DrawRectangle(screenWidth / 2 + 50, padding + 5, barWidth * p2HealthPct, barHeight, BLUE);
    DrawRectangleLines(screenWidth / 2 + 50, padding + 5, barWidth, barHeight, WHITE);

}
// ------------------------------------------


void DrawBattleDeathOverlay(int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    DrawText("YOU DIED", (vWidth - MeasureText("YOU DIED", 64)) / 2, vHeight / 2 - 60, 64, RED);
    DrawText("Press R to Restart", (vWidth - MeasureText("Press R to Restart", 28)) / 2, vHeight / 2 + 20, 28, MAROON);
}

void DrawBattleWinOverlay(int vWidth, int vHeight) {
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    DrawText("VICTORY", (vWidth - MeasureText("VICTORY", 64)) / 2, vHeight / 2 - 60, 64, GOLD);
    DrawText("You won the fight!", (vWidth - MeasureText("You won the fight!", 28)) / 2, vHeight / 2 + 20, 28, YELLOW);
}
