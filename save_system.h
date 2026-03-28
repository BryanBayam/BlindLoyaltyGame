#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// This struct holds everything we want to remember
typedef struct {
    char name[32];      // The custom name the player types
    Vector2 playerPos;  // Reuben's exact X and Y position
    float health;       // Reuben's health
    float energy;       // Reuben's energy/stamina
    int savedScreen;    // NEW: Remembers if you were in a Story Scene or Gameplay
} GameSaveData;

// Writes the data to a file
static inline bool SaveGameData(int slot, GameSaveData data) {
    FILE *file = fopen(TextFormat("save_slot_%d.dat", slot), "wb");
    if (!file) return false;
    fwrite(&data, sizeof(GameSaveData), 1, file);
    fclose(file);
    return true;
}

// Reads the data from a file
static inline bool LoadGameData(int slot, GameSaveData *data) {
    FILE *file = fopen(TextFormat("save_slot_%d.dat", slot), "rb");
    if (!file) return false;
    fread(data, sizeof(GameSaveData), 1, file);
    fclose(file);
    return true;
}

// Checks if a save file exists
static inline bool SaveExists(int slot) {
    return FileExists(TextFormat("save_slot_%d.dat", slot));
}

#endif