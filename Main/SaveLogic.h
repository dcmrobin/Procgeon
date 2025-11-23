#ifndef SAVELOGIC_H
#define SAVELOGIC_H

#include <Arduino.h>
#include <SD.h>

// Path of the save file on the Teensy's SD card
#define SAVE_FILE_PATH "/save.dat"

// ---- STRUCT THAT HOLDS ALL SAVE DATA ----
struct SaveData {
    uint32_t worldSeed;
    uint8_t currentDungeon;
    uint16_t playerX;
    uint16_t playerY;
    uint8_t hp;
    uint8_t food;
    bool succubusFriend;
    int attackDamage;
    bool endlessMode;
    int kills;
    float armorValue;
    bool equippedRiddleStone;
    int swiftnessRingsNum;
    int strengthRingsNum;
    int weaknessRingsNum;
    uint32_t checksum;  // Simple integrity check
};

bool saveGame(const SaveData& data);
bool loadGame(SaveData& outData);
bool deleteSave();
bool saveExists();
void checkMemory();

#endif