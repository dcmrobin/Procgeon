#ifndef ITEM_H 
#define ITEM_H

#include <Arduino.h>

#define NUM_POTIONS 11
#define NUM_SCROLLS 3

enum GameItems { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion, OrangePotion, PurplePotion, CyanPotion, MaroonPotion, DarkGreenPotion, Mushroom, EmptyBottle, RiddleStone, Scroll, WetScroll, LeatherArmor, IronArmor, MagicRobe, Cloak, Null };
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

// Structure to define potion combinations
struct PotionCombination {
    GameItems ingredient1;
    GameItems ingredient2;
    GameItems result;
};

extern String scrollNames[];
extern PotionEffect potionEffects[];
extern PotionCombination potionCombinations[];
extern const int NUM_POTION_COMBINATIONS;

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
GameItem combinePotions(GameItem item1, GameItem item2);

#endif // ITEM_H
