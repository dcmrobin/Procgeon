
#include <algorithm>
#include "Item.h"
#include "Inventory.h"
#include "HelperFunctions.h"
#include "Player.h"

extern int numInventoryPages;

GameItem itemList[] = {
  { RedPotion, PotionCategory, std::string("Red Potion"),  0,  0,  0, 0, 0, std::string("Drink it to find out."), std::string("Red Potion"), std::string("Nothing happens.")},
  { GreenPotion, PotionCategory, std::string("Green Potion"), 0,  0,  0, 0, 0, std::string("Drink it to find out."), std::string("Green Potion"), std::string("Nothing happens.")},
  { BluePotion,  PotionCategory, std::string("Blue Potion"),  0,  0,  0, 0, 0, std::string("Drink it to find out."), std::string("Blue Potion"), std::string("Nothing happens.")},
  { BlackPotion, PotionCategory, std::string("Black Potion"), 0,  0,  0, 0, 0, std::string("Drink it to find out."), std::string("Black Potion"), std::string("Nothing happens.")},
  { WhitePotion, PotionCategory, std::string("White Potion"), 0,  0,  0, 0, 0, std::string("Drink it to find out."), std::string("White Potion"), std::string("Nothing happens.")},
  { YellowPotion, PotionCategory, std::string("Yellow Potion"), 0,  0,  0, 0, 0, std::string("Drink it to find out."), std::string("Yellow Potion"), std::string("Nothing happens.")},
  { OrangePotion, PotionCategory, std::string("Orange Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Orange Potion"), std::string("Nothing happens.")},
  { PurplePotion, PotionCategory, std::string("Purple Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Purple Potion"), std::string("Nothing happens.")},
  { CyanPotion, PotionCategory, std::string("Cyan Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Cyan Potion"), std::string("Nothing happens.")},
  { MaroonPotion, PotionCategory, std::string("Maroon Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Maroon Potion"), std::string("Nothing happens.")},
  { DarkGreenPotion, PotionCategory, std::string("DarkGreen Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("DarkGreen Potion"), std::string("Nothing happens.")},
  { LimePotion, PotionCategory, std::string("Lime Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Lime Potion"), std::string("Nothing happens.")},
  { GreyPotion, PotionCategory, std::string("Grey Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Grey Potion"), std::string("Nothing happens.")},
  { OlivePotion, PotionCategory, std::string("Olive Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Olive Potion"), std::string("Nothing happens.")},
  { CreamPotion, PotionCategory, std::string("Cream Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Cream Potion"), std::string("Nothing happens.")},
  { NavyPotion, PotionCategory, std::string("Navy Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Navy Potion"), std::string("Nothing happens.")},
  { AzurePotion, PotionCategory, std::string("Azure Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Azure Potion"), std::string("Nothing happens.")},
  { MintPotion, PotionCategory, std::string("Mint Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Mint Potion"), std::string("Nothing happens.")},
  { SalmonPotion, PotionCategory, std::string("Salmon Potion"), 0,  0,  0,  0, 0, std::string("Drink it to find out."), std::string("Salmon Potion"), std::string("Nothing happens.")},
  { Mushroom, FoodCategory, std::string("Mushroom"), 0,  20,  0, 0, 0, std::string("It is edible."), std::string("Mushroom"), std::string("You become less hungry.")},
  { EmptyBottle, PotionCategory, std::string("Empty Bottle"), 0,  20,  0, 0, 0, std::string("It is an empty bottle."), std::string("Empty Bottle"), std::string("Nothing happens. It's an empty bottle."), false},
  { RiddleStone, EquipmentCategory, std::string("Riddle Stone"), 0,  0,  0, 0, 0, std::string("Looks like it could be used for many things..."), std::string("Riddle Stone"), std::string("Solve this riddle!"), true, DefaultEffect, 0, false, false, 2},
  { Scroll, ScrollsCategory, std::string("Scroll"), 0,  0,  0, 0, 0, std::string("Read it to find out."), std::string("Scroll"), std::string("Nothing happens.")},
  { WetScroll, ScrollsCategory, std::string("Wet Scroll"), 0,  0,  0, 0, 0, std::string("A scroll that is too wet to read."), std::string("Wet Scroll"), std::string("The scroll is too wet to read. Nothing happens."), false},
  { Ring, EquipmentCategory, std::string("Ring"), 0,  0,  0, 0, 0, std::string("Put it on to find out."), std::string("Ring"), std::string("You equip the ring."), false},
  { LeatherArmor, EquipmentCategory, std::string("Leather Armor"), 0,  0,  0, 0, 0, std::string("Basic leather armor. Reduces damage by 2."), std::string("Leather Armor"), std::string("You equip the leather armor."), false, ArmorEffect, 2, false, false, 2},
  { IronArmor, EquipmentCategory, std::string("Iron Armor"), 0,  0,  0, 0, 0, std::string("Sturdy iron armor. Reduces damage by 3."), std::string("Iron Armor"), std::string("You equip the iron armor."), false, ArmorEffect, 3, false, false, 2},
  { MagicRobe, EquipmentCategory, std::string("Magic Robe"), 0,  0,  0, 0, 0, std::string("Enchanted robe. Reduces damage by 1 and increases magic resistance."), std::string("Magic Robe"), std::string("You equip the magic robe."), false, ArmorEffect, 1, false, false, 2},
  { Cloak, EquipmentCategory, std::string("Cloak"), 0,  0,  0, 0, 0, std::string("A simple cloth cloak. Provides no protection but keeps you warm."), std::string("Cloak"), std::string("You equip the cloak."), false, ArmorEffect, 0, false, false, 2},
  { Null, PotionCategory, std::string("Null"), 0, 0, 0, 0, 0, std::string(""), std::string("Null"), std::string(""), false }
};

std::string scrollNames[] = {
  "null",
  "null",
  "null"
};

// Possible effect pool
PotionEffect potionEffects[] = {// do not change any of the effectresult strings, as they are used for comparing effects
  { 20,  0,  0, 0, std::string("Healing Potion"), std::string("Healing. Heals 20 of your HP."), std::string("You feel better."), HealingEffect },
  { -20, 0,  0, 0, std::string("Diluted Poison"), std::string("Deducts 20 of your HP. Don't drink. Unless your guilty of something..."), std::string("You lose 20 HP."), PoisonEffect },
  { 0,   4, 40, 0, std::string("Explosion Potion"), std::string("Bomb. Deals 40 damage to enemies around you."), std::string("The enemies around you lose 40 HP."), ExplosionEffect },
  { 40,   4, -30, 0, std::string("Buffing Potion"), std::string("Heals 40 of your HP, but also heals 30 HP of enemies around you."), std::string("You feel better, but so do the enemies close to you."), BuffingEffect },
  { 70,  0,  0, 0, std::string("Mega Heal Potion"), std::string("Healing, but mega. Heals 70 of your HP."), std::string("You feel much better."), MegaHealEffect },
  { -50,  4,  -20, 0, std::string("Bad Potion"), std::string("It deducts 50 of your HP, and gives enemies around you 20 HP. Maybe don't drink this."), std::string("You lose 50 HP, and the enemies around you gain 20 HP."), BadEffect },
  { 0,  0,  0, 1, std::string("Speed Potion"), std::string("Drink this, and you'll go twice as fast."), std::string("Your are faster now, but only for a limited amount of time."), SpeedEffect },
  { 0,  0,  0, -0.4, std::string("Slowing Potion"), std::string("Drink this, and you'll go half as fast."), std::string("Your are slower now, but only for a limited amount of time."), SlowEffect },
  { 0,  0,  0, 0, std::string("Hunger Potion"), std::string("Makes you more hungry."), std::string("You are now more hungry."), HungerEffect },
  { 0,  0,  0, 0, std::string("See-all Potion"), std::string("Opens your eyes to the unseen."), std::string("You can now see that which was unseen for a limited time."), SeeAllEffect },
  { 0,  0,  0, 0, std::string("Confusion Potion"), std::string("You go in the opposite direction to the direction you want to go."), std::string("What is going on?"), ConfusionEffect },
  { 0,  0,  0, 0, std::string("Ridicule Potion"), std::string("Drinking this makes you feel stupid."), std::string("You feel stupid."), RidiculeEffect },
  { 0,  0,  0, 0, std::string("Bland Potion"), std::string("Colored liquid that does nothing."), std::string("Nothing happens."), NoEffect },
  { 0,  0,  0, 0, std::string("Glamour Potion"), std::string("Drinking this makes you feel awesome."), std::string("You feel fabulous!"), GlamourEffect},
  { 0,  0,  0, 0, std::string("Chaos Potion"), std::string("The effect of this potion is random."), std::string("A lot happens."), ChaosEffect},
  { 0,  0,  0, 0, std::string("Blindness Potion"), std::string("Makes you blind for a time. Do not drink this potion."), std::string("A cloak of darkness falls around you."), BlindnessEffect},
  { 0,  0,  0, 0, std::string("Strength Potion"), std::string("Makes you do more damage."), std::string("You feel stronger."), StrengthEffect},
  { 0,  0,  0, 0, std::string("Restore Potion"), std::string("Cures all your ailments."), std::string("You feel restored!"), RestoreEffect},
  { 0,  0,  0, 0, std::string("Paralysis Potion"), std::string("Paralyzes you for a time."), std::string("You can't move!"), ParalysisEffect},
};

ScrollEffect scrollEffects[] = {
  {std::string("Protect scroll"), std::string("Protects your armor from rusting and raises its damage reduction."), std::string("Your armor is covered by a shimmering gold shield!"), ScrollProtectionEffect},
  {std::string("Identify scroll"), std::string("Reveals the true name of an item and sees if it is cursed."), std::string("Select an item to identify."), ScrollIdentifyEffect},
  {std::string("Enchant scroll"), std::string("Makes an item better than it used to be."), std::string("Select an item to enchant."), ScrollEnchantEffect}
};

// Define all possible item combinations (generalized from potions)
ItemCombination itemCombinations[] = {
    {BluePotion, YellowPotion, GreenPotion},
    {RedPotion, GreenPotion, YellowPotion},
    {RedPotion, YellowPotion, OrangePotion},
    {RedPotion, BluePotion, PurplePotion},
    {GreenPotion, BluePotion, CyanPotion},
    {RedPotion, BlackPotion, MaroonPotion},
    {GreenPotion, BlackPotion, DarkGreenPotion},
    {GreenPotion, YellowPotion, LimePotion},
    {WhitePotion, BlackPotion, GreyPotion},
    {YellowPotion, BlackPotion, OlivePotion},
    {YellowPotion, WhitePotion, CreamPotion},
    {BluePotion, BlackPotion, NavyPotion},
    {BluePotion, WhitePotion, AzurePotion},
    {GreenPotion, WhitePotion, MintPotion},
    {RedPotion, WhitePotion, SalmonPotion}
    // Add more general item combinations here
};

const int NUM_ITEM_COMBINATIONS = sizeof(itemCombinations) / sizeof(itemCombinations[0]);

std::string ringTypes[NUM_RINGS] = { "Wooden Ring", "Emerald Ring", "Diamond Ring", "Clay Ring", "Iron Ring" };
std::string ringEffects[NUM_RINGS] = { "Ring of Swiftness", "Ring of Strength", "Ring of Weakness", "Ring of Hunger", "Ring of Regeneration" };
bool ringCursed[NUM_RINGS] = { false, false, true, true, false };
bool ringIdentified[NUM_RINGS] = { false, false, false, false, false };

void randomizePotionEffects() {
  // Shuffle the potion effects array
  for (int i = NUM_POTIONS - 1; i > 0; i--) {
    int j = rand() % (i + 1);  // Random index from 0 to i inclusive
    // Swap effects
    PotionEffect temp = potionEffects[i];
    potionEffects[i] = potionEffects[j];
    potionEffects[j] = temp;
  }

  // Assign shuffled effects to potions
  for (int i = 0; i < NUM_POTIONS; i++) {
    itemList[i].healthRecoverAmount = potionEffects[i].healthChange;
    itemList[i].AOEsize = potionEffects[i].AOEsize;
    itemList[i].AOEdamage = potionEffects[i].AOEdamage;
    itemList[i].SpeedMultiplier = potionEffects[i].SpeedMultiplier;
    itemList[i].itemResult = potionEffects[i].effectResult;
    itemList[i].effectType = potionEffects[i].effectType;
  }
}

GameItem getItem(GameItems item) {
  GameItem newItem = itemList[item];
  
  // If it's a scroll, assign a random effect and name
  if (item == Scroll) {
    int effectIndex = rand() % (NUM_SCROLLS - 0) + 0;
    newItem.scrollEffectIndex = effectIndex;
    newItem.name = scrollNames[effectIndex];
    newItem.description = "Read it to find out.";
    newItem.isScrollRevealed = false;
  }
  
  // Assign a random effect to rings
  if (item == Ring) {
    int effectIndex = rand() % (NUM_RINGS - 0) + 0;
    newItem.ringEffectIndex = effectIndex;
    newItem.isCursed = ringCursed[effectIndex];
    if (ringIdentified[effectIndex]) {
      newItem.isRingIdentified = true;
      newItem.name = ringEffects[effectIndex];
    } else {
      newItem.isRingIdentified = false;
      newItem.name = ringTypes[effectIndex];
    }
  }
  
  return newItem;
}

void updatePotionName(GameItem &potion) {
  for (PotionEffect effect : potionEffects) {
    if (potion.effectType == effect.effectType) {
      for (int i = 0; i < NUM_POTIONS; i++) {
        if (itemList[i].item == potion.item) {  
          itemList[i].name = effect.effectName;
          itemList[i].description = effect.effectDescription;
        }
      }

      // Update all instances of the potion in the inventory
      for (int i = 0; i < inventorySize; i++) {
        if (inventoryPages[0].items[i].item == potion.item) {
          inventoryPages[0].items[i].name = effect.effectName;
          inventoryPages[0].items[i].description = effect.effectDescription;
        }
      }

      break;
    }
  }
}

// Resets all potion names to their original names using the stored originalName field
void resetPotionNames() {
  for (int i = 0; i < NUM_POTIONS; i++)
  {
    itemList[i].name = itemList[i].originalName;
  }
}

GameItems getRandomPotion(int randInt, bool primaryColors) {
    if (primaryColors) {
        GameItems potions[] = { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion };
        return potions[randInt % 6];
    } else {
        // Dynamically collect all potions from itemList
        GameItems potions[NUM_POTIONS];
        int count = 0;
        for (size_t i = 0; i < sizeof(itemList)/sizeof(itemList[0]); i++) {
            if (itemList[i].category == PotionCategory && itemList[i].item != EmptyBottle && itemList[i].item != Null) {
                potions[count++] = itemList[i].item;
            }
        }
        return potions[randInt % count];
    }
}

void applyAOEEffect(float centerX, float centerY, int aoeRadius, int aoeDamage) {
  // Loop through all enemies
  for (int i = 0; i < maxEnemies; i++) {
    // Only consider enemies that are still alive
    if (enemies[i].hp > 0) {
      // Use the same rounding as your collision functions:
      int dx = round(centerX) - round(enemies[i].x);
      int dy = round(centerY) - round(enemies[i].y);
      // Compare squared distance to avoid computing square roots
      if (dx * dx + dy * dy <= aoeRadius * aoeRadius) {
        enemies[i].hp -= aoeDamage;
        if (enemies[i].hp <= 0) {
          kills += 1;
        }
      }
    }
  }
}

void renderItemResult() {
  display.clearDisplay();
  
  // Message text
  display.setCursor(15, 65);
  display.print(itemResultMessage);
  
  display.display();
}

bool areItemsEqual(GameItem item1, GameItem item2) {
 return (item1.item == item2.item) &&
        (item1.category == item2.category) &&
        (item1.healthRecoverAmount == item2.healthRecoverAmount) &&
        (item1.AOEsize == item2.AOEsize) &&
        (item1.AOEdamage == item2.AOEdamage) &&
        (item1.SpeedMultiplier == item2.SpeedMultiplier) &&
        (item1.name == item2.name) &&
        (item1.description == item2.description) &&
        (item1.originalName == item2.originalName) &&
        (item1.itemResult == item2.itemResult);
}

// Generalized combination function
GameItem combineItems(GameItem item1, GameItem item2) {
    // Try to find a matching combination
    for (int i = 0; i < NUM_ITEM_COMBINATIONS; i++) {
        const ItemCombination& combo = itemCombinations[i];
        // Check both possible orderings of the ingredients
        if ((item1.item == combo.ingredient1 && item2.item == combo.ingredient2) ||
            (item1.item == combo.ingredient2 && item2.item == combo.ingredient1)) {
            return getItem(combo.result);
        }
    }
    // If no combination found, return empty item
    return getItem(Null);
}

// Update CombineTwoItemsToGetItem to use combineItems for all types
GameItem CombineTwoItemsToGetItem(GameItem item1, GameItem item2) {
    // Generalized item combination
    GameItem result = combineItems(item1, item2);
    if (result.item != Null) {
        return result;
    }
    // Potion + Scroll or Scroll + Potion → Wet Scroll (special case)
    if ((item1.category == PotionCategory && item2.category == ScrollsCategory) ||
        (item2.category == PotionCategory && item1.category == ScrollsCategory)) {
        return getItem(WetScroll);
    }
    // No valid combination
    GameItem nullItem = getItem(Null);
    nullItem.name = "Null";
    return nullItem;
}

// Generate a random scroll name using a simple algorithm
std::string generateScrollName() {
  const char* consonants[] = {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "y", "z"};
  const char* vowels[] = {"a", "e", "i", "o", "u"};
  
  std::string name = "";
  int length = rand() % (8 - 4 + 1) + 4; // Random length between 4-7 characters
  
  for (int i = 0; i < length; i++) {
    if (i % 2 == 0) {
      // Even positions get consonants
      int cidx = rand() % (sizeof(consonants)/sizeof(consonants[0]));
      name += consonants[cidx];
    } else {
      // Odd positions get vowels
      int vidx = rand() % (sizeof(vowels)/sizeof(vowels[0]));
      name += vowels[vidx];
    }
  }
  // Capitalize first letter
  if (!name.empty()) name[0] = toupper(name[0]);
  return "Scroll: " + name;
}

void randomizeScrollEffects() {
  // Generate random names for scrolls
  for (int i = 0; i < NUM_SCROLLS; i++) {
    scrollNames[i] = generateScrollName();
  }
  
  // Shuffle the scroll effects array
  for (int i = NUM_SCROLLS - 1; i > 0; i--) {
    int j = rand() % (i + 1);  // Random index from 0 to i inclusive
    std::swap(scrollEffects[i], scrollEffects[j]);
  }
}

void updateScrollName(GameItem &scroll) {
  if (scroll.scrollEffectIndex >= 0 && scroll.scrollEffectIndex < NUM_SCROLLS) {
    ScrollEffect effect = scrollEffects[scroll.scrollEffectIndex];
    
    // Update the scroll's name and description
    scroll.name = effect.effectName;
    scroll.description = effect.effectDescription;
    scroll.itemResult = effect.effectResult;
    scroll.effectType = effect.effectType;
    scroll.isScrollRevealed = true;
    
    // Update the scrollNames array so future scrolls use the revealed name
    scrollNames[scroll.scrollEffectIndex] = effect.effectName;
    
    // Update all instances of this scroll in the inventory
    for (int p = 0; p < numInventoryPages; p++) {
      for (int i = 0; i < inventorySize; i++) {
        if (inventoryPages[p].items[i].item == Scroll && 
            inventoryPages[p].items[i].scrollEffectIndex == scroll.scrollEffectIndex) {
          inventoryPages[p].items[i].name = effect.effectName;
          inventoryPages[p].items[i].description = effect.effectDescription;
          inventoryPages[p].items[i].itemResult = effect.effectResult;
          inventoryPages[p].items[i].effectType = effect.effectType;
          inventoryPages[p].items[i].isScrollRevealed = true;
        }
      }
    }
  }
}

void randomizeRingEffects() {
    for (int i = NUM_RINGS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        std::string tempEffect = ringEffects[i];
        ringEffects[i] = ringEffects[j];
        ringEffects[j] = tempEffect;
        bool tempCursed = ringCursed[i];
        ringCursed[i] = ringCursed[j];
        ringCursed[j] = tempCursed;
    }
}

void updateRingName(GameItem &ring) {
    if (ring.ringEffectIndex >= 0 && ring.ringEffectIndex < NUM_RINGS) {
        ring.name = ringEffects[ring.ringEffectIndex];
        ring.description = "A mysterious ring. Its power is now revealed.";
        ring.isRingIdentified = true;
        ringIdentified[ring.ringEffectIndex] = true; // Mark globally as identified
        ring.isCursed = ringCursed[ring.ringEffectIndex];
    }
}