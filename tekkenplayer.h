#ifndef TEKKENPLAYER_H
#define TEKKENPLAYER_H

#include "raylib.h"
#include <stdbool.h>

// Define all character states
typedef enum {
    STATE_IDLE = 0,
    STATE_WALK,
    STATE_RUN,
    STATE_JUMP,
    STATE_SHIELD,
    STATE_ATTACK_1,
    STATE_ATTACK_2,
    STATE_ATTACK_3,
    STATE_HURT,
    STATE_DEAD,
    NUM_STATES
} CharState;

// Character structure
typedef struct {
    Vector2 position;
    Vector2 velocity;
    CharState state;
    bool facingRight;
    bool isGrounded;  
    bool hasHealed;
    int health;
    
    // Animation properties
    Texture2D textures[NUM_STATES];
    int frameCounts[NUM_STATES];
    int currentFrame;
    float frameTimer;
    float frameDuration;
    
    // Hitbox
    Rectangle hitBox;

    // AI properties
    float aiTimer;
} Character;

// Function prototypes
void InitCharacter(Character* c, Vector2 startPos, int charType);
void UpdateCharacter(Character* c, Character* opponent, float dt, bool isPlayer);
void DrawCharacter(Character* c);
void DrawGameUI(int playerHealth, int enemyHealth, int timeRemaining, int screenWidth, int screenHeight);

#endif // TEKKENPLAYER_H
