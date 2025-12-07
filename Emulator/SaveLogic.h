#ifndef SAVELOGIC_H
#define SAVELOGIC_H

#include "Translation.h"
#include "Inventory.h"
#include "Item.h"
#include "Entities.h"
#include "Dungeon.h"

#define SAVE_FILE_PATH "savegame.dat"
#define SAVE_CHUNK_SIZE 256
#define FILE_WRITE "wb"
#define FILE_READ "rb"

struct SaveData {
    uint32_t worldSeed;
    uint8_t currentDungeon;
    uint16_t playerX;
    uint16_t playerY;
    uint8_t hp;
    uint8_t food;
    InventoryPage savedInventory[4];
    Damsel damsel;
    Enemy savedEnemies[30];
    bool succubusFriend;
    int attackDamage;
    bool endlessMode;
    int kills;
    float armorValue;
    GameItem equippedArmor;
    GameItem equippedWeapon; // Persist the full equipped weapon (including stats)
    bool equippedRiddleStone;
    int swiftnessRingsNum;
    int strengthRingsNum;
    int weaknessRingsNum;
    uint32_t checksum;
    TileTypes dungeonMap[64][64];
    char scrollNames[10][20];
    char scrollNamesRevealed[10][20];
    GameItem itemList[38];
    //WeaponItem weaponList[9];
    bool hasMap;
    bool playerNearClockEnemy;
    bool knowsDamselName;
    bool damselSayThanksForRescue;
    bool damselGotTaken;
    int levelOfDamselDeath;
    int hungerRingsNumber;
    int regenRingsNumber;
    int sicknessRingsNumber;
    int aggravateRingsNumber;
    int armorRingsNumber;
    int indigestionRingsNumber;
    int teleportRingsNumber;
    int invisibleRingsNumber;
    int keysCount;
};

bool saveGame(const SaveData& data);
bool loadGame(SaveData& outData);
bool deleteSave();
bool saveExists();
void stopAllAudio();
void resumeAudio();
#endif