#ifndef ITEM_H 
#define ITEM_H

#include <Arduino.h>

#define NUM_POTIONS 12
#define NUM_SCROLLS 3
#define NUM_RINGS 5

enum GameItems { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion, OrangePotion, PurplePotion, CyanPotion, MaroonPotion, DarkGreenPotion, LimePotion, Mushroom, EmptyBottle, RiddleStone, Scroll, WetScroll, Ring, LeatherArmor, IronArmor, MagicRobe, Cloak, Null };
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
  ArmorEffect,
  ScrollProtectionEffect,
  ScrollIdentifyEffect,
  ScrollEnchantEffect
};

struct GameItem {
  GameItems item = Null;
  ItemCategory category = PotionCategory;
  String name = "Null";
  int healthRecoverAmount = 0;
  int hungerRecoverAmount = 0;
  int AOEsize = 0;
  int AOEdamage = 0;
  float SpeedMultiplier = 0;
  String description = "";
  String originalName = "";
  String itemResult = "";
  bool oneTimeUse = true;
  EffectType effectType = DefaultEffect;
  int armorValue = 0;  // Damage reduction when equipped
  bool isEquipped = false;  // Whether this item is currently equipped
  bool isCursed = false;
  int curseChance = 0;
  bool isScrollRevealed = false;  // Whether the scroll's true name has been revealed
  int scrollEffectIndex = -1;  // Index of the assigned scroll effect
  // --- Ring support ---
  int ringEffectIndex = -1; // Index of the assigned ring effect
  bool isRingIdentified = false;
};

// Possible potion effects
struct PotionEffect {
  int healthChange;
  int AOEsize;
  int AOEdamage;
  float SpeedMultiplier;
  String effectName;
  String effectDescription;
  String effectResult;
  EffectType effectType;
};

struct ScrollEffect {
  String effectName;
  String effectDescription;
  String effectResult;
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

extern String scrollNames[];
extern PotionEffect potionEffects[];

extern String ringTypes[NUM_RINGS];
extern String ringEffects[NUM_RINGS];
extern bool ringCursed[NUM_RINGS];
extern bool ringIdentified[NUM_RINGS];

void randomizePotionEffects();  // Call this once at game start
void randomizeScrollEffects();  // Call this once at game start
String generateScrollName();  // Generate a random scroll name
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

#endif // ITEM_H
