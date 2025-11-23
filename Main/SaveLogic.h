#ifndef SAVELOGIC_H
#define SAVELOGIC_H

#include <Arduino.h>
#include <SD.h>
#include "Inventory.h"
#include "Item.h"
#include "Entities.h"

// Path of the save file on the Teensy's SD card
#define SAVE_FILE_PATH "/save.dat"

// XOR key for lightweight encryption (change to anything)
#define SAVE_XOR_KEY 0xA7

// ---- STRUCT THAT HOLDS ALL SAVE DATA ----

struct SaveData {
    uint32_t worldSeed;
    uint8_t currentDungeon;

    uint16_t playerX;
    uint16_t playerY;

    uint8_t hp;
    uint8_t food;

    //InventoryPage savedInventory[4];
    //Damsel damsel;
    bool succubusFriend;
    int attackDamage;
    bool endlessMode;
    int kills;
    float armorValue;
    //GameItem equippedArmor;
    bool equippedRiddleStone;
    int swiftnessRingsNum;
    int strengthRingsNum;
    int weaknessRingsNum;

    uint32_t checksum;        // Always last
};

bool saveGame(const SaveData& data);
bool loadGame(SaveData& outData);
bool deleteSave();
bool saveExists();
void cleanupMemory();

// Internal helpers
bool copyFile(const char* src, const char* dest);

#endif