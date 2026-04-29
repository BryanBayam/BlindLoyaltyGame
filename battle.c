#include "battle.h"
#include <math.h>
#include <stdio.h>

// Game Constants
const float GRAVITY = 1200.0f;
const float JUMP_FORCE = -850.0f;
const float GROUND_Y = 680.0f;

// Movement speeds for different maneuvers
#define PLAYER_CROSSUP_JUMP_SPEED 360.0f
#define ENEMY_DODGE_JUMP_SPEED 420.0f
#define AIR_DRIFT_SPEED 220.0f
#define CROSSUP_DISTANCE 180.0f 

// Settings for Sound and Combat Timing
#define BATTLE_SFX_VARIANTS 2  // Every action has 2 different sounds (so it's not repetitive)
#define BATTLE_SFX_QUEUE_MAX 32  // How many sounds can be waiting to play at once

// Cooldowns: How long the player must wait before it again
#define PLAYER_LIGHT_COOLDOWN 0.42f
#define PLAYER_MEDIUM_COOLDOWN 0.62f
#define PLAYER_HEAVY_COOLDOWN 0.85f
#define PLAYER_DEFENCE_COOLDOWN 0.60f
#define PLAYER_SHIELD_MAX_HOLD 1.35f
#define PLAYER_JUMP_COOLDOWN 0.48f
#define PLAYER_ATTACK_PRESSURE_TIME 1.0f
#define ENEMY_SPACING_CHANCE 24
#define ENEMY_PROACTIVE_JUMP_CHANCE 8

// --- Data Structures ---

// Stores all the sounds for a specific character (e.g., Reuben or the Enemy)
typedef struct CharacterBattleSfx {
    Sound attack1[BATTLE_SFX_VARIANTS];
    Sound attack2[BATTLE_SFX_VARIANTS];
    Sound attack3[BATTLE_SFX_VARIANTS];
    Sound hurt[BATTLE_SFX_VARIANTS];
    Sound defence[BATTLE_SFX_VARIANTS];
    Sound death;
    Sound jump;

    // Counters to keep track of which variant (1 or 2) to play next
    int nextAttack1;
    int nextAttack2;
    int nextAttack3;
    int nextHurt;
    int nextDefence;
} CharacterBattleSfx;

// An "Event" combines a sound with a movement/animation change
typedef struct BattleSfxEvent {
    Sound *sound;  // The audio file to play
    Character *target;  // Who is doing the action
    CharState stateToStart;  // The animation state (Idle, Attacking, etc.)
    bool startAnimation;  // Should we change the animation right now?
} BattleSfxEvent;

// --- Global Variables (Game Memory) ---
static CharacterBattleSfx gBattleSfx[3];  // SFX for 3 characters (Player + 2 Enemies)
static Sound gReubenWalkSfx;  // Separate sound for walking
static BattleSfxEvent gBattleSfxQueue[BATTLE_SFX_QUEUE_MAX];  // The waiting line for sounds
static int gBattleSfxQueueHead = 0;   // The front of the line
static int gBattleSfxQueueCount = 0;  // How many items are in line
static Sound *gCurrentBattleSfx = NULL;  // What's playing right now
static bool gBattleSfxLoaded = false;  // Have we loaded files from the disk?

// Timers to track cooldowns in real-time
static float gPlayerActionCooldown = 0.0f;
static float gPlayerShieldHoldTimer = 0.0f;
static float gPlayerAttackPressureTimer = 0.0f;

// Logic for when someone dies
static Character *gPendingDeathCharacter = NULL;
static int gPendingDeathCharType = 0;
static bool gPendingDeathStarted = false;

// --- Functions ---

// Loads MP3 files from the computer into the game's memory
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
    // ... repeats for other sounds ...
    sfx->nextAttack1 = 0;
    sfx->nextAttack2 = 0;
    sfx->nextAttack3 = 0;
    sfx->nextHurt = 0;
    sfx->nextDefence = 0;
}

// Clears memory to prevent the game from crashing when closed
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

// Prepares all sounds for the battle scene
void LoadBattleSfx(void) {
    if (gBattleSfxLoaded) return;
    LoadCharacterBattleSfx(&gBattleSfx[0], "audio/Sfx/Reuben");
    LoadCharacterBattleSfx(&gBattleSfx[1], "audio/Sfx/Ashat Leader");
    LoadCharacterBattleSfx(&gBattleSfx[2], "audio/Sfx/Commander");
    gReubenWalkSfx = LoadSound("audio/Sfx/walk.mp3");
    gBattleSfxLoaded = true;
}

bool IsBattleSfxBusy(void) {
    (void)gBattleSfxLoaded;
    return false;
}

