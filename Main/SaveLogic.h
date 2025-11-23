#ifndef SAVELOGIC_H
#define SAVELOGIC_H

#include <Arduino.h>
#include <SD.h>

#define SAVE_FILE_PATH "/save.txt"

// Individual save functions
bool saveGame(uint32_t worldSeed, uint8_t currentDungeon, uint16_t playerX, uint16_t playerY,
              uint8_t hp, uint8_t food, bool succubusFriend, int attackDamage, bool endlessMode,
              int kills, float armorValue, bool equippedRiddleStone, int swiftnessRingsNum,
              int strengthRingsNum, int weaknessRingsNum);

bool loadGame(uint32_t& worldSeed, uint8_t& currentDungeon, uint16_t& playerX, uint16_t& playerY,
              uint8_t& hp, uint8_t& food, bool& succubusFriend, int& attackDamage, bool& endlessMode,
              int& kills, float& armorValue, bool& equippedRiddleStone, int& swiftnessRingsNum,
              int& strengthRingsNum, int& weaknessRingsNum);

bool deleteSave();
bool saveExists();

#endif