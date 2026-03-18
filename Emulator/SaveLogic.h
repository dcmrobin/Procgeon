#ifndef SAVELOGIC_H
#define SAVELOGIC_H

#include "Common.h"
#include "Inventory.h"
#include "Item.h"
#include "Entities.h"
#include "Dungeon.h"
#include "Shop.h"

// ─────────────────────────────────────────────────────────────────────────────
// Save version — bump whenever SaveData layout changes
// v2: fixed pointer serialisation bug
// v3: added stackCount to GameItem
// v4: added goldCount; NUM_ITEMS 38→42; NUM_SCROLLS 10→11
// ─────────────────────────────────────────────────────────────────────────────
#define SAVE_MAGIC   0x54445653u   // "SVDT" little-endian

// ─────────────────────────────────────────────────────────────────────────────
// SavedEnemy — pointer-free snapshot of Enemy
// ─────────────────────────────────────────────────────────────────────────────
struct SavedEnemy {
    float    x, y;
    int      hp;
    bool     chasingPlayer;
    float    moveAmount;
    char     name[30];
    int      attackDelay;
    int      damage;
    bool     hasWanderPath;
    int      pathLength;
    int      currentPathIndex;
    PathNode wanderPath[ASTAR_MAX_NODES];
    int      attackDelayCounter;
    bool     nearClock;
    bool     isFriend;
};

// ─────────────────────────────────────────────────────────────────────────────
// SaveData
// ─────────────────────────────────────────────────────────────────────────────
struct SaveData {
    // Header
    uint32_t magic;
    uint8_t  version;

    // World
    uint32_t worldSeed;
    uint8_t  currentDungeon;

    // Player
    float    playerX, playerY;
    int16_t  hp, food;

    // Inventory / entities
    InventoryPage savedInventory[NUM_INVENTORY_PAGES];
    Damsel        damsel;
    SavedEnemy    savedEnemies[MAX_ENEMIES];

    // Equipment / combat
    float    armorValue;
    int      attackDamage;
    GameItem equippedArmor;
    GameItem equippedWeapon;
    bool     equippedRiddleStone;
    ShopItem shopItems[SHOP_MAX_ITEMS];

    // Flags
    bool     endlessMode;
    bool     succubusFriend;
    bool     hasMap;
    bool     playerNearClockEnemy;
    bool     knowsDamselName;
    bool     damselSayThanksForRescue;
    bool     damselGotTaken;
    int      levelOfDamselDeath;
    int      kills;
    int      keysCount;
    int      goldCount;

    // Ring counters
    int      swiftnessRingsNum;
    int      strengthRingsNum;
    int      weaknessRingsNum;
    int      hungerRingsNumber;
    int      regenRingsNumber;
    int      sicknessRingsNumber;
    int      aggravateRingsNumber;
    int      armorRingsNumber;
    int      indigestionRingsNumber;
    int      teleportRingsNumber;
    int      invisibleRingsNumber;

    // Integrity — MUST stay here; everything above is hashed
    uint32_t checksum;

    // Large data (written to disk but not hashed)
    TileTypes dungeonMap[MAP_HEIGHT][MAP_WIDTH];
    char      scrollNames[NUM_SCROLLS][20];
    char      scrollNamesRevealed[NUM_SCROLLS][20];
    GameItem  itemList[NUM_ITEMS];
};

// ─────────────────────────────────────────────────────────────────────────────
// Functions
// ─────────────────────────────────────────────────────────────────────────────
bool saveGame(const SaveData& data);
bool loadGame(SaveData& outData);
bool deleteSave();
bool saveExists();

void stopAllAudio();
void resumeAudio();

#endif // SAVELOGIC_H