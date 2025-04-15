#ifndef ITEM_H 
#define ITEM_H

#include <Arduino.h>

#define NUM_POTIONS 10

enum GameItems { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion, OrangePotion, PurplePotion, CyanPotion, MaroonPotion, Mushroom, EmptyBottle, RiddleStone, Null };
enum ItemCategory { PotionCategory, FoodCategory, EquipmentCategory };

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
};

extern PotionEffect potionEffects[];

void randomizePotionEffects();  // Call this once at game start
GameItem getItem(GameItems item);
void updatePotionName(GameItem &potion);  // Changes potion name when used
GameItems getRandomPotion(int randInt);
void resetPotionNames();
void applyAOEEffect(float centerX, float centerY, int aoeRadius, int aoeDamage);
void renderItemResult();
bool areItemsEqual(GameItem item1, GameItem item2);
GameItem CombineTwoItemsToGetItem(GameItem item1, GameItem item2);

#endif // ITEM_H
