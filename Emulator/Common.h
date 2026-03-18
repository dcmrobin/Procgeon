#ifndef COMMON_H
#define COMMON_H

// ─────────────────────────────────────────────────────────────────────────────
// Common.h
//
// Every game source file should include ONLY this header.
// It pulls in:
//   • Translation.h  — Arduino→SDL2 platform shims (types, millis, SD, Serial…)
//   • Constants.h    — All magic numbers / tuning values
//   • Shared enums and struct forward-declarations used across modules
//
// Individual module headers (Player.h, Dungeon.h, etc.) are NOT included here;
// they are included only where actually needed, keeping compile times fast and
// dependency directions clear.
// ─────────────────────────────────────────────────────────────────────────────

#include "Translation.h"   // platform adapter — must come first
#include "Constants.h"     // game-wide tuning constants

// ─────────────────────────────────────────────────────────────────────────────
// Display object — defined in Adafruit_SSD1327_emu.cpp, used everywhere
// ─────────────────────────────────────────────────────────────────────────────
#include "Adafruit_SSD1327_emu.h"
#include "Adafruit_GFX_emu.h"
extern Adafruit_SSD1327 display;

// ─────────────────────────────────────────────────────────────────────────────
// Tile types  (needed by Dungeon, Player, Entities, HelperFunctions, Item …)
// Defined here so every module sees the same enum without pulling in Dungeon.h.
// ─────────────────────────────────────────────────────────────────────────────
enum TileTypes {
    StartStairs,
    Floor,
    Wall,
    Kiosk,
    ShopWall,
    Bars,
    DoorClosed,
    DoorOpen,
    Exit,
    KeyTile,
    KeyItem,
    Freedom,
    Potion,
    Map,
    MushroomTile,
    RiddleStoneTile,
    ArmorTile,
    ScrollTile,
    RingTile,
    ChestTile,
    WeaponTile,
    GoldTile
};

// ─────────────────────────────────────────────────────────────────────────────
// UI state  (needed by HelperFunctions, Player, Inventory, Puzzles …)
// ─────────────────────────────────────────────────────────────────────────────
enum UIState {
    UI_NORMAL,
    UI_INVENTORY,
    UI_MINIMAP,
    UI_ITEM_ACTION,
    UI_ITEM_INFO,
    UI_ITEM_RESULT,
    UI_PAUSE,
    UI_RIDDLE,
    UI_PICROSS,
    UI_LIGHTSOUT,
    UI_SPLASH,
    UI_INTRO,
    UI_SECRET
};

// ─────────────────────────────────────────────────────────────────────────────
// Boss states  (needed by game loop and Entities)
// ─────────────────────────────────────────────────────────────────────────────
enum BossStates {
    Idle,
    Floating,
    Shooting,
    Summoning,
    Enraged,
    Beaten
};

// ─────────────────────────────────────────────────────────────────────────────
// Item / weapon enums  (needed by Item, Inventory, Player, SaveLogic …)
// ─────────────────────────────────────────────────────────────────────────────
enum ItemCategory {
    PotionCategory,
    FoodCategory,
    EquipmentCategory,
    ScrollsCategory,
    WeaponCategory
};

enum WeaponType {
    Sword,
    LongSword,
    Staff,
    Dagger,
    MagicStaff,
    MagicDagger,
    MagicSword,
    MagicLongSword,
    NoWeapon
};

enum EffectType {
    DefaultEffect,
    HealingEffect,
    PoisonEffect,
    ExplosionEffect,
    BuffingEffect,
    MegaHealEffect,
    BadEffect,
    SpeedEffect,
    SlowEffect,
    HungerEffect,
    SeeAllEffect,
    ConfusionEffect,
    RidiculeEffect,
    NoEffect,
    GlamourEffect,
    ChaosEffect,
    BlindnessEffect,
    StrengthEffect,
    RestoreEffect,
    ParalysisEffect,
    VeryPoisonEffect,
    ArmorEffect,
    ScrollProtectionEffect,
    ScrollIdentifyEffect,
    ScrollEnchantEffect,
    ScrollUncurseEffect,
    ScrollEmptyEffect,
    ScrollMapEffect,
    ScrollAmnesiaEffect,
    ScrollAggravateEffect,
    ScrollDestroyEffect,
    ScrollTeleportEffect,
    ScrollLoreEffect,
    WeaponEffect,
    StaleMeatEffect
};

enum GameItems {
    RedPotion,
    GreenPotion,
    BluePotion,
    BlackPotion,
    WhitePotion,
    YellowPotion,
    OrangePotion,
    PurplePotion,
    CyanPotion,
    MaroonPotion,
    DarkGreenPotion,
    LimePotion,
    GreyPotion,
    OlivePotion,
    CreamPotion,
    NavyPotion,
    AzurePotion,
    MintPotion,
    SalmonPotion,
    BrownPotion,
    Mushroom,
    Bread,
    Cheese,
    StaleMeat,
    Berries,
    EmptyBottle,
    RiddleStone,
    Scroll,
    WetScroll,
    Ring,
    LeatherArmor,
    IronArmor,
    MagicRobe,
    Cloak,
    Trenchcoat,
    DenimJacket,
    RingMailArmor,
    ChaosArmor,
    SpikyArmor,
    KingArmor,
    Weapon,
    Null
};

// ─────────────────────────────────────────────────────────────────────────────
// Konami code input enum  (needed by HelperFunctions + game loop)
// ─────────────────────────────────────────────────────────────────────────────
enum KonamiInput {
    K_UP,
    K_DOWN,
    K_LEFT,
    K_RIGHT,
    K_B,
    K_A,
    K_START
};

// ─────────────────────────────────────────────────────────────────────────────
// Map dimensions — aliases kept for source compatibility
// (the #defines are in Constants.h; these just make the intent obvious
//  when used as array sizes in extern declarations)
// ─────────────────────────────────────────────────────────────────────────────
static constexpr int mapWidth  = MAP_WIDTH;
static constexpr int mapHeight = MAP_HEIGHT;
static constexpr int tileSize  = TILE_SIZE;

#endif // COMMON_H