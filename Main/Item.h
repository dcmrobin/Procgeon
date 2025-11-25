#ifndef ITEM_H 
#define ITEM_H

#include <Arduino.h>
#include "Dungeon.h"

#define NUM_POTIONS 20
#define NUM_SCROLLS 4
#define NUM_RINGS 6

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
  Null
};
enum ItemCategory { PotionCategory, FoodCategory, EquipmentCategory, ScrollsCategory };
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
  ScrollUncurseEffect
};

struct GameItem {
  GameItems item = Null;
  ItemCategory category = PotionCategory;
  char name[20] = "Null";
  int healthRecoverAmount = 0;
  int hungerRecoverAmount = 0;
  int AOEsize = 0;
  int AOEdamage = 0;
  float SpeedMultiplier = 0;
  char description[100] = "";
  char originalName[20] = "";
  char itemResult[100] = "";
  int rarity = 2;
  bool oneTimeUse = true;
  EffectType effectType = DefaultEffect;
  float armorValue = 0.0f;  // Damage reduction when equipped
  bool isEquipped = false;  // Whether this item is currently equipped
  bool isCursed = false;
  int curseChance = 0;
  bool canRust = false;
  bool isScrollRevealed = false;  // Whether the scroll's true name has been revealed
  int scrollEffectIndex = -1;  // Index of the assigned scroll effect
  // --- Ring support ---
  int ringEffectIndex = -1; // Index of the assigned ring effect
  int ringTypeIndex = -1; // Index of the ring's visible type (Wooden, Diamond, etc.)
  bool isRingIdentified = false;
};

// Possible potion effects
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

struct ScrollEffect {
  char effectName[20];
  char effectDescription[100];
  char effectResult[100];
  EffectType effectType;
};

// Structure to define item combinations (generalized from potions)
struct ItemCombination {
    GameItems ingredient1;
    GameItems ingredient2;
    GameItems result;
};

extern ItemCombination itemCombinations[];
extern const int NUM_ITEM_COMBINATIONS;

extern char scrollNames[][20];
extern PotionEffect potionEffects[];
extern GameItem itemList[32];

extern char ringTypes[NUM_RINGS][20];
extern char ringEffects[NUM_RINGS][100];
extern bool ringCursed[NUM_RINGS];

void randomizePotionEffects();  // Call this once at game start
void randomizeScrollEffects();  // Call this once at game start
void generateScrollName(char *name, size_t nameSize);  // Generate a random scroll name
GameItem getItem(GameItems item);
void updatePotionName(GameItem &potion);  // Changes potion name when used
void updateScrollName(GameItem &scroll);  // Reveals scroll's true name when read
GameItems getRandomPotion(int randInt, bool primaryColors);
void resetPotionNames();
void applyAOEEffect(float centerX, float centerY, int aoeRadius, int aoeDamage);
void renderItemResult();
bool areItemsEqual(GameItem item1, GameItem item2);
GameItem CombineTwoItemsToGetItem(GameItem item1, GameItem item2);
GameItem combineItems(GameItem item1, GameItem item2);
void randomizeRingEffects();
void updateRingName(GameItem &ring);

// Rarity-based item selection functions
GameItems getRandomItemByRarity(ItemCategory category, int maxRarity);
GameItems getRandomItemByRarityAnyCategory(int maxRarity);
TileTypes getRandomLootTile(int maxRarity);

#endif // ITEM_H