// Stops all sounds and clears the "waiting line" (useful between rounds)
void ResetBattleSfxQueue(void) {
    if (gCurrentBattleSfx != NULL) StopSound(*gCurrentBattleSfx);
    // ... loops through all characters and stops their sounds ...
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

// Completely removes all battle sounds from memory to free up space
void UnloadBattleSfx(void) {
    // 1. Check if sounds are even there. If not, there is nothing to do!
    if (!gBattleSfxLoaded) return;

    // 2. Stop any sounds that are currently playing and clear the waiting line
    ResetBattleSfxQueue();

    // 3. Loop through each of the 3 character slots (Player + Enemies)
    // and delete their specific attack/hurt sounds from the RAM
    for (int c = 0; c < 3; c++) {
        UnloadCharacterBattleSfx(&gBattleSfx[c]);
    }

    // 4. Specifically delete the walking sound file
    UnloadSound(gReubenWalkSfx);

    // 5. Set the flag to "false" so the game knows sounds need to be 
    // re-loaded if another battle starts
    gBattleSfxLoaded = false;
}

// Safety check: makes sure we don't look for a character that doesn't exist
static int ClampBattleCharType(int charType) {
    return (charType < 0 || charType > 2) ? 0 : charType;
}

// Forces a character to switch to a new animation (like "Hurt" or "Jump")
static void StartBattleAnimation(Character *target, CharState state) {
    if (target == NULL) return;
    target->state = state;
    target->currentFrame = 0;  // Start the animation at the beginning
    target->frameTimer = 0.0f;
}

// Plays a sound immediately without waiting in the queue
static void PlayBattleSfxNow(Sound *sound) {
    // 1. Safety Check: Don't play if sounds aren't loaded, or the sound file is empty/broken
    if (!gBattleSfxLoaded || sound == NULL || sound->frameCount == 0) return;

    // 2. Interrupt: Stop the walking sound so it doesn't overlap with the action
    StopSound(gReubenWalkSfx);

    // 3. Play: Start the requested sound and keep track of it as the "current" sound
    PlaySound(*sound);
    gCurrentBattleSfx = sound;
}

// The main loop that processes the queue
void UpdateBattleSfxQueue(void) {
    if (!gBattleSfxLoaded) return;

    // As long as there are sounds in the "waiting line," play them
    while (gBattleSfxQueueCount > 0) {
        BattleSfxEvent event = gBattleSfxQueue[gBattleSfxQueueHead];

        // Move to the next spot in the circular queue
        gBattleSfxQueueHead = (gBattleSfxQueueHead + 1) % BATTLE_SFX_QUEUE_MAX;
        gBattleSfxQueueCount--;

        // If this event has an animation, start it
        if (event.startAnimation) {
            StartBattleAnimation(event.target, event.stateToStart);
        }

        // If this event has a sound, play it
        if (event.sound != NULL && event.sound->frameCount > 0) {
            PlaySound(*event.sound);
            gCurrentBattleSfx = event.sound;
        }
    }
}

// This function checks if a specific character (target) is already 
// waiting in line to start a new animation.
static bool HasQueuedAnimationForTarget(Character *target) {
    
    // Safety check: if there is no character to look for, return false
    if (target == NULL) return false;
    
    // Loop through only the number of items currently in the queue
    for (int i = 0; i < gBattleSfxQueueCount; i++) {
        
        // Calculate the actual position in the circular array.
        // We start at the 'Head' and wrap around to the beginning if we hit the MAX.
        int index = (gBattleSfxQueueHead + i) % BATTLE_SFX_QUEUE_MAX;
        
        // Check the event at this position:
        // 1. Does this event involve starting an animation?
        // 2. Is this animation for the specific character we are looking for?
        if (gBattleSfxQueue[index].startAnimation && gBattleSfxQueue[index].target == target) {
            return true; // We found a match! Stop looking and return true.
        }
    }
    
    // If we checked every item in the line and found nothing, return false
    return false;
}

// This function picks which sound file to use when a character attacks
static Sound *GetAttackSfx(int charType, int attackNumber) {
    
    // 1. Find the character's sound folder (using a safety check to ensure the ID is 0, 1, or 2)
    CharacterBattleSfx *sfx = &gBattleSfx[ClampBattleCharType(charType)];
    
    // 2. If it's the FIRST type of attack (e.g., a Light Punch)
    if (attackNumber == 1) {
        // Grab the current sound variant (either variant 0 or variant 1)
        Sound *sound = &sfx->attack1[sfx->nextAttack1];
        
        // Update the counter: if it was 0, it becomes 1. If it was 1, it wraps back to 0.
        // This ensures the NEXT time you punch, it uses a different sound.
        sfx->nextAttack1 = (sfx->nextAttack1 + 1) % BATTLE_SFX_VARIANTS;
        
        return sound; // Send the chosen sound back to be played
    }
    
    // 3. If it's the SECOND type of attack (e.g., a Kick)
    if (attackNumber == 2) {
        Sound *sound = &sfx->attack2[sfx->nextAttack2];
        sfx->nextAttack2 = (sfx->nextAttack2 + 1) % BATTLE_SFX_VARIANTS;
        return sound;
    }
    
    // 4. Default: If it's not attack 1 or 2, assume it's the THIRD type (e.g., a Heavy Slash)
    Sound *sound = &sfx->attack3[sfx->nextAttack3];
    sfx->nextAttack3 = (sfx->nextAttack3 + 1) % BATTLE_SFX_VARIANTS;
    
    return sound;
}

// Picks the next "Ouch!" sound for a character when they take damage
static Sound *GetHurtSfxOnly(int charType) {
    // 1. Find the sound set for this character
    CharacterBattleSfx *sfx = &gBattleSfx[ClampBattleCharType(charType)];
    
    // 2. Pick the current "hurt" sound variant
    Sound *sound = &sfx->hurt[sfx->nextHurt];
    
    // 3. Cycle the counter so the next hit uses a different "hurt" sound
    sfx->nextHurt = (sfx->nextHurt + 1) % BATTLE_SFX_VARIANTS;
    
    return sound;
}

// Simply grabs the specific "Death" sound for a character
static Sound *GetDeathSfxOnly(int charType) {
    // No variants here; death usually only has one dramatic sound!
    return &gBattleSfx[ClampBattleCharType(charType)].death;
}

// "Bookmarks" a character to die soon
static void ScheduleDeathAfterHit(Character *character, int charType) {
    // We store the character and their type in global variables (gPending...)
    // This is like saying: "This person is out of health, but let's 
    // finish the current hit animation before they vanish/fall over."
    gPendingDeathCharacter = character;
    gPendingDeathCharType = ClampBattleCharType(charType);
    
    // Reset the flag to show the death process hasn't actually begun yet
    gPendingDeathStarted = false;
}

// Checks if this specific character is currently "on deck" to die
static bool IsPendingDeathCharacter(Character *character) {
    // Returns TRUE only if:
    // 1. The character actually exists (not NULL)
    // 2. This character matches the one we bookmarked for death earlier
    // 3. The death animation/process hasn't actually begun yet
    return character != NULL && character == gPendingDeathCharacter && !gPendingDeathStarted;
}

// Checks if ANY battle sound is playing (except for the death sound)
static bool IsNonDeathBattleSfxPlaying(void) {
    // 1. If sounds aren't even loaded, nothing can be playing
    if (!gBattleSfxLoaded) return false;

    // 2. Loop through all 3 characters (Player and Enemies)
    for (int c = 0; c < 3; c++) {
        // Check every variant of every action sound
        for (int i = 0; i < BATTLE_SFX_VARIANTS; i++) {
            // If any attack, hurt, or block sound is still making noise, return true
            if (IsSoundPlaying(gBattleSfx[c].attack1[i])) return true;
            if (IsSoundPlaying(gBattleSfx[c].attack2[i])) return true;
            if (IsSoundPlaying(gBattleSfx[c].attack3[i])) return true;
            if (IsSoundPlaying(gBattleSfx[c].hurt[i])) return true;
            if (IsSoundPlaying(gBattleSfx[c].defence[i])) return true;
        }
        // Also check if the jumping sound is playing
        if (IsSoundPlaying(gBattleSfx[c].jump)) return true;
    }

    // 3. Finally, check if the walking sound is playing
    if (IsSoundPlaying(gReubenWalkSfx)) return true;

    // If we got through all checks and nothing was playing, return false
    return false;
}

// Checks if the character is currently in the middle of an "Action" (Attacking or being Hurt)
static bool IsActionAnimationActive(Character *character) {
    // 1. Safety check: If there's no character, they aren't doing anything
    if (character == NULL) return false;

    // 2. Identify "Action States": Is the character currently attacking (1, 2, or 3) or reeling from a hit?
    bool actionState = character->state == STATE_ATTACK_1 || character->state == STATE_ATTACK_2 ||
                       character->state == STATE_ATTACK_3 || character->state == STATE_HURT;

    // 3. If they aren't in one of those states, they are free (Idle, Walking, etc.)
    if (!actionState) return false;

    // 4. If they ARE in an action state, check if the animation has finished.
    // We look up the total frames for the current state and compare it to the current frame.
    int lastFrame = character->frameCounts[character->state] - 1;
    
    // Returns TRUE if the character hasn't reached the final frame of the animation yet
    return character->currentFrame < lastFrame;
}

// Picks the next "Block" or "Shield" sound variant for a character
static Sound *GetDefenceSfx(int charType) {
    // Find the sound set for this character (Reuben, Enemy Leader, or Commander)
    CharacterBattleSfx *sfx = &gBattleSfx[ClampBattleCharType(charType)];
    
    // Pick the sound and move the counter (0 to 1, or 1 to 0) to keep audio fresh
    Sound *sound = &sfx->defence[sfx->nextDefence];
    sfx->nextDefence = (sfx->nextDefence + 1) % BATTLE_SFX_VARIANTS;
    
    return sound;
}

// Grabs the specific "Jump" sound for a character
static Sound *GetJumpSfx(int charType) {
    // Unlike attacks, jumping usually only has one sound file per character
    return &gBattleSfx[ClampBattleCharType(charType)].jump;
}

// Checks if the character is allowed to perform a new move right now
static bool CanStartNewBattleAction(bool isPlayer) {
    // If it's the player, we check the 'Cooldown' timer.
    // If the timer is greater than 0, the player is still "recovering" and can't move.
    if (isPlayer && gPlayerActionCooldown > 0.0f) return false;
    
    // Otherwise, they are free to act!
    return true;
}

// Adjusts how much damage is dealt based on who is fighting whom (Rock-Paper-Scissors logic)
static int ApplyBattleDamageScaling(int damage, int attackerType, int defenderType) {
    // Logic: If Player (Type 0) attacks the Commander (Type 2), they do less damage.
    if (attackerType == 0 && defenderType == 2) {
        // Multiply damage by 0.75 (meaning the attacker only does 75% of their normal power)
        int scaledDamage = (int)roundf((float)damage * 0.75f);
        
        // Safety: Ensure the attack always does at least 1 damage (don't let it be 0)
        return scaledDamage < 1 ? 1 : scaledDamage;
    }
    
    // If it's any other matchup, deal normal 100% damage
    return damage;
}

// Automatically slides a character away from their opponent
static void MoveAwayFromOpponent(Character *character, Character *opponent, float speed, float dt) {
    // Safety check: Make sure both fighters exist
    if (character == NULL || opponent == NULL) return;
    
    // Determine direction: 
    // If I am to the left of the opponent, move further left (-1.0).
    // If I am to the right, move further right (1.0).
    float dir = (character->position.x < opponent->position.x) ? -1.0f : 1.0f;
    
    // Update the X position. 
    // 'dt' (Delta Time) ensures the movement is smooth regardless of the computer's speed.
    character->position.x += dir * speed * dt;
}

// Automatically turns the character so they are always looking at their rival
static void FaceOpponent(Character *character, Character *opponent) {
    // Safety check: Make sure both characters exist
    if (character == NULL || opponent == NULL) return;

    // Compare X positions. If the opponent's X is greater or equal to mine, 
    // it means they are to my right, so 'facingRight' becomes true.
    character->facingRight = (opponent->position.x >= character->position.x);
}

// A math helper that tells us which way to move to reach the opponent
static float DirectionToward(Character *from, Character *to) {
    // If someone is missing, default to moving Right (1.0f)
    if (from == NULL || to == NULL) return 1.0f;

    // Returns 1.0 (Right) if the target is ahead, or -1.0 (Left) if the target is behind
    return (to->position.x >= from->position.x) ? 1.0f : -1.0f;
}

// Executes a "Crossup" jump (jumping over the opponent to hit them from behind)
static void StartCrossupJump(Character *jumper, Character *opponent, float horizontalSpeed) {
    if (jumper == NULL) return;

    // 1. Vertical Launch: Apply the JUMP_FORCE to make the character go UP
    jumper->velocity.y = JUMP_FORCE;

    // 2. Initial Horizontal Speed: Start with 0 (jumping straight up by default)
    jumper->velocity.x = 0.0f;

    if (opponent != NULL) {
        // 3. Proximity Check: Calculate how far away the opponent is (fabs = absolute value)
        float distance = fabs(jumper->position.x - opponent->position.x);

        // 4. Trigger Crossup: If we are close enough (less than CROSSUP_DISTANCE),
        // give the character forward speed so they vault OVER the opponent.
        if (distance < CROSSUP_DISTANCE) {
            jumper->velocity.x = DirectionToward(jumper, opponent) * horizontalSpeed;
        }
    }
}

// Forces an action (like an attack) to happen RIGHT NOW
static void StartImmediateActionWithSfx(Character *character, CharState state, Sound *sound) {
    // 1. Instantly switch the character's visual pose (e.g., from Idle to Punching)
    StartBattleAnimation(character, state);
    
    // 2. Instantly play the associated sound effect
    PlayBattleSfxNow(sound);
}

// Triggers a reaction (like being hit) and its sound effect
static void QueueReactionAnimationWithSfx(Character *character, CharState state, Sound *sound) {
    // Note: Currently, this does the same thing as the function above.
    // It instantly changes the animation and plays the sound.
    StartBattleAnimation(character, state);
    PlayBattleSfxNow(sound);
}

// A "placeholder" function meant to play footstep sounds
static void PlayWalkSfxIfNeeded(CharState previousState, CharState currentState, int charType) {
    // These lines (void) tell the compiler: "I know I'm not using these variables yet."
    // This prevents the computer from giving a warning message while the 
    // programmer is still building this part of the game.
    (void)previousState;
    (void)currentState;
    (void)charType;
}

// Sets up a character from scratch when the game starts
void InitCharacter(Character* c, Vector2 startPos, int charType) {
    // 1. Basic Stats: Set where they stand, how fast they move, and their health
    c->position = startPos;
    c->velocity = (Vector2){ 0, 0 };  // Start standing still
    c->state = STATE_IDLE;  // Start in the idle (waiting) pose
    c->facingRight = (charType == 0);  // Usually, players (0) face right, enemies face left
    c->isGrounded = false;  // Assume they are in the air until they touch the floor
    c->health = 100;
    c->aiTimer = 0.0f;
    c->hasHealed = false; 
    
    // 2. Folder Paths: Pick the right folder based on who the character is
    const char* basePath = "";
    if (charType == 0) basePath = "images/Character/Reuben";
    else if (charType == 2) basePath = "images/Character/Commander";
    else if (charType == 1) basePath = "images/Character/Ashat Leader";

    // 3. Loading Graphics: Load the "Sprite Sheets" (images) for every action
    c->textures[STATE_IDLE] = LoadTexture(TextFormat("%s/Idle.png", basePath));
    c->textures[STATE_WALK] = LoadTexture(TextFormat("%s/Walk.png", basePath));
    c->textures[STATE_RUN] = LoadTexture(TextFormat("%s/Walk.png", basePath));
    c->textures[STATE_JUMP] = LoadTexture(TextFormat("%s/Jump.png", basePath));
    c->textures[STATE_ATTACK_1] = LoadTexture(TextFormat("%s/Attack_1.png", basePath));
    c->textures[STATE_ATTACK_2] = LoadTexture(TextFormat("%s/Attack_2.png", basePath));
    c->textures[STATE_HURT] = LoadTexture(TextFormat("%s/Hurt.png", basePath));
    c->textures[STATE_DEAD] = LoadTexture(TextFormat("%s/Dead.png", basePath));

    // Special case: Different enemies might use different names for their defensive images
    if (charType ==1) { 
        c->textures[STATE_SHIELD] = LoadTexture(TextFormat("%s/Defence.png", basePath));
        c->textures[STATE_ATTACK_3] = LoadTexture(TextFormat("%s/Attack_2.png", basePath));
    } else {
        c->textures[STATE_SHIELD] = LoadTexture(TextFormat("%s/Shield.png", basePath));
        c->textures[STATE_ATTACK_3] = LoadTexture(TextFormat("%s/Attack_3.png", basePath));
    }
    
    // 4. Animation Data: Tell the game how many "frames" (drawings) are in each image
    if (charType == 0) {  // Reuben's frame counts
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
    } else if (charType == 1) {  // Ashat Leader's frame counts
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
    } else if (charType == 2) {  // Commander's frame counts
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
    
    // 5. Animation Timer: Set how fast the character breathes/moves
    c->currentFrame = 0;
    c->frameTimer = 0.0f;
    c->frameDuration = 0.1f; // Each picture stays on screen for 0.1 seconds
    // 6. Physics: Create the "Hitbox" (an invisible box used to detect punches)
    c->hitBox = (Rectangle){ c->position.x, c->position.y, 50, 100 }; 
}

void UpdateCharacterWithSfx(Character* c, Character* opponent, float dt, bool isPlayer, int charType, int opponentType) {
    // Keep track of what the character was doing before this update
    CharState previousState = c->state;

    // --- PLAYER TIMER UPDATES ---
    if (isPlayer) {
        // If the player is waiting for a cooldown (like after a jump or attack)
        if (gPlayerActionCooldown > 0.0f) {
            gPlayerActionCooldown -= dt;
            if (gPlayerActionCooldown < 0.0f) gPlayerActionCooldown = 0.0f;
        }
        // Pressure timer: used to make the AI back off if the player is being aggressive
        if (gPlayerAttackPressureTimer > 0.0f) {
            gPlayerAttackPressureTimer -= dt;
            if (gPlayerAttackPressureTimer < 0.0f) gPlayerAttackPressureTimer = 0.0f;
        }
    }

    // --- PHYSICS & GRAVITY ---
    if (c->state != STATE_DEAD) {
        // Apply gravity to downward speed
        c->velocity.y += GRAVITY * dt;
        // Move the character up/down
        c->position.y += c->velocity.y * dt;

        // If in the air, apply horizontal momentum (drifting)
        if (!c->isGrounded) {
            c->position.x += c->velocity.x * dt;
        }

        // Floor Collision: Stop the character if they hit the GROUND_Y
        if (c->position.y >= GROUND_Y) {
            c->position.y = GROUND_Y;
            c->velocity.y = 0.0f;
            c->velocity.x = 0.0f;
            c->isGrounded = true;
            // If they land while in a jump state, put them back to idle
            if (c->state == STATE_JUMP) c->state = STATE_IDLE;
        } else {
            c->isGrounded = false;
        }
    }

    // Check if the character is stuck in a "reaction" (like getting hit) from the queue
    bool waitingForQueuedReaction = HasQueuedAnimationForTarget(c);

    // --- SHIELD LOGIC (Blocking) ---
    if (isPlayer && c->state == STATE_SHIELD) {
        // As long as S is held and they have "shield energy" (hold timer), stay blocking
        if (IsKeyDown(KEY_S) && c->isGrounded && gPlayerShieldHoldTimer > 0.0f) {
            gPlayerShieldHoldTimer -= dt;
            if (gPlayerShieldHoldTimer < 0.0f) gPlayerShieldHoldTimer = 0.0f;
        } else {
            // Stop blocking if button released or timer runs out
            c->state = STATE_IDLE;
            c->currentFrame = 0;
            c->frameTimer = 0.0f;
            gPlayerShieldHoldTimer = 0.0f;
            // Apply a small delay before they can block again
            if (gPlayerActionCooldown < PLAYER_DEFENCE_COOLDOWN) {
                gPlayerActionCooldown = PLAYER_DEFENCE_COOLDOWN;
            }
        }
    }

    // --- PLAYER INPUT CONTROLS ---
    // Only allow movement/attacks if not dead, not hurt, and not already attacking
    if (!waitingForQueuedReaction && isPlayer && c->state != STATE_DEAD && c->state != STATE_HURT &&
        c->state != STATE_SHIELD &&
        c->state != STATE_ATTACK_1 && c->state != STATE_ATTACK_2 && c->state != STATE_ATTACK_3)
    {
        bool isMoving = false;
        // Shift key makes the character run faster
        float moveSpeed = IsKeyDown(KEY_LEFT_SHIFT) ? 500.0f : 250.0f;
        CharState moveState = IsKeyDown(KEY_LEFT_SHIFT) ? STATE_RUN : STATE_WALK;

        // Move Right (D)
        if (IsKeyDown(KEY_D)) {
            c->position.x += moveSpeed * dt;
            if (c->isGrounded) c->state = moveState;
            isMoving = true;
        } 
        // Move Left (A)
        else if (IsKeyDown(KEY_A)) {
            c->position.x -= moveSpeed * dt;
            if (c->isGrounded) c->state = moveState;
            isMoving = true;
        }

        // Block (S)
        if (IsKeyDown(KEY_S) && c->isGrounded && !isMoving && CanStartNewBattleAction(true)) {
            StartImmediateActionWithSfx(c, STATE_SHIELD, GetDefenceSfx(charType));
            gPlayerShieldHoldTimer = PLAYER_SHIELD_MAX_HOLD;
        } 
        // Idle (Standing still)
        else if (!isMoving && c->isGrounded && c->state != STATE_SHIELD) {
            c->state = STATE_IDLE;
        }

        // Jump (Space)
        if (IsKeyPressed(KEY_SPACE) && c->isGrounded && CanStartNewBattleAction(true)) {
            StartImmediateActionWithSfx(c, STATE_JUMP, GetJumpSfx(charType));
            // Check for Crossup (leaping over opponent)
            StartCrossupJump(c, opponent, PLAYER_CROSSUP_JUMP_SPEED);
            // If too far for a crossup, allow "Air Drift" based on A/D keys
            if (fabs(c->position.x - opponent->position.x) >= CROSSUP_DISTANCE) {
                if (IsKeyDown(KEY_D)) c->velocity.x = AIR_DRIFT_SPEED;
                else if (IsKeyDown(KEY_A)) c->velocity.x = -AIR_DRIFT_SPEED;
            }
            gPlayerActionCooldown = PLAYER_JUMP_COOLDOWN;
        }

        // --- PLAYER ATTACK LOGIC ---
        bool wantsAttack1 = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);  // Light
        bool wantsAttack2 = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT); // Medium
        bool wantsAttack3 = IsKeyPressed(KEY_E);                      // Heavy

        if ((wantsAttack1 || wantsAttack2 || wantsAttack3) && c->isGrounded && CanStartNewBattleAction(true)) {
            int damage = 10;
            int attackNumber = 1;
            CharState attackState = STATE_ATTACK_1;
            float cooldown = PLAYER_LIGHT_COOLDOWN;

            // Define which attack was chosen
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

            // Start the attack animation and sound
            StartImmediateActionWithSfx(c, attackState, GetAttackSfx(charType, attackNumber));
            gPlayerActionCooldown = cooldown;
            gPlayerAttackPressureTimer = PLAYER_ATTACK_PRESSURE_TIME;
            
            // Adjust damage based on matchup
            damage = ApplyBattleDamageScaling(damage, charType, opponentType);

            // HIT DETECTION: Check if opponent is close enough (140 pixels)
            if (fabs(c->position.x - opponent->position.x) < 140.0f && opponent->state != STATE_DEAD && opponent->isGrounded && opponent->state != STATE_JUMP && !HasQueuedAnimationForTarget(opponent)) {
                bool blocked = false;
                bool canDefendOrDodge = (opponent->state == STATE_IDLE || opponent->state == STATE_WALK || opponent->state == STATE_RUN || opponent->state == STATE_SHIELD) && opponent->isGrounded;
                bool dodgedByJump = false;
                
                // If opponent is already holding shield
                blocked = opponent->state == STATE_SHIELD;

                // AI "Intelligence" Roll: Decide if the opponent dodges or blocks
                if (canDefendOrDodge) {
                    int defenseRoll = GetRandomValue(1, 100);
                    if (defenseRoll <= 32) {
                        dodgedByJump = true; // 32% chance to jump away
                    } else if (defenseRoll <= 55) {
                        blocked = true;      // 23% chance to block
                    }
                }

                // Apply the result to the opponent
                if (dodgedByJump) {
                    StartImmediateActionWithSfx(opponent, STATE_JUMP, GetJumpSfx(opponentType));
                    StartCrossupJump(opponent, c, ENEMY_DODGE_JUMP_SPEED);
                    opponent->aiTimer = GetRandomValue(10, 17) / 10.0f;
                } else if (blocked) {
                    QueueReactionAnimationWithSfx(opponent, STATE_SHIELD, GetDefenceSfx(opponentType));
                } else {
                    // Successful Hit!
                    opponent->health -= damage;
                    if (opponent->health <= 0) {
                        opponent->health = 0;
                        QueueReactionAnimationWithSfx(opponent, STATE_HURT, GetHurtSfxOnly(opponentType));
                        ScheduleDeathAfterHit(opponent, opponentType); // Book death for later
                    } else {
                        QueueReactionAnimationWithSfx(opponent, STATE_HURT, GetHurtSfxOnly(opponentType));
                    }
                }
            }
        }
    }
    // --- AI ENEMY LOGIC ---
    else if (!waitingForQueuedReaction && !isPlayer && c->state != STATE_DEAD && c->state != STATE_HURT &&
             c->state != STATE_ATTACK_1 && c->state != STATE_ATTACK_2 && c->state != STATE_ATTACK_3)
    {
        c->facingRight = (opponent->position.x > c->position.x);
        float distanceToPlayer = fabs(c->position.x - opponent->position.x);

        c->aiTimer -= dt;

        // Spacing: If player is aggressive, the AI might move away
        if (gPlayerAttackPressureTimer > 0.0f && distanceToPlayer < 220.0f &&
                 c->aiTimer <= 0.0f && GetRandomValue(1, 100) <= ENEMY_SPACING_CHANCE) {
            MoveAwayFromOpponent(c, opponent, 185.0f, dt);
            if (c->isGrounded) c->state = STATE_WALK;
            c->aiTimer = GetRandomValue(4, 7) / 10.0f;
        }
        // Chase: If player is far away, move toward them
        else if (distanceToPlayer > 120.0f) {
            if (distanceToPlayer > 300.0f) {
                c->position.x += (c->facingRight ? 320.0f : -320.0f) * dt;
                if (c->isGrounded) c->state = STATE_RUN;
            } else {
                c->position.x += (c->facingRight ? 220.0f : -220.0f) * dt;
                if (c->isGrounded) c->state = STATE_WALK;
            }
        }
        // Attack: If player is close, try to hit them
        else {
            if (c->isGrounded && c->state != STATE_IDLE && c->state != STATE_SHIELD) c->state = STATE_IDLE;

            // Proactive Jump (Dodge)
            if (c->aiTimer <= 0.0f && c->isGrounded && distanceToPlayer < 170.0f &&
                GetRandomValue(1, 100) <= ENEMY_PROACTIVE_JUMP_CHANCE) {
                StartImmediateActionWithSfx(c, STATE_JUMP, GetJumpSfx(charType));
                StartCrossupJump(c, opponent, ENEMY_DODGE_JUMP_SPEED);
                c->aiTimer = GetRandomValue(7, 11) / 10.0f;
            } 
            // Random Attack
            else if (c->aiTimer <= 0.0f && opponent->state != STATE_DEAD && opponent->isGrounded && opponent->state != STATE_JUMP && !HasQueuedAnimationForTarget(opponent) && CanStartNewBattleAction(false)) {
                int randAttack = GetRandomValue(1, 3);
                int damage = 10;
                CharState attackState = STATE_ATTACK_1;

                if (randAttack == 1) { damage = 10; attackState = STATE_ATTACK_1; }
                else if (randAttack == 2) { damage = 15; attackState = STATE_ATTACK_2; }
                else { damage = 25; attackState = STATE_ATTACK_3; }

                StartImmediateActionWithSfx(c, attackState, GetAttackSfx(charType, randAttack));
                c->aiTimer = GetRandomValue(10, 17) / 10.0f;

                // Did the player block?
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

    // --- SCREEN BOUNDARIES ---
    if (c->position.x < 50.0f) c->position.x = 50.0f;
    if (c->position.x > 1230.0f) c->position.x = 1230.0f;

    // Always look at the opponent
    FaceOpponent(c, opponent);

    // --- ANIMATION FRAME ADVANCEMENT ---
    c->frameTimer += dt;
    if (c->frameTimer >= c->frameDuration) {
        c->frameTimer = 0.0f;
        c->currentFrame++;

        // If we reached the end of an animation
        if (c->currentFrame >= c->frameCounts[c->state]) {
            // Attacks and Hurt animations return to IDLE
            if (c->state == STATE_ATTACK_1 || c->state == STATE_ATTACK_2 || c->state == STATE_ATTACK_3 || c->state == STATE_HURT) {
                if (c->health <= 0) {
                    // If they are dying, stay on the last frame or switch to DEAD state
                    if (IsPendingDeathCharacter(c)) {
                        c->currentFrame = c->frameCounts[c->state] - 1;
                    } else {
                        c->state = STATE_DEAD;
                        c->currentFrame = 0;
                    }
                } else {
                    c->state = STATE_IDLE;
                }
            } 
            // Stay on the last frame for Jump and Dead
            else if (c->state == STATE_JUMP) {
                c->currentFrame = c->frameCounts[STATE_JUMP] - 1;
            } else if (c->state == STATE_DEAD) {
                c->currentFrame = c->frameCounts[STATE_DEAD] - 1;
            } 
            // Blocking returns to idle
            else if (c->state == STATE_SHIELD) {
                c->state = STATE_IDLE;
                c->currentFrame = 0;
            } 
            // Loop back to start for Idle/Walk/Run
            else {
                c->currentFrame = 0;
            }
        }
    }

    // Finally, play footstep sounds if necessary
    PlayWalkSfxIfNeeded(previousState, c->state, charType);
}

// Manages animation frames during the "Slow Motion" or end-of-battle sequence
static void AdvanceBattleEndAnimation(Character *c, float dt) {
    // 1. Safety check: Exit if the character doesn't exist
    if (c == NULL) return;

    // 2. Timing: Update the timer and only proceed if it's time for the next frame
    c->frameTimer += dt;
    if (c->frameTimer < c->frameDuration) return;

    // 3. Increment: Move to the next picture in the animation sequence
    c->frameTimer = 0.0f;
    c->currentFrame++;

    // 4. End-of-Animation Logic: What happens when we run out of frames?
    if (c->currentFrame >= c->frameCounts[c->state]) {
        
        // If they just finished an attack, put them back into their waiting (Idle) pose
        if (c->state == STATE_ATTACK_1 || c->state == STATE_ATTACK_2 || c->state == STATE_ATTACK_3) {
            c->state = STATE_IDLE;
            c->currentFrame = 0;
        } 
        // If they were reeling from a hit (Hurt state)
        else if (c->state == STATE_HURT) {
            // If this hit was fatal, freeze on the last "Hurt" frame until the death state kicks in
            if (IsPendingDeathCharacter(c)) {
                c->currentFrame = c->frameCounts[c->state] - 1;
            } else {
                // Otherwise, they recovered; go back to Idle
                c->state = STATE_IDLE;
                c->currentFrame = 0;
            }
        } 
        // If they are in the Dead state, stay frozen on the very last frame (lying on the ground)
        else if (c->state == STATE_DEAD) {
            c->currentFrame = c->frameCounts[STATE_DEAD] - 1;
        } 
        // For anything else (like Idle or Walk), loop back to the first frame
        else {
            c->currentFrame = 0;
        }
    }
}

// Manages the final moments of a battle (The "KO" sequence)
bool UpdateBattleEndSequence(Character *player, Character *enemy, float dt) {
    // 1. Keep the animations moving for both characters (breathing, falling, etc.)
    AdvanceBattleEndAnimation(player, dt);
    AdvanceBattleEndAnimation(enemy, dt);

    // 2. If nobody is marked to die, we are done here. 
    // Return true to tell the game it's okay to leave the battle screen.
    if (gPendingDeathCharacter == NULL) {
        return true;
    }

    // 3. Check if both characters have finished their current "Action" (like a final swing or reeling back)
    bool animationsFinished = !IsActionAnimationActive(player) && !IsActionAnimationActive(enemy);

    // 4. Trigger the actual Death:
    // If we haven't started the death yet, AND animations are done, AND no other sounds are playing...
    if (!gPendingDeathStarted && animationsFinished && !IsNonDeathBattleSfxPlaying()) {
        // Change the character's pose to the "Dead" sprite sheet
        StartBattleAnimation(gPendingDeathCharacter, STATE_DEAD);
        // Play that final dramatic death sound effect
        PlayBattleSfxNow(GetDeathSfxOnly(gPendingDeathCharType));
        // Set this flag so we don't trigger the death sound/animation more than once
        gPendingDeathStarted = true;
    }

    // 5. Final Cleanup:
    if (gPendingDeathStarted) {
        // Is the character officially on the last frame of their "Dead" animation?
        bool deathAnimDone = gPendingDeathCharacter->state == STATE_DEAD &&
                             gPendingDeathCharacter->currentFrame >= gPendingDeathCharacter->frameCounts[STATE_DEAD] - 1;
        
        // Has the death sound finished ringing out?
        bool deathSfxDone = !IsSoundPlaying(*GetDeathSfxOnly(gPendingDeathCharType));

        // Only when the body is on the floor AND the sound is silent do we finish
        if (deathAnimDone && deathSfxDone) {
            gPendingDeathCharacter = NULL; // Clear the "to-die" bookmark
            gPendingDeathStarted = false;  // Reset the flag for the next battle
            return true;                   // Tell the game: "Everything is finished, you can switch screens now!"
        }
    }

    // If we reach this point, the death sequence is still playing
    return false;
}

// A simple wrapper that calls the main update logic with the correct character IDs
void UpdateCharacter(Character* c, Character* opponent, float dt, bool isPlayer) {
    // If it's the player, set type to 0. If it's the enemy, set type to 1.
    // This tells the main update function which sound/animation set to use.
    UpdateCharacterWithSfx(c, opponent, dt, isPlayer, isPlayer ? 0 : 1, isPlayer ? 1 : 0);
}

// Prevents characters from overlapping and ensures they always face each other
void ResolveFighterOverlap(Character *a, Character *b) {
    // 1. Safety Check: If someone is missing, we can't calculate distance
    if (a == NULL || b == NULL) return;

    // 2. Distance Check: Calculate how far apart the two fighters are on the X-axis
    const float minDistance = 95.0f; // This is the "personal space" bubble (95 pixels)
    float dx = b->position.x - a->position.x;
    float distance = fabsf(dx);

    // 3. Collision Resolution: If both are on the ground and too close...
    if (a->isGrounded && b->isGrounded && distance < minDistance) {
        // Calculate how much they need to move to stop overlapping
        // We divide by 2 so they both move an equal amount (0.5f)
        float push = (minDistance - distance) * 0.5f;

        // If B is to the right of A, push A left and B right
        if (dx >= 0.0f) {
            a->position.x -= push;
            b->position.x += push;
        } 
        // Otherwise, push them the opposite way
        else {
            a->position.x += push;
            b->position.x -= push;
        }

        // 4. Bound Check: After pushing them, make sure nobody was shoved off-screen
        if (a->position.x < 50.0f) a->position.x = 50.0f;
        if (a->position.x > 1230.0f) a->position.x = 1230.0f;
        if (b->position.x < 50.0f) b->position.x = 50.0f;
        if (b->position.x > 1230.0f) b->position.x = 1230.0f;
    }

    // 5. Final Orientation: Make sure both fighters turn to look at each other
    a->facingRight = (b->position.x >= a->position.x);
    b->facingRight = (a->position.x >= b->position.x);
}

// This function handles drawing the character's sprite onto the screen
void DrawCharacter(Character* c) {
    // 1. Pick the correct "Sprite Sheet" based on the character's current state (Idle, Attack, etc.)
    Texture2D tex = c->textures[c->state];
    
    // Safety check: If the image didn't load properly, don't try to draw it
    if (tex.id == 0) return; 

    // 2. Calculations for Sprite Sheets:
    // We divide the total width of the image by the number of frames to get the width of ONE frame.
    int numFrames = c->frameCounts[c->state];
    float frameWidth = (float)tex.width / numFrames;
    float frameHeight = (float)tex.height;

    // 3. Source Rectangle: This defines which "slice" of the Sprite Sheet we are looking at.
    // We multiply currentFrame by frameWidth to "slide" across the image.
    // If facingRight is false, we use a negative width to horizontally flip the image!
    Rectangle sourceRec = { 
        (float)c->currentFrame * frameWidth, 0.0f, 
        c->facingRight ? frameWidth : -frameWidth, frameHeight 
    };

    // 4. Destination Rectangle: This defines WHERE on the screen the character is drawn.
    // We multiply by 3.5f to scale the character up (making them 3.5 times larger).
    Rectangle destRec = { 
        c->position.x, c->position.y, 
        frameWidth * 3.5f, frameHeight * 3.5f 
    };

    // 5. Origin point: This sets the "anchor" of the character to the bottom-center of their feet.
    // This ensures that when they rotate or scale, they stay pinned to the ground.
    Vector2 origin = { (frameWidth * 3.5f) / 2.0f, frameHeight * 3.5f };

    // 6. Visual Effects:
    Color tint = WHITE; // Normal color
    if (c->state == STATE_HURT) {
        // If the character is hurt, make them flash RED and WHITE rapidly.
        // GetTime() * 24.0 creates a fast pulsing effect.
        tint = (((int)(GetTime() * 24.0)) % 2 == 0) ? RED : WHITE;
    }

    // 7. Final Draw: Put it all together and send it to the graphics card.
    DrawTexturePro(tex, sourceRec, destRec, origin, 0.0f, tint);
}

// Draws the main HUD (Health bars and Timer) at the top of the screen
void DrawGameUI(int playerHealth, int enemyHealth, int timeRemaining, int screenWidth, int screenHeight) {
    (void)screenHeight; // Tells the compiler we aren't using this variable to avoid warnings
    int barWidth = (screenWidth / 2) - 80; // Size of the health bars
    int barHeight = 40;
    int padding = 30; // Space from the edge of the screen

    // 1. Draw the Timer Box in the center
    DrawRectangle((screenWidth / 2) - 30, padding, 60, 50, BLACK);
    DrawRectangleLines((screenWidth / 2) - 30, padding, 60, 50, WHITE); // White outline
    
    // 2. Format and draw the time (e.g., "99")
    char timeText[5];
    sprintf(timeText, "%02d", timeRemaining);
    DrawText(timeText, (screenWidth / 2) - 15, padding + 15, 20, WHITE);

    // 3. Player 1 Health Bar (Left side)
    float p1HealthPct = (float)playerHealth / 100.0f;
    // Background (Red represents the missing health)
    DrawRectangle(padding, padding + 5, barWidth, barHeight, RED); 
    // Foreground (Blue represents current health - drains from right to left)
    DrawRectangle(padding + (barWidth * (1.0f - p1HealthPct)), padding + 5, barWidth * p1HealthPct, barHeight, BLUE);
    DrawRectangleLines(padding, padding + 5, barWidth, barHeight, WHITE);

    // 4. Enemy Health Bar (Right side)
    float p2HealthPct = (float)enemyHealth / 100.0f;
    // Background (Red)
    DrawRectangle(screenWidth / 2 + 50, padding + 5, barWidth, barHeight, RED); 
    // Foreground (Blue - drains from right to left)
    DrawRectangle(screenWidth / 2 + 50, padding + 5, barWidth * p2HealthPct, barHeight, BLUE);
    DrawRectangleLines(screenWidth / 2 + 50, padding + 5, barWidth, barHeight, WHITE);
}

// Draws the "You Died" screen when the player loses
void DrawBattleDeathOverlay(int vWidth, int vHeight) {
    // 1. Darken the screen with a semi-transparent black layer (70% opacity)
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    
    // 2. Draw "YOU DIED" centered in the middle of the screen
    DrawText("YOU DIED", (vWidth - MeasureText("YOU DIED", 64)) / 2, vHeight / 2 - 60, 64, RED);
    
    // 3. Draw instructions to restart
    DrawText("Press R to Restart", (vWidth - MeasureText("Press R to Restart", 28)) / 2, vHeight / 2 + 20, 28, MAROON);
}

// Draws the "Victory" screen when the enemy is defeated
void DrawBattleWinOverlay(int vWidth, int vHeight) {
    // 1. Darken the screen
    DrawRectangle(0, 0, vWidth, vHeight, Fade(BLACK, 0.70f));
    
    // 2. Draw "VICTORY" in Gold
    DrawText("VICTORY", (vWidth - MeasureText("VICTORY", 64)) / 2, vHeight / 2 - 60, 64, GOLD);
    
    // 3. Draw congratulatory text
    DrawText("You won the fight!", (vWidth - MeasureText("You won the fight!", 28)) / 2, vHeight / 2 + 20, 28, YELLOW);
}