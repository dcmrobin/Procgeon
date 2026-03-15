#ifndef SAVELOGIC_H
#define SAVELOGIC_H

#include <stdint.h>
#include "Inventory.h"
#include "Item.h"
#include "Entities.h"
#include "Dungeon.h"

#define SAVE_FILE_PATH  "savegame.dat"
#define SAVE_CHUNK_SIZE 256
#define FILE_WRITE      "wb"
#define FILE_READ       "rb"

// Bump whenever SaveData layout changes so old files are cleanly rejected.
// v2: fixed pointer serialisation bug
// v3: added stackCount to GameItem
// v4: added goldCount; NUM_ITEMS 38->42; NUM_SCROLLS 10->11; ScrollEffect gained oneTimeUse field
#define SAVE_VERSION 4

// "SVDT" as a 32-bit magic number (little-endian: 'S','V','D','T').
static const uint32_t SAVE_MAGIC = 0x54445653u;

// ── SavedEnemy ────────────────────────────────────────────────────────────────
// Pointer-free snapshot of Enemy (sprite excluded, restored by name on load).
struct SavedEnemy {
    float    x;
    float    y;
    int      hp;
    bool     chasingPlayer;
    float    moveAmount;
    char     name[30];
    int      attackDelay;
    int      damage;
    bool     hasWanderPath;
    int      pathLength;
    int      currentPathIndex;
    PathNode wanderPath[32];
    int      attackDelayCounter;
    bool     nearClock;
    bool     isFriend;
};

// ── SaveData ──────────────────────────────────────────────────────────────────
struct SaveData {
    // ── Header ────────────────────────────────────────────────────────────
    uint32_t magic;
    uint8_t  version;

    // ── World ─────────────────────────────────────────────────────────────
    uint32_t worldSeed;
    uint8_t  currentDungeon;

    // ── Player ────────────────────────────────────────────────────────────
    float    playerX;
    float    playerY;
    int16_t  hp;
    int16_t  food;

    // ── Inventory / entities ──────────────────────────────────────────────
    InventoryPage savedInventory[5];
    Damsel        damsel;
    SavedEnemy    savedEnemies[30];

    // ── Equipment / combat ────────────────────────────────────────────────
    float    armorValue;
    int      attackDamage;
    GameItem equippedArmor;
    GameItem equippedWeapon;
    bool     equippedRiddleStone;

    // ── Flags ─────────────────────────────────────────────────────────────
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
    int      goldCount;   // added v4

    // ── Ring counters ─────────────────────────────────────────────────────
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

    // ── Integrity check ───────────────────────────────────────────────────
    // MUST stay here: calculateChecksum() hashes every byte up to this field.
    uint32_t checksum;

    // ── Large data (written to disk but not hashed) ───────────────────────
    TileTypes dungeonMap[64][64];
    char      scrollNames[NUM_SCROLLS][20];          // [11][20]
    char      scrollNamesRevealed[NUM_SCROLLS][20];  // [11][20]
    GameItem  itemList[NUM_ITEMS];                   // [42]
};

bool saveGame(const SaveData& data);
bool loadGame(SaveData& outData);
bool deleteSave();
bool saveExists();

void stopAllAudio();
void resumeAudio();

#endif // SAVELOGIC_H