#include "Item.h"
#include "Inventory.h"
#include "HelperFunctions.h"
#include "Player.h"

extern int numInventoryPages;

GameItem itemList[] = {
  { RedPotion, PotionCategory, String("Red Potion"),  0,  0,  0, 0, 0, String("Drink it to find out."), String("Red Potion"), String("Nothing happens.")},
  { GreenPotion, PotionCategory, String("Green Potion"), 0,  0,  0, 0, 0, String("Drink it to find out."), String("Green Potion"), String("Nothing happens.")},
  { BluePotion,  PotionCategory, String("Blue Potion"),  0,  0,  0, 0, 0, String("Drink it to find out."), String("Blue Potion"), String("Nothing happens.")},
  { BlackPotion, PotionCategory, String("Black Potion"), 0,  0,  0, 0, 0, String("Drink it to find out."), String("Black Potion"), String("Nothing happens.")},
  { WhitePotion, PotionCategory, String("White Potion"), 0,  0,  0, 0, 0, String("Drink it to find out."), String("White Potion"), String("Nothing happens.")},
  { YellowPotion, PotionCategory, String("Yellow Potion"), 0,  0,  0, 0, 0, String("Drink it to find out."), String("Yellow Potion"), String("Nothing happens.")},
  { OrangePotion, PotionCategory, String("Orange Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Orange Potion"), String("Nothing happens.")},
  { PurplePotion, PotionCategory, String("Purple Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Purple Potion"), String("Nothing happens.")},
  { CyanPotion, PotionCategory, String("Cyan Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Cyan Potion"), String("Nothing happens.")},
  { MaroonPotion, PotionCategory, String("Maroon Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Maroon Potion"), String("Nothing happens.")},
  { DarkGreenPotion, PotionCategory, String("DarkGreen Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("DarkGreen Potion"), String("Nothing happens.")},
  { Mushroom, FoodCategory, String("Mushroom"), 0,  20,  0, 0, 0, String("It is edible."), String("Mushroom"), String("You become less hungry.")},
  { EmptyBottle, PotionCategory, String("Empty Bottle"), 0,  20,  0, 0, 0, String("It is an empty bottle."), String("Empty Bottle"), String("Nothing happens. It's an empty bottle."), false},
  { RiddleStone, EquipmentCategory, String("Riddle Stone"), 0,  0,  0, 0, 0, String("Looks like it could be used for many things..."), String("Riddle Stone"), String("Solve this riddle!"), true, DefaultEffect, 0, false, false, 2},
  { Scroll, PotionCategory, String("Scroll"), 0,  0,  0, 0, 0, String("Read it to find out."), String("Scroll"), String("Nothing happens.")},
  { LeatherArmor, EquipmentCategory, String("Leather Armor"), 0,  0,  0, 0, 0, String("Basic leather armor. Reduces damage by 2."), String("Leather Armor"), String("You equip the leather armor."), false, ArmorEffect, 2, false, false, 2},
  { IronArmor, EquipmentCategory, String("Iron Armor"), 0,  0,  0, 0, 0, String("Sturdy iron armor. Reduces damage by 3."), String("Iron Armor"), String("You equip the iron armor."), false, ArmorEffect, 3, false, false, 2},
  { MagicRobe, EquipmentCategory, String("Magic Robe"), 0,  0,  0, 0, 0, String("Enchanted robe. Reduces damage by 1 and increases magic resistance."), String("Magic Robe"), String("You equip the magic robe."), false, ArmorEffect, 1, false, false, 2},
  { Cloak, EquipmentCategory, String("Cloak"), 0,  0,  0, 0, 0, String("A simple cloth cloak. Provides no protection but keeps you warm."), String("Cloak"), String("You equip the cloak."), false, ArmorEffect, 0, false, false, 2}
};

String scrollNames[] = {
  "null",
  "null",
  "null"
};

// Possible effect pool
PotionEffect potionEffects[] = {
  { 20,  0,  0, 0, String("Healing Potion"), String("Healing. Heals 20 of your HP."), String("You feel better."), HealingEffect },
  { -20, 0,  0, 0, String("Diluted Poison"), String("Deducts 20 of your HP. Don't drink. Unless your guilty of something..."), String("You lose 20 HP."), PoisonEffect },
  { 0,   4, 40, 0, String("Explosion Potion"), String("Bomb. Deals 40 damage to enemies around you."), String("The enemies around you lose 40 HP."), ExplosionEffect },
  { 40,   4, -30, 0, String("Buffing Potion"), String("Heals 40 of your HP, but also heals 30 HP of enemies around you."), String("You feel better, but so do the enemies close to you."), BuffingEffect },
  { 70,  0,  0, 0, String("Mega Heal Potion"), String("Healing, but mega. Heals 70 of your HP."), String("You feel much better."), MegaHealEffect },
  { -50,  4,  -20, 0, String("Bad Potion"), String("It deducts 50 of your HP, and gives enemies around you 20 HP. Maybe don't drink this."), String("You lose 50 HP, and the enemies around you gain 20 HP."), BadEffect },
  { 0,  0,  0, 2, String("Speed Potion"), String("Drink this, and you'll go twice as fast."), String("Your are faster now, but only for a limited amount of time."), SpeedEffect },
  { 0,  0,  0, 0.4, String("Slowing Potion"), String("Drink this, and you'll go half as fast."), String("Your are slower now, but only for a limited amount of time."), SlowEffect },
  { 0,  0,  0, 0, String("Hunger Potion"), String("Makes you more hungry."), String("You are now more hungry."), HungerEffect },
  { 0,  0,  0, 0, String("See-all Potion"), String("Opens your eyes to the unseen."), String("You can now see that which was unseen for a limited time."), SeeAllEffect },
  { 0,  0,  0, 0, String("Confusion Potion"), String("You go in the opposite direction to the direction you want to go."), String("What is going on?"), ConfusionEffect }
};

ScrollEffect scrollEffects[] = {
  {String("Protect scroll"), String("Protects your armor from rusting and raises its damage reduction."), String("Your armor is covered by a shimmering gold shield!"), ScrollProtectionEffect},
  {String("Identify scroll"), String("Reveals the true name of an item and sees if it is cursed."), String("Select an item to identify."), ScrollIdentifyEffect},
  {String("Enchant scroll"), String("Makes an item better than it used to be."), String("Select an item to enchant."), ScrollEnchantEffect}
};

// Define all possible potion combinations
PotionCombination potionCombinations[] = {
    {BluePotion, YellowPotion, GreenPotion},
    {RedPotion, GreenPotion, YellowPotion},
    {RedPotion, YellowPotion, OrangePotion},
    {RedPotion, BluePotion, PurplePotion},
    {GreenPotion, BluePotion, CyanPotion},
    {RedPotion, BlackPotion, MaroonPotion},
    {GreenPotion, BlackPotion, DarkGreenPotion}
};

const int NUM_POTION_COMBINATIONS = sizeof(potionCombinations) / sizeof(potionCombinations[0]);

void randomizePotionEffects() {
  // Shuffle the potion effects array
  for (int i = NUM_POTIONS - 1; i > 0; i--) {
    int j = random(i + 1);  // Random index from 0 to i inclusive
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
    int effectIndex = random(0, NUM_SCROLLS);
    newItem.scrollEffectIndex = effectIndex;
    newItem.name = scrollNames[effectIndex];
    newItem.description = "Read it to find out.";
    newItem.isScrollRevealed = false;
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
    return potions[randInt % 6];  // Ensure it's within bounds
  } else {
    GameItems allPotions[] = { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion, 
                              OrangePotion, PurplePotion, CyanPotion, MaroonPotion, DarkGreenPotion };
    return allPotions[randInt % NUM_POTIONS];  // Use all potions
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
  u8g2_for_adafruit_gfx.setFont(u8g2_font_profont12_tr);
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
        (item1.name.equals(item2.name)) &&
        (item1.description.equals(item2.description)) &&
        (item1.originalName.equals(item2.originalName)) &&
        (item1.itemResult.equals(item2.itemResult));
}

GameItem combinePotions(GameItem item1, GameItem item2) {
    // Check if both items are potions
    if (item1.category != PotionCategory || item2.category != PotionCategory) {
        return {};  // Return empty item if not both potions
    }

    // Try to find a matching combination
    for (int i = 0; i < NUM_POTION_COMBINATIONS; i++) {
        const PotionCombination& combo = potionCombinations[i];
        
        // Check both possible orderings of the ingredients
        if ((item1.item == combo.ingredient1 && item2.item == combo.ingredient2) ||
            (item1.item == combo.ingredient2 && item2.item == combo.ingredient1)) {
            return getItem(combo.result);
        }
    }

    // If no combination found, return empty item
    return {};
}

// Keep the old function for backward compatibility, but make it use the new system
GameItem CombineTwoItemsToGetItem(GameItem item1, GameItem item2) {
    return combinePotions(item1, item2);
}

// Generate a random scroll name using a simple algorithm
String generateScrollName() {
  const char* consonants[] = {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "y", "z"};
  const char* vowels[] = {"a", "e", "i", "o", "u"};
  
  String name = "";
  int length = random(4, 8); // Random length between 4-7 characters
  
  for (int i = 0; i < length; i++) {
    if (i % 2 == 0) {
      // Even positions get consonants
      name += consonants[random(0, sizeof(consonants)/sizeof(consonants[0]))];
    } else {
      // Odd positions get vowels
      name += vowels[random(0, sizeof(vowels)/sizeof(vowels[0]))];
    }
  }
  
  // Capitalize first letter
  name.setCharAt(0, toupper(name.charAt(0)));
  
  return "Scroll: " + name;
}

void randomizeScrollEffects() {
  // Generate random names for scrolls
  for (int i = 0; i < NUM_SCROLLS; i++) {
    scrollNames[i] = generateScrollName();
  }
  
  // Shuffle the scroll effects array
  for (int i = NUM_SCROLLS - 1; i > 0; i--) {
    int j = random(i + 1);  // Random index from 0 to i inclusive
    // Swap effects
    ScrollEffect temp = scrollEffects[i];
    scrollEffects[i] = scrollEffects[j];
    scrollEffects[j] = temp;
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