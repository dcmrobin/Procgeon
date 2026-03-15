#ifndef ITEM_H 
#define ITEM_H

#include <string>
#include "Dungeon.h"
#include "Translation.h"

#define NUM_POTIONS 20
#define NUM_SCROLLS 11        // +1 for lore scroll effect
#define NUM_RINGS 12
#define NUM_ITEMS 42          // +4 for Bread, Cheese, StaleMeat, Berries
#define NUM_WEAPONS 9
#define MAX_STACK_SIZE 7
#define NUM_LORE_TEXTS 20     // Fill loreTexts[] in Item.cpp with your story

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
  Bread,        // moderate hunger restore
  Cheese,       // small hunger restore, common
  StaleMeat,    // good hunger restore, small HP penalty
  Berries,      // small hunger restore, rare
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

enum ItemCategory { PotionCategory, FoodCategory, EquipmentCategory, ScrollsCategory, WeaponCategory };

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
  ScrollLoreEffect,   // Not consumed on read; shows lore text
  WeaponEffect,
  StaleMeatEffect     // Hunger restore + small HP penalty
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
  NoWeapon,
};

struct WeaponItem {
  WeaponType type = NoWeapon;
  WeaponType magicType = NoWeapon;
  char name[30] = "No Weapon";
  char description[110] = "Not a weapon.";
  bool canRust = false;
  float damage = 0;
  int attackDelay = 10;
  int range = 1;
};

struct GameItem {
  GameItems item = Null;
  ItemCategory category = PotionCategory;
  char name[40] = "Null";
  int healthRecoverAmount = 0;
  int hungerRecoverAmount = 0;
  int AOEsize = 0;
  int AOEdamage = 0;
  float SpeedMultiplier = 0;
  char description[110] = "";
  char originalName[20] = "";
  char itemResult[150] = "";
  int rarity = 2;
  bool oneTimeUse = true;
  EffectType effectType = DefaultEffect;
  float armorValue = 0.0f;
  bool isEquipped = false;
  bool isCursed = false;
  int curseChance = 0;
  bool canRust = false;
  bool isScrollRevealed = false;
  int scrollEffectIndex = -1;
  int ringEffectIndex = -1;
  int ringTypeIndex = -1;
  bool isRingIdentified = false;
  WeaponItem weapon = {};
  int stackCount = 1;
};

// Returns true if this item type is allowed to stack.
inline bool isStackable(const GameItem& item) {
  if (item.item == Null) return false;
  if (item.category == WeaponCategory) return false;
  if (item.item == Ring) return false;
  if (item.item == RiddleStone) return false;
  if (item.effectType == ArmorEffect) return false;
  return true;
}

// Returns true if two items can be merged into the same stack.
inline bool canStackWith(const GameItem& a, const GameItem& b) {
  if (!isStackable(a) || !isStackable(b)) return false;
  if (a.item != b.item) return false;
  if (a.item == Scroll && a.scrollEffectIndex != b.scrollEffectIndex) return false;
  if (strcmp(a.name, b.name) != 0) return false;
  return true;
}

struct PotionEffect {
  int healthChange;
  int AOEsize;
  int AOEdamage;
  float SpeedMultiplier;
  char effectName[20];
  char effectDescription[100];
  char effectResult[100];
  EffectType effectType;
};

// oneTimeUse added — false for lore scraps, true for everything else
struct ScrollEffect {
  char effectName[20];
  char effectDescription[100];
  char effectResult[100];
  EffectType effectType;
  bool oneTimeUse;
};

struct ItemCombination {
  GameItems ingredient1;
  GameItems ingredient2;
  GameItems result;
};

extern ItemCombination itemCombinations[];
extern const int NUM_ITEM_COMBINATIONS;

extern char scrollNames[NUM_SCROLLS][20];
extern char scrollNamesRevealed[NUM_SCROLLS][20];
extern PotionEffect potionEffects[NUM_POTIONS];
extern GameItem itemList[NUM_ITEMS];
extern WeaponItem weaponList[NUM_WEAPONS];
extern ScrollEffect scrollEffects[NUM_SCROLLS];

extern char ringTypes[NUM_RINGS][20];
extern char ringEffects[NUM_RINGS][100];
extern bool ringCursed[NUM_RINGS];
extern char ringDescriptions[NUM_RINGS][100];

// Lore scrap texts — fill these in Item.cpp with your dungeon's story.
extern const char* loreTexts[NUM_LORE_TEXTS];

void randomizePotionEffects();
void randomizeScrollEffects();
void generateScrollName(char *name, size_t nameSize);
GameItem getItem(GameItems item);
void updatePotionName(GameItem &potion);
void updateScrollName(GameItem &scroll);
GameItems getRandomPotion(int randInt, bool primaryColors);
void resetPotionNames();
void applyAOEEffect(float centerX, float centerY, int aoeRadius, int aoeDamage);
void renderItemResult();
bool areItemsEqual(GameItem item1, GameItem item2);
GameItem CombineTwoItemsToGetItem(GameItem item1, GameItem item2);
GameItem combineItems(GameItem item1, GameItem item2);
void randomizeRingEffects();
void updateRingName(GameItem &ring);

GameItems getRandomItemByRarity(ItemCategory category, int maxRarity);
GameItems getRandomItemByRarityAnyCategory(int maxRarity);
TileTypes getRandomLootTile(int maxRarity);

#endif // ITEM_H