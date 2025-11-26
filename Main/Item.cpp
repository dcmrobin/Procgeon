#include "Item.h"
#include "Inventory.h"
#include "HelperFunctions.h"
#include "Player.h"

extern int numInventoryPages;

GameItem itemList[] = {
  { RedPotion, PotionCategory, "Red Potion",  0,  0,  0, 0, 0, "Drink it to find out.", "Red Potion", "Nothing happens."},
  { GreenPotion, PotionCategory, "Green Potion", 0,  0,  0, 0, 0, "Drink it to find out.", "Green Potion", "Nothing happens."},
  { BluePotion,  PotionCategory, "Blue Potion",  0,  0,  0, 0, 0, "Drink it to find out.", "Blue Potion", "Nothing happens."},
  { BlackPotion, PotionCategory, "Black Potion", 0,  0,  0, 0, 0, "Drink it to find out.", "Black Potion", "Nothing happens."},
  { WhitePotion, PotionCategory, "White Potion", 0,  0,  0, 0, 0, "Drink it to find out.", "White Potion", "Nothing happens."},
  { YellowPotion, PotionCategory, "Yellow Potion", 0,  0,  0, 0, 0, "Drink it to find out.", "Yellow Potion", "Nothing happens."},
  { OrangePotion, PotionCategory, "Orange Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Orange Potion", "Nothing happens."},
  { PurplePotion, PotionCategory, "Purple Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Purple Potion", "Nothing happens."},
  { CyanPotion, PotionCategory, "Cyan Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Cyan Potion", "Nothing happens."},
  { MaroonPotion, PotionCategory, "Maroon Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Maroon Potion", "Nothing happens."},
  { DarkGreenPotion, PotionCategory, "DarkGreen Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "DarkGreen Potion", "Nothing happens."},
  { LimePotion, PotionCategory, "Lime Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Lime Potion", "Nothing happens."},
  { GreyPotion, PotionCategory, "Grey Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Grey Potion", "Nothing happens."},
  { OlivePotion, PotionCategory, "Olive Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Olive Potion", "Nothing happens."},
  { CreamPotion, PotionCategory, "Cream Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Cream Potion", "Nothing happens."},
  { NavyPotion, PotionCategory, "Navy Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Navy Potion", "Nothing happens."},
  { AzurePotion, PotionCategory, "Azure Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Azure Potion", "Nothing happens."},
  { MintPotion, PotionCategory, "Mint Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Mint Potion", "Nothing happens."},
  { SalmonPotion, PotionCategory, "Salmon Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Salmon Potion", "Nothing happens."},
  { BrownPotion, PotionCategory, "Brown Potion", 0,  0,  0,  0, 0, "Drink it to find out.", "Brown Potion", "Nothing happens."},
  { Mushroom, FoodCategory, "Mushroom", 0,  20,  0, 0, 0, "It is edible.", "Mushroom", "You become less hungry.", 1},
  { EmptyBottle, PotionCategory, "Empty Bottle", 0,  20,  0, 0, 0, "It is an empty bottle.", "Empty Bottle", "Nothing happens. It's an empty bottle.", 4, false},
  { RiddleStone, EquipmentCategory, "Riddle Stone", 0,  0,  0, 0, 0, "Looks like it could be used for many things...", "Riddle Stone", "Solve this riddle!", 5, true, DefaultEffect, 0, false, false, 2, false},
  { Scroll, ScrollsCategory, "Scroll", 0,  0,  0, 0, 0, "Read it to find out.", "Scroll", "Nothing happens.", 4},
  { WetScroll, ScrollsCategory, "Wet Scroll", 0,  0,  0, 0, 0, "A scroll that is too wet to read.", "Wet Scroll", "The scroll is too wet to read. Nothing happens.", 3, false},
  { Ring, EquipmentCategory, "Ring", 0,  0,  0, 0, 0, "Put it on to find out.", "Ring", "You equip the ring.", 3, false},
  { LeatherArmor, EquipmentCategory, "Leather Armor", 0,  0,  0, 0, 0, "Basic leather armor. Reduces damage taken.", "Leather Armor", "You equip the leather armor.", 3, false, ArmorEffect, 2, false, false, 2, false},
  { IronArmor, EquipmentCategory, "Iron Armor", 0,  0,  0, 0, 0, "Sturdy iron armor. Reduces damage taken.", "Iron Armor", "You equip the iron armor.", 4, false, ArmorEffect, 4, false, false, 2, true},
  { MagicRobe, EquipmentCategory, "Magic Robe", 0,  0,  0, 0, 0, "Enchanted robe. Reduces damage taken and increases magic resistance.", "Magic Robe", "You equip the magic robe.", 4, false, ArmorEffect, 1, false, false, 2, false},
  { Cloak, EquipmentCategory, "Cloak", 0,  0,  0, 0, 0, "A simple cloth cloak. Provides no protection but keeps you warm.", "Cloak", "You equip the cloak.", 4, false, ArmorEffect, 0, false, false, 2, false},
  { Trenchcoat, EquipmentCategory, "Trenchcoat", 0,  0,  0, 0, 0, "A trenchcoat and fedora. Provides no protection but looks cool.", "Trenchcoat", "You equip the trenchcoat.", 3, false, ArmorEffect, 0, false, false, 2, false},
  { DenimJacket, EquipmentCategory, "Denim Jacket", 0,  0,  0, 0, 0, "A simple denim jacket. Provides no protection but looks cool.", "Denim Jacket", "You equip the denim jacket.", 3, false, ArmorEffect, 0, false, false, 2, false},
  { RingMailArmor, EquipmentCategory, "Ring Mail Armor", 0,  0,  0, 0, 0, "Ring mail armor. Reduces damage taken.", "Ring Mail Armor", "You equip the ring mail armor.", 4, false, ArmorEffect, 3, false, false, 2, true},
  { ChaosArmor, EquipmentCategory, "Chaos Armor", 0,  0,  0, 0, 0, "This physical state of this armor is not stable.", "Chaos Armor", "The chaos armor clings to you.", 5, false, ArmorEffect, 0, false, true, 2, true},
  { Null, PotionCategory, "Null", 0, 0, 0, 0, 0, "", "Null", "", 5, false }
};

char scrollNames[NUM_SCROLLS][20] = {
    "Protect scroll",
    "Identify scroll",
    "Enchant scroll",
    "Uncurse scroll",
    "Empty scroll",
    "Mapping scroll",
    "Aggravate scroll"
};

// Possible effect pool
PotionEffect potionEffects[] = {// do not change any of the effectresult strings, as they are used for comparing effects
  { 20,  0,  0, 0, "Healing Potion", "Healing. Heals 20 of your HP.", "You feel better.", HealingEffect },
  { -20, 0,  0, 0, "Diluted Poison", "Deducts 20 of your HP. Don't drink.", "You feel a bit sick.", PoisonEffect },
  { 0,   4, 40, 0, "Explosion Potion", "Bomb. Deals 40 damage to enemies around you.", "The enemies around you lose 40 HP.", ExplosionEffect },
  { 40,   4, -30, 0, "Buffing Potion", "Heals 40 of your HP, but also heals 30 HP of enemies around you.", "You feel better, but so do the enemies close to you.", BuffingEffect },
  { 70,  0,  0, 0, "Mega Heal Potion", "Healing, but mega. Heals 70 of your HP.", "You feel much better.", MegaHealEffect },
  { -50,  4,  -20, 0, "Bad Potion", "It deducts 50 of your HP, and gives enemies around you 20 HP. Maybe don't drink this.", "You lose 50 HP, and the enemies around you gain 20 HP.", BadEffect },
  { 0,  0,  0, 1, "Speed Potion", "Drink this, and you'll go twice as fast.", "Your are faster now, but only for a limited amount of time.", SpeedEffect },
  { 0,  0,  0, -0.4, "Slowing Potion", "Drink this, and you'll go half as fast.", "Your are slower now, but only for a limited amount of time.", SlowEffect },
  { 0,  0,  0, 0, "Hunger Potion", "Makes you more hungry.", "You are now more hungry.", HungerEffect },
  { 0,  0,  0, 0, "See-all Potion", "Opens your eyes to the unseen.", "You can now see that which was unseen for a limited time.", SeeAllEffect },
  { 0,  0,  0, 0, "Confusion Potion", "You go in the opposite direction to the direction you want to go.", "What is going on?", ConfusionEffect },
  { 0,  0,  0, 0, "Ridicule Potion", "Drinking this makes you feel stupid.", "You feel stupid.", RidiculeEffect },
  { 0,  0,  0, 0, "Bland Potion", "Colored liquid that does nothing.", "Nothing happens.", NoEffect },
  { 0,  0,  0, 0, "Glamour Potion", "Drinking this makes you feel awesome.", "You feel fabulous!", GlamourEffect},
  { 0,  0,  0, 0, "Chaos Potion", "The effect of this potion is random.", "A lot happens.", ChaosEffect},
  { 0,  0,  0, 0, "Blindness Potion", "Makes you blind for a time. Do not drink this potion.", "A cloak of darkness falls around you.", BlindnessEffect},
  { 0,  0,  0, 0, "Strength Potion", "Makes you do more damage.", "You feel stronger.", StrengthEffect},
  { 0,  0,  0, 0, "Restore Potion", "Cures all your ailments.", "You feel restored!", RestoreEffect},
  { 0,  0,  0, 0, "Paralysis Potion", "Paralyzes you for a time.", "You can't move!", ParalysisEffect},
  { -40, 0, 0, 0, "Poison", "Deducts 40 of your HP.", "You feel very sick.", VeryPoisonEffect },
};

ScrollEffect scrollEffects[NUM_SCROLLS] = {
    {"Protect scroll", "Protects your armor from rusting and raises its damage reduction.", "Your armor is covered by a shimmering gold shield!", ScrollProtectionEffect},
    {"Identify scroll", "Reveals the true name of an item and sees if it is cursed.", "Select an item to identify.", ScrollIdentifyEffect},
    {"Enchant scroll", "Makes an item better than it used to be.", "Select an item to enchant.", ScrollEnchantEffect},
    {"Uncurse scroll", "Removes curses from all equipped items.", "You feel as if someone is watching over you.", ScrollUncurseEffect},
    {"Empty scroll", "It looks like it's just paper.", "You look at the empty scroll.", ScrollEmptyEffect},
    {"Mapping scroll", "It has a map on it.", "You study the map on the scroll.", ScrollMapEffect},
    {"Amnesia scroll", "You feel like forgetting things when around this scroll.", "You feel as if you've forgotten something...", ScrollAmnesiaEffect},
    {"Aggravate scroll" , "You feel bothered around this scroll.", "You hear a high-pitched humming sound.", ScrollAggravateEffect}
};

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
};

const int NUM_ITEM_COMBINATIONS = sizeof(itemCombinations) / sizeof(itemCombinations[0]);

char ringTypes[NUM_RINGS][20] = { "Wooden Ring", "Emerald Ring", "Diamond Ring", "Clay Ring", "Gold Ring", "Ruby ring", "Washer" };
char ringEffects[NUM_RINGS][100] = { "Ring of Swiftness", "Ring of Strength", "Ring of Weakness", "Ring of Hunger", "Ring of Regeneration", "Ring", "Ring of Sickness" };
bool ringCursed[NUM_RINGS] = { false, false, true, true, false, false };

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
    snprintf(itemList[i].itemResult, sizeof(itemList[i].itemResult), "%s", potionEffects[i].effectResult);
    itemList[i].effectType = potionEffects[i].effectType;
  }
}

GameItem getItem(GameItems item) {
  GameItem newItem = itemList[item];
  
  // If it's a scroll, assign a random effect and name
  if (item == Scroll) {
    int effectIndex = random(0, NUM_SCROLLS);
    newItem.scrollEffectIndex = effectIndex;
    if (strcmp(newItem.name, "Scroll") == 0) {
      snprintf(newItem.description, sizeof(newItem.description), "%s", "Read it to find out.");
      newItem.isScrollRevealed = false;
    }
    snprintf(newItem.name, sizeof(newItem.name), "%s", scrollNames[effectIndex]);
    snprintf(newItem.originalName, sizeof(newItem.originalName), "%s", scrollNames[effectIndex]);
  }
  
  // Create a ring item: assign it a visible type now, but do NOT assign its effect until worn or identified
  if (item == Ring) {
    int typeIndex = random(0, NUM_RINGS);
    newItem.ringTypeIndex = typeIndex;
    newItem.ringEffectIndex = -1; // effect not yet assigned
    newItem.isRingIdentified = false;
    newItem.isCursed = false; // curse status depends on the effect once assigned
    snprintf(newItem.name, sizeof(newItem.name), "%s", ringTypes[typeIndex]);
    snprintf(newItem.originalName, sizeof(newItem.originalName), "%s", ringTypes[typeIndex]);
  }
  
  return newItem;
}

void updatePotionName(GameItem &potion) {
  for (PotionEffect effect : potionEffects) {
    if (potion.effectType == effect.effectType) {
      for (int i = 0; i < NUM_POTIONS; i++) {
        if (itemList[i].item == potion.item) {  
          snprintf(itemList[i].name, sizeof(itemList[i].name), "%s", effect.effectName);
          snprintf(itemList[i].description, sizeof(itemList[i].description), "%s", effect.effectDescription);
        }
      }

      // Update all instances of the potion in the inventory
      for (int i = 0; i < inventorySize; i++) {
        if (inventoryPages[0].items[i].item == potion.item) {
          snprintf(inventoryPages[0].items[i].name, sizeof(inventoryPages[0].items[i].name), "%s", effect.effectName);
          snprintf(inventoryPages[0].items[i].description, sizeof(inventoryPages[0].items[i].description), "%s", effect.effectDescription);
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
    snprintf(itemList[i].name, sizeof(itemList[i].name), "%s", itemList[i].originalName);
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
        (strcmp(item1.name, item2.name) == 0) &&
        (strcmp(item1.description, item2.description) == 0) &&
        (strcmp(item1.originalName, item2.originalName) == 0) &&
        (strcmp(item1.itemResult, item2.itemResult) == 0);
}

// Generalized combination function
GameItem combineItems(GameItem item1, GameItem item2) {
    // Define primary potion items
    GameItems primaryPotions[] = { RedPotion, GreenPotion, BluePotion, YellowPotion, WhitePotion, BlackPotion };
    auto isPrimaryPotion = [&](GameItems item) {
        for (int i = 0; i < 6; i++) {
            if (item == primaryPotions[i]) return true;
        }
        return false;
    };
    // If both are potions
    if (item1.category == PotionCategory && item2.category == PotionCategory) {
        bool item1Primary = isPrimaryPotion(item1.item);
        bool item2Primary = isPrimaryPotion(item2.item);
        bool item1Empty = (item1.item == EmptyBottle);
        bool item2Empty = (item2.item == EmptyBottle);
        // If one of them is an empty bottle, return potion
        if (item1Empty && !item2Empty) {
            return getItem(item2.item);
        } else if (!item1Empty && item2Empty) {
            return getItem(item1.item);
        } else if (item1Empty && item2Empty) {
            return getItem(EmptyBottle);
        }
        // Prevent combining an item with itself
        if (item1.item == item2.item) {
            return getItem(Null);
        }
        // If either is non-primary, return BrownPotion
        if (!item1Primary || !item2Primary) {
            return getItem(BrownPotion);
        }
    } else if ((item1.category == PotionCategory && item2.category == EquipmentCategory) || (item1.category == EquipmentCategory && item2.category == PotionCategory)) {
      if (item1.item == EmptyBottle || item2.item == EmptyBottle) {
        GameItem item = item1.item == EmptyBottle ? item2 : item1;
        snprintf(item.itemResult, sizeof(item.itemResult), "The %s cannot fit inside the bottle.", item.name);
        return item;
      } else if (strcmp(item1.name, "Washer") == 0 || strcmp(item2.name, "Washer") == 0 ) {
        GameItem item = item1.item == Ring ? item1 : item2;
        snprintf(item.name, sizeof(item.name), "%s", "Wet Washer");
        snprintf(item.originalName, sizeof(item.originalName), "%s", "Wet Washer");
        snprintf(item.itemResult, sizeof(item.itemResult), "%s", "The washer is wet now, making it easy to remove.");
        snprintf(item.description, sizeof(item.description), "%s", "A wet washer. It being wet prevents it from getting stuck on your finger.");
        return item;
      } else if (item1.category == EquipmentCategory ? item1.canRust : item2.canRust) {
        GameItem item = item1.category == EquipmentCategory ? item1 : item2;
        bool cursed = random(0, 10) < 3 ? true : false;
        GameItem rustedItem = item; // Copy the entire struct first
        rustedItem.AOEdamage = 123; // This is to make sure "rusty"s don't get added more than necessary to the name
        rustedItem.armorValue = item.armorValue - (item.armorValue < 0 ? 0 : 1);
        rustedItem.isCursed = cursed;
        rustedItem.canRust = true;
        snprintf(rustedItem.description, sizeof(rustedItem.description), "A Rusty %s. It looks degraded%s.", item.name, (cursed ? ", and you feel a sense of unease around it." : ""));
        snprintf(rustedItem.name, sizeof(rustedItem.name), "Rusty %s", item.name);
        snprintf(rustedItem.itemResult, sizeof(rustedItem.itemResult), "You pour the %s over the %s. It rusts%s.", (item1.category == PotionCategory ? item1.name : item2.name), item.name, (cursed ? ", becomes less durable, and shimmers slightly red for a moment" : " and becomes less durable"));
        if (item.isEquipped) {
          equippedArmorValue = item.armorValue;
        }
        return rustedItem;
      } else if (item1.category == EquipmentCategory ? !item1.canRust : !item2.canRust) {
        GameItem item = item1.category == EquipmentCategory ? item1 : item2;
        GameItem unrustedItem = item; // Copy the entire struct
        snprintf(unrustedItem.itemResult, sizeof(unrustedItem.itemResult), "You pour the %s over the %s, but nothing happens.", (item1.category == PotionCategory ? item1.name : item2.name), item.name);
        return unrustedItem;
      }
    } else if ((item1.category == PotionCategory && item2.category == FoodCategory) || (item2.category == PotionCategory && item1.category == FoodCategory)) {
      if (item1.item == EmptyBottle || item2.item == EmptyBottle) {
        GameItem item = item1.item == EmptyBottle ? item2 : item1;
        snprintf(item.itemResult, sizeof(item.itemResult), "The %s cannot fit inside the bottle.", item.name);
        return item;
      }
      GameItem potion = item1.category == PotionCategory ? item1 : item2;
      GameItem food = item1.category == FoodCategory ? item1 : item2;
      food.healthRecoverAmount = potion.healthRecoverAmount;
      food.hungerRecoverAmount = potion.hungerRecoverAmount;
      food.AOEsize = potion.AOEsize;
      food.AOEdamage = potion.AOEdamage;
      food.SpeedMultiplier = potion.SpeedMultiplier;
      snprintf(food.name, sizeof(food.name), "Odd %s", (item1.category == FoodCategory ? item1.name : item2.name));
      return food;
    }
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
    snprintf(nullItem.name, sizeof(nullItem.name), "%s", "Null");
    return nullItem;
}

// Generate a random scroll name using a simple algorithm
void generateScrollName(char *name, size_t nameSize) {
  const char* consonants[] = {"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r", "s", "t", "v", "w", "x", "y", "z"};
  const char* vowels[] = {"a", "e", "i", "o", "u"};
  
  char baseName[20] = "";
  int length = random(4, 8); // Random length between 4-7 characters
  
  for (int i = 0; i < length && i < (int)(sizeof(baseName) - 1); i++) {
    if (i % 2 == 0) {
      // Even positions get consonants
      strcat(baseName, consonants[random(0, sizeof(consonants)/sizeof(consonants[0]))]);
    } else {
      // Odd positions get vowels
      strcat(baseName, vowels[random(0, sizeof(vowels)/sizeof(vowels[0]))]);
    }
  }
  
  // Capitalize first letter
  if (strlen(baseName) > 0) {
    baseName[0] = toupper(baseName[0]);
  }
  
  snprintf(name, nameSize, "Scroll: %s", baseName);
}

void randomizeScrollEffects() {
  // Generate random names for scrolls
  for (int i = 0; i < NUM_SCROLLS; i++) {
    generateScrollName(scrollNames[i], sizeof(scrollNames[i]));
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
    snprintf(scroll.name, sizeof(scroll.name), "%s", effect.effectName);
    snprintf(scroll.description, sizeof(scroll.description), "%s", effect.effectDescription);
    snprintf(scroll.itemResult, sizeof(scroll.itemResult), "%s", effect.effectResult);
    scroll.effectType = effect.effectType;
    scroll.isScrollRevealed = true;
    
    // Update the scrollNames array so future scrolls use the revealed name
    snprintf(scrollNames[scroll.scrollEffectIndex], sizeof(scrollNames[scroll.scrollEffectIndex]), "%s", effect.effectName);
    
    // Update all instances of this scroll in the inventory
    for (int p = 0; p < numInventoryPages; p++) {
      for (int i = 0; i < inventorySize; i++) {
        if (inventoryPages[p].items[i].item == Scroll && inventoryPages[p].items[i].scrollEffectIndex == scroll.scrollEffectIndex) {
          snprintf(inventoryPages[p].items[i].name, sizeof(inventoryPages[p].items[i].name), "%s", effect.effectName);
          snprintf(inventoryPages[p].items[i].description, sizeof(inventoryPages[p].items[i].description), "%s", effect.effectDescription);
          snprintf(inventoryPages[p].items[i].itemResult, sizeof(inventoryPages[p].items[i].itemResult), "%s", effect.effectResult);
          inventoryPages[p].items[i].effectType = effect.effectType;
          inventoryPages[p].items[i].isScrollRevealed = true;
        }
      }
    }
  }
}

void randomizeRingEffects() {
    for (int i = NUM_RINGS - 1; i > 0; i--) {
        int j = random(i + 1);
        std::swap(ringEffects[i], ringEffects[j]);
        std::swap(ringCursed[i], ringCursed[j]);
    }
}

void updateRingName(GameItem &ring) {
  // If this ring doesn't yet have an assigned effect, assign one now for this instance
  if (ring.ringEffectIndex < 0 || ring.ringEffectIndex >= NUM_RINGS) {
    ring.ringEffectIndex = random(0, NUM_RINGS);
    ring.isCursed = ringCursed[ring.ringEffectIndex];
  }
  if (ring.ringEffectIndex >= 0 && ring.ringEffectIndex < NUM_RINGS) {
    snprintf(ring.name, sizeof(ring.name), "%s", ringEffects[ring.ringEffectIndex]);
    snprintf(ring.description, sizeof(ring.description), "%s", "A mysterious ring. Its power is now revealed.");
    ring.isRingIdentified = true;
    // NOTE: identification is per-instance now — do NOT mark any global identification flags
  }
}

// Rarity-based item selection functions
GameItems getRandomItemByRarity(ItemCategory category, int maxRarity) {
    // Create a weighted list based on rarity (lower rarity = higher weight)
    GameItems candidates[50]; // Max possible items
    int weights[50];
    int candidateCount = 0;
    
    // Find all items in the category within the rarity limit
    for (size_t i = 0; i < sizeof(itemList) / sizeof(itemList[0]); i++) {
        GameItem item = itemList[i];
        if (item.category == category && item.rarity <= maxRarity && item.item != Null) {
            candidates[candidateCount] = item.item;
            // Higher weight for lower rarity (rarity 1 = weight 5, rarity 2 = weight 4, etc.)
            weights[candidateCount] = max(1, 6 - item.rarity);
            candidateCount++;
        }
    }
    
    if (candidateCount == 0) {
        return Null; // No valid items found
    }
    
    // Calculate total weight
    int totalWeight = 0;
    for (int i = 0; i < candidateCount; i++) {
        totalWeight += weights[i];
    }
    
    // Select random item based on weight
    int randomValue = random(0, totalWeight);
    int currentWeight = 0;
    
    for (int i = 0; i < candidateCount; i++) {
        currentWeight += weights[i];
        if (randomValue < currentWeight) {
            return candidates[i];
        }
    }
    
    // Fallback to first candidate
    return candidates[0];
}

GameItems getRandomItemByRarityAnyCategory(int maxRarity) {
    // Create a weighted list based on rarity (lower rarity = higher weight)
    GameItems candidates[50]; // Max possible items
    int weights[50];
    int candidateCount = 0;
    
    // Find all items within the rarity limit
    for (size_t i = 0; i < sizeof(itemList) / sizeof(itemList[0]); i++) {
        GameItem item = itemList[i];
        if (item.rarity <= maxRarity && item.item != Null && item.item != WetScroll) {
            candidates[candidateCount] = item.item;
            // Higher weight for lower rarity (rarity 1 = weight 5, rarity 2 = weight 4, etc.)
            weights[candidateCount] = max(1, 6 - item.rarity);
            candidateCount++;
        }
    }
    
    if (candidateCount == 0) {
        return Null; // No valid items found
    }
    
    // Calculate total weight
    int totalWeight = 0;
    for (int i = 0; i < candidateCount; i++) {
        totalWeight += weights[i];
    }
    
    // Select random item based on weight
    int randomValue = random(0, totalWeight);
    int currentWeight = 0;
    
    for (int i = 0; i < candidateCount; i++) {
        currentWeight += weights[i];
        if (randomValue < currentWeight) {
            return candidates[i];
        }
    }
    
    // Fallback to first candidate
    return candidates[0];
}

TileTypes getRandomLootTile(int maxRarity) {
    // First, select a category with equal probability
    ItemCategory categories[] = {PotionCategory, FoodCategory, ScrollsCategory, EquipmentCategory};
    int randomCategoryIndex = random(0, 4); // Random between 0-3
    
    // Get a random item from the selected category
    GameItems randomItem = getRandomItemByRarity(categories[randomCategoryIndex], maxRarity);
    
    // If no item found in this category, try any category as fallback
    if (randomItem == Null) {
        randomItem = getRandomItemByRarityAnyCategory(maxRarity);
    }
    
    // Convert GameItems to TileTypes
    switch (getItem(randomItem).category) {
        case PotionCategory:
            return Potion;
        case ScrollsCategory:
            return ScrollTile;
        case EquipmentCategory:
            if (randomItem == Ring) {
                return RingTile;
            } else if (randomItem == RiddleStone) {
                return RiddleStoneTile;
            } else {
                return ArmorTile; // For armor items
            }
        case FoodCategory:
            return MushroomTile;
        default:
            return Potion; // Default fallback
    }
}