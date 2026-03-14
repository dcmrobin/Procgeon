#ifndef SAVELOGIC_H
#define SAVELOGIC_H

#include <stdint.h>
#include "Inventory.h"
#include "Item.h"
#include "Entities.h"
#include "Dungeon.h"

// ── File / chunk constants ───────────────────────────────────────────────────
#define SAVE_FILE_PATH  "savegame.dat"
#define SAVE_CHUNK_SIZE 256
#define FILE_WRITE      "wb"
#define FILE_READ       "rb"

// Bump whenever SaveData layout changes so old files are cleanly rejected.
#define SAVE_VERSION 2

// "SVDT" as a 32-bit magic number (little-endian: 'S','V','D','T').
static const uint32_t SAVE_MAGIC = 0x54445653u;

// ── SavedEnemy ───────────────────────────────────────────────────────────────
// A pointer-free snapshot of Enemy.
//
// Enemy contains  const unsigned char* sprite  which is a runtime address.
// Writing that address to disk and reading it back in a new session gives a
// dangling pointer → renderEnemies() crashes within the first rendered frame.
// We exclude the sprite here and restore it from the name string on load.
//
// Enemy also contains  PathNode wanderPath[32]  which is plain data and is
// safe to serialise directly.
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
    // sprite           ← intentionally omitted; restored by name on load
    int      attackDelayCounter;
    bool     nearClock;
    bool     isFriend;
};

// ── SaveData ─────────────────────────────────────────────────────────────────
// Complete game state for one save slot.
//
// Type fixes vs. the old struct:
//   hp / food   uint8_t  → int16_t   (playerHP can be < 0 or > 255)
//   playerX/Y   uint16_t → float     (preserves sub-tile position)
//   savedEnemies Enemy[] → SavedEnemy[] (no raw pointers)
//
// IMPORTANT: checksum must remain the last field that is *hashed*.
// Everything in the struct after it (dungeonMap, scrollNames, itemList) is
// still written to disk but not included in the hash, so keep that ordering.
// If you add new fields, add them before checksum or after itemList and
// update the hash boundary accordingly (see calculateChecksum in .cpp).
struct SaveData {
    // ── Header (validated before anything else) ──────────────────────────
    uint32_t magic;           // must equal SAVE_MAGIC
    uint8_t  version;         // must equal SAVE_VERSION

    // ── World ─────────────────────────────────────────────────────────────
    uint32_t worldSeed;
    uint8_t  currentDungeon;

    // ── Player ────────────────────────────────────────────────────────────
    float    playerX;         // was uint16_t – float keeps sub-tile position
    float    playerY;
    int16_t  hp;              // was uint8_t  – supports negative values & >255
    int16_t  food;            // was uint8_t

    // ── Inventory / entities ──────────────────────────────────────────────
    InventoryPage savedInventory[5];   // 5 tabs: Potions, Food, Equipment, Scrolls, Weapons
    Damsel        damsel;
    SavedEnemy    savedEnemies[30];    // pointer-free; sprite re-derived on load

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

    // ── Large data (written to disk but not included in checksum hash) ────
    TileTypes dungeonMap[64][64];
    char      scrollNames[10][20];
    char      scrollNamesRevealed[10][20];
    GameItem  itemList[38];
};

// ── Public API ───────────────────────────────────────────────────────────────
bool saveGame(const SaveData& data);
bool loadGame(SaveData& outData);
bool deleteSave();
bool saveExists();

// Audio helpers called internally by save/load to silence the SD card I/O.
void stopAllAudio();
void resumeAudio();

#endif // SAVELOGIC_H