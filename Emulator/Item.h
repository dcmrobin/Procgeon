#ifndef ITEM_H
#define ITEM_H

#include "Common.h"

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

struct WeaponItem {
    WeaponType  type        = NoWeapon;
    WeaponType  magicType   = NoWeapon;
    char        name[30]    = "No Weapon";
    char        description[110] = "Not a weapon.";
    bool        canRust     = false;
    float       damage      = 0.0f;
    int         attackDelay = DEFAULT_ATTACK_DELAY;
    int         range       = 1;
};

struct GameItem {
    GameItems   item        = Null;
    ItemCategory category   = PotionCategory;
    char        name[40]    = "Null";
    int         healthRecoverAmount = 0;
    int         hungerRecoverAmount = 0;
    int         AOEsize     = 0;
    int         AOEdamage   = 0;
    float       SpeedMultiplier = 0.0f;
    char        description[110] = "";
    char        originalName[20] = "";
    char        itemResult[150]  = "";
    int         rarity      = 2;
    bool        oneTimeUse  = true;
    EffectType  effectType  = DefaultEffect;
    float       armorValue  = 0.0f;
    bool        isEquipped  = false;
    bool        isCursed    = false;
    int         curseChance = 0;
    bool        canRust     = false;
    bool        isScrollRevealed = false;
    int         scrollEffectIndex = -1;
    int         ringEffectIndex   = -1;
    int         ringTypeIndex     = -1;
    bool        isRingIdentified  = false;
    WeaponItem  weapon      = {};
    int         stackCount  = 1;
};

struct PotionEffect {
    int         healthChange;
    int         AOEsize;
    int         AOEdamage;
    float       SpeedMultiplier;
    char        effectName[20];
    char        effectDescription[100];
    char        effectResult[100];
    EffectType  effectType;
};

struct ScrollEffect {
    char        effectName[20];
    char        effectDescription[100];
    char        effectResult[100];
    EffectType  effectType;
    bool        oneTimeUse;
};

struct ItemCombination {
    GameItems ingredient1;
    GameItems ingredient2;
    GameItems result;
};

// ─────────────────────────────────────────────────────────────────────────────
// Stacking helpers  (inline — no .cpp needed)
// ─────────────────────────────────────────────────────────────────────────────
inline bool isStackable(const GameItem& item) {
    if (item.item == Null)           return false;
    if (item.category == WeaponCategory) return false;
    if (item.item == Ring)           return false;
    if (item.item == RiddleStone)    return false;
    if (item.effectType == ArmorEffect) return false;
    return true;
}

inline bool canStackWith(const GameItem& a, const GameItem& b) {
    if (!isStackable(a) || !isStackable(b))             return false;
    if (a.item != b.item)                               return false;
    if (a.item == Scroll &&
        a.scrollEffectIndex != b.scrollEffectIndex)     return false;
    if (strcmp(a.name, b.name) != 0)                    return false;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Global data tables (defined in Item.cpp)
// ─────────────────────────────────────────────────────────────────────────────
extern ItemCombination  itemCombinations[];
extern PotionEffect     potionEffects[NUM_POTIONS];
extern GameItem         itemList[NUM_ITEMS];
extern WeaponItem       weaponList[NUM_WEAPONS];
extern ScrollEffect     scrollEffects[NUM_SCROLLS];

extern char scrollNames[NUM_SCROLLS][20];
extern char scrollNamesRevealed[NUM_SCROLLS][20];

extern char ringTypes[NUM_RINGS][20];
extern char ringEffects[NUM_RINGS][100];
extern bool ringCursed[NUM_RINGS];
extern char ringDescriptions[NUM_RINGS][100];

extern const char* loreTexts[NUM_LORE_TEXTS];

// ─────────────────────────────────────────────────────────────────────────────
// Item functions (defined in Item.cpp)
// ─────────────────────────────────────────────────────────────────────────────
void        randomizePotionEffects();
void        randomizeScrollEffects();
void        randomizeRingEffects();
void        generateScrollName(char* name, size_t nameSize);
GameItem    getItem(GameItems item);
void        updatePotionName(GameItem& potion);
void        updateScrollName(GameItem& scroll);
void        updateRingName(GameItem& ring);
GameItems   getRandomPotion(int randInt, bool primaryColors);
void        resetPotionNames();
void        applyAOEEffect(float centerX, float centerY, int aoeRadius, int aoeDamage);
void        renderItemResult();
bool        areItemsEqual(GameItem item1, GameItem item2);
GameItem    CombineTwoItemsToGetItem(GameItem item1, GameItem item2);
GameItem    combineItems(GameItem item1, GameItem item2);

GameItems   getRandomItemByRarity(ItemCategory category, int maxRarity);
GameItems   getRandomItemByRarityAnyCategory(int maxRarity);
TileTypes   getRandomLootTile(int maxRarity);

// ─────────────────────────────────────────────────────────────────────────────
// Equipped item state  (defined in GameState.cpp via Item.cpp extern)
// These live outside GameState because GameState.h cannot include Item.h
// without creating a circular dependency.
// ─────────────────────────────────────────────────────────────────────────────
extern GameItem equippedArmor;
extern GameItem equippedWeapon;
extern GameItem combiningItem1;
extern GameItem combiningItem2;

#endif // ITEM_H