
#ifndef ITEM_H 
#define ITEM_H
#include <string>


#define NUM_POTIONS 19
#define NUM_SCROLLS 3
#define NUM_RINGS 5

enum GameItems { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion, OrangePotion, PurplePotion, CyanPotion, MaroonPotion, DarkGreenPotion, LimePotion, GreyPotion, OlivePotion, CreamPotion, NavyPotion, AzurePotion, MintPotion, SalmonPotion, Mushroom, EmptyBottle, RiddleStone, Scroll, WetScroll, Ring, LeatherArmor, IronArmor, MagicRobe, Cloak, Null };
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
  ArmorEffect,
  ScrollProtectionEffect,
  ScrollIdentifyEffect,
  ScrollEnchantEffect
};

struct GameItem {
  GameItems item = Null;
  ItemCategory category = PotionCategory;
  std::string name = "Null";
  GameItem() : item(Null), category(PotionCategory), name("Empty") {}
  GameItem(GameItems i, ItemCategory c, const std::string& n) : item(i), category(c), name(n) {}
  GameItem(
    GameItems i,
    ItemCategory c,
    const std::string& n,
    int hp,
    int hunger,
    int mp,
    int xp,
    int gold,
    const std::string& desc,
    const std::string& displayName,
    const std::string& useResult,
    bool cursed = false,
    EffectType effect = DefaultEffect,
    int effectValue = 0,
    bool identified = false,
    bool equipped = false,
    int stackSize = 1
  ) :
    item(i),
    category(c),
    name(n),
    healthRecoverAmount(hp),
    hungerRecoverAmount(hunger),
    AOEsize(mp),
    AOEdamage(xp),
    armorValue(gold),
    description(desc),
    originalName(displayName),
    itemResult(useResult),
    isCursed(cursed),
    effectType(effect),
    curseChance(effectValue),
    isScrollRevealed(identified),
    isEquipped(equipped),
    ringEffectIndex(stackSize)
  {}
  int healthRecoverAmount = 0;
  int hungerRecoverAmount = 0;
  int AOEsize = 0;
  int AOEdamage = 0;
  float SpeedMultiplier = 0;
  std::string description = "";
  std::string originalName = "";
  std::string itemResult = "";
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
  std::string effectName;
  std::string effectDescription;
  std::string effectResult;
  EffectType effectType;
};

struct ScrollEffect {
  std::string effectName;
  std::string effectDescription;
  std::string effectResult;
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

extern std::string scrollNames[];
extern PotionEffect potionEffects[];

extern std::string ringTypes[NUM_RINGS];
extern std::string ringEffects[NUM_RINGS];
extern bool ringCursed[NUM_RINGS];
extern bool ringIdentified[NUM_RINGS];

void randomizePotionEffects();  // Call this once at game start
void randomizeScrollEffects();  // Call this once at game start
// std::string generateScrollName();  // Generate a random scroll name
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