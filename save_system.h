#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
 * Stores the data needed to restore a saved game state.
 *
 * Fields:
 * - name: User-defined save slot name
 * - playerPos: Player position at the time of saving
 * - health: Player health value
 * - energy: Player stamina/energy value
 * - savedScreen: Screen/state to restore when loading
 */
typedef struct {
    char name[32];
    Vector2 playerPos;
    float health;
    float energy;
    int savedScreen;
} GameSaveData;

/*
 * Save game data to a slot file.
 *
 * Parameters:
 * - slot: Save slot index
 * - data: Save data to write
 *
 * Returns:
 * - true if the file was written successfully
 * - false if the file could not be opened
 */
static inline bool SaveGameData(int slot, GameSaveData data) {
    FILE *file = fopen(TextFormat("save_slot_%d.dat", slot), "wb");
    if (!file) {
        return false;
    }

    fwrite(&data, sizeof(GameSaveData), 1, file);
    fclose(file);
    return true;
}

/*
 * Load game data from a slot file.
 *
 * Parameters:
 * - slot: Save slot index
 * - data: Output pointer that receives the loaded save data
 *
 * Returns:
 * - true if the file was read successfully
 * - false if the file could not be opened
 */
static inline bool LoadGameData(int slot, GameSaveData *data) {
    FILE *file = fopen(TextFormat("save_slot_%d.dat", slot), "rb");
    if (!file) {
        return false;
    }

    fread(data, sizeof(GameSaveData), 1, file);
    fclose(file);
    return true;
}

/*
 * Check whether a save file exists for the given slot.
 *
 * Parameters:
 * - slot: Save slot index
 *
 * Returns:
 * - true if the save file exists
 * - false if the save file does not exist
 */
static inline bool SaveExists(int slot) {
    return FileExists(TextFormat("save_slot_%d.dat", slot));
}

#endif