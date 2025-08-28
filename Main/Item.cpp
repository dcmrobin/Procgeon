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
  { LimePotion, PotionCategory, String("Lime Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Lime Potion"), String("Nothing happens.")},
  { GreyPotion, PotionCategory, String("Grey Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Grey Potion"), String("Nothing happens.")},
  { OlivePotion, PotionCategory, String("Olive Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Olive Potion"), String("Nothing happens.")},
  { CreamPotion, PotionCategory, String("Cream Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Cream Potion"), String("Nothing happens.")},
  { NavyPotion, PotionCategory, String("Navy Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Navy Potion"), String("Nothing happens.")},
  { AzurePotion, PotionCategory, String("Azure Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Azure Potion"), String("Nothing happens.")},
  { MintPotion, PotionCategory, String("Mint Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Mint Potion"), String("Nothing happens.")},
  { SalmonPotion, PotionCategory, String("Salmon Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Salmon Potion"), String("Nothing happens.")},
  { BrownPotion, PotionCategory, String("Brown Potion"), 0,  0,  0,  0, 0, String("Drink it to find out."), String("Brown Potion"), String("Nothing happens.")},
  { Mushroom, FoodCategory, String("Mushroom"), 0,  20,  0, 0, 0, String("It is edible."), String("Mushroom"), String("You become less hungry."), 1},
  { EmptyBottle, PotionCategory, String("Empty Bottle"), 0,  20,  0, 0, 0, String("It is an empty bottle."), String("Empty Bottle"), String("Nothing happens. It's an empty bottle."), 4, false},
  { RiddleStone, EquipmentCategory, String("Riddle Stone"), 0,  0,  0, 0, 0, String("Looks like it could be used for many things..."), String("Riddle Stone"), String("Solve this riddle!"), 5, true, DefaultEffect, 0, false, false, 2, false},
  { Scroll, ScrollsCategory, String("Scroll"), 0,  0,  0, 0, 0, String("Read it to find out."), String("Scroll"), String("Nothing happens."), 4},
  { WetScroll, ScrollsCategory, String("Wet Scroll"), 0,  0,  0, 0, 0, String("A scroll that is too wet to read."), String("Wet Scroll"), String("The scroll is too wet to read. Nothing happens."), 3, false},
  { Ring, EquipmentCategory, String("Ring"), 0,  0,  0, 0, 0, String("Put it on to find out."), String("Ring"), String("You equip the ring."), 3, false},
  { LeatherArmor, EquipmentCategory, String("Leather Armor"), 0,  0,  0, 0, 0, String("Basic leather armor. Reduces damage by 2."), String("Leather Armor"), String("You equip the leather armor."), 3, false, ArmorEffect, 2, false, false, 2, false},
  { IronArmor, EquipmentCategory, String("Iron Armor"), 0,  0,  0, 0, 0, String("Sturdy iron armor. Reduces damage by 3."), String("Iron Armor"), String("You equip the iron armor."), 4, false, ArmorEffect, 3, false, false, 2, true},
  { MagicRobe, EquipmentCategory, String("Magic Robe"), 0,  0,  0, 0, 0, String("Enchanted robe. Reduces damage by 1 and increases magic resistance."), String("Magic Robe"), String("You equip the magic robe."), 4, false, ArmorEffect, 1, false, false, 2, false},
  { Cloak, EquipmentCategory, String("Cloak"), 0,  0,  0, 0, 0, String("A simple cloth cloak. Provides no protection but keeps you warm."), String("Cloak"), String("You equip the cloak."), 4, false, ArmorEffect, 0, false, false, 2, false},
  { Null, PotionCategory, String("Null"), 0, 0, 0, 0, 0, String(""), String("Null"), String(""), 5, false }
};

String scrollNames[NUM_SCROLLS] = {
    "Protect scroll",
    "Identify scroll",
    "Enchant scroll",
    "Uncurse scroll"
};

// Possible effect pool
PotionEffect potionEffects[] = {// do not change any of the effectresult strings, as they are used for comparing effects
  { 20,  0,  0, 0, String("Healing Potion"), String("Healing. Heals 20 of your HP."), String("You feel better."), HealingEffect },
  { -20, 0,  0, 0, String("Diluted Poison"), String("Deducts 20 of your HP. Don't drink."), String("You feel a bit sick."), PoisonEffect },
  { 0,   4, 40, 0, String("Explosion Potion"), String("Bomb. Deals 40 damage to enemies around you."), String("The enemies around you lose 40 HP."), ExplosionEffect },
  { 40,   4, -30, 0, String("Buffing Potion"), String("Heals 40 of your HP, but also heals 30 HP of enemies around you."), String("You feel better, but so do the enemies close to you."), BuffingEffect },
  { 70,  0,  0, 0, String("Mega Heal Potion"), String("Healing, but mega. Heals 70 of your HP."), String("You feel much better."), MegaHealEffect },
  { -50,  4,  -20, 0, String("Bad Potion"), String("It deducts 50 of your HP, and gives enemies around you 20 HP. Maybe don't drink this."), String("You lose 50 HP, and the enemies around you gain 20 HP."), BadEffect },
  { 0,  0,  0, 1, String("Speed Potion"), String("Drink this, and you'll go twice as fast."), String("Your are faster now, but only for a limited amount of time."), SpeedEffect },
  { 0,  0,  0, -0.4, String("Slowing Potion"), String("Drink this, and you'll go half as fast."), String("Your are slower now, but only for a limited amount of time."), SlowEffect },
  { 0,  0,  0, 0, String("Hunger Potion"), String("Makes you more hungry."), String("You are now more hungry."), HungerEffect },
  { 0,  0,  0, 0, String("See-all Potion"), String("Opens your eyes to the unseen."), String("You can now see that which was unseen for a limited time."), SeeAllEffect },
  { 0,  0,  0, 0, String("Confusion Potion"), String("You go in the opposite direction to the direction you want to go."), String("What is going on?"), ConfusionEffect },
  { 0,  0,  0, 0, String("Ridicule Potion"), String("Drinking this makes you feel stupid."), String("You feel stupid."), RidiculeEffect },
  { 0,  0,  0, 0, String("Bland Potion"), String("Colored liquid that does nothing."), String("Nothing happens."), NoEffect },
  { 0,  0,  0, 0, String("Glamour Potion"), String("Drinking this makes you feel awesome."), String("You feel fabulous!"), GlamourEffect},
  { 0,  0,  0, 0, String("Chaos Potion"), String("The effect of this potion is random."), String("A lot happens."), ChaosEffect},
  { 0,  0,  0, 0, String("Blindness Potion"), String("Makes you blind for a time. Do not drink this potion."), String("A cloak of darkness falls around you."), BlindnessEffect},
  { 0,  0,  0, 0, String("Strength Potion"), String("Makes you do more damage."), String("You feel stronger."), StrengthEffect},
  { 0,  0,  0, 0, String("Restore Potion"), String("Cures all your ailments."), String("You feel restored!"), RestoreEffect},
  { 0,  0,  0, 0, String("Paralysis Potion"), String("Paralyzes you for a time."), String("You can't move!"), ParalysisEffect},
  { -40, 0, 0, 0, String("Poison"), String("Deducts 40 of your HP."), String("You feel very sick."), VeryPoisonEffect },
};

ScrollEffect scrollEffects[NUM_SCROLLS] = {
    {String("Protect scroll"), String("Protects your armor from rusting and raises its damage reduction."), String("Your armor is covered by a shimmering gold shield!"), ScrollProtectionEffect},
    {String("Identify scroll"), String("Reveals the true name of an item and sees if it is cursed."), String("Select an item to identify."), ScrollIdentifyEffect},
    {String("Enchant scroll"), String("Makes an item better than it used to be."), String("Select an item to enchant."), ScrollEnchantEffect},
    {String("Uncurse scroll"), String("Removes curses from all equipped items."), String("You feel as if someone is watching over you."), ScrollUncurseEffect}
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

String ringTypes[NUM_RINGS] = { "Wooden Ring", "Emerald Ring", "Diamond Ring", "Clay Ring", "Gold Ring" };
String ringEffects[NUM_RINGS] = { "Ring of Swiftness", "Ring of Strength", "Ring of Weakness", "Ring of Hunger", "Ring of Regeneration" };
bool ringCursed[NUM_RINGS] = { false, false, true, true, false };
bool ringIdentified[NUM_RINGS] = { false, false, false, false, false };

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
  
  // Assign a random effect to rings
  if (item == Ring) {
    int effectIndex = random(0, NUM_RINGS);
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
        // If either is non-primary, return BrownPotion
        if (!item1Primary || !item2Primary) {
            return getItem(BrownPotion);
        }
    } else if ((item1.category == PotionCategory && item2.category == EquipmentCategory) || (item1.category == EquipmentCategory && item2.category == PotionCategory)) {
      if (item1.item == EmptyBottle || item2.item == EmptyBottle) {
        GameItem item = item1.item == EmptyBottle ? item2 : item1;
        item.itemResult = "The " + item.name + " cannot fit inside the bottle.";
        return item;
      } else if (item1.category == EquipmentCategory ? item1.canRust : item2.canRust) {
        GameItem item = item1.category == EquipmentCategory ? item1 : item2;
        bool cursed = random(0, 10) < 3 ? true : false;
        GameItem rustedItem = {
          item.item,
          item.category,
          item.AOEdamage != 123 ? "Rusty " + item.name : item.name,
          item.healthRecoverAmount,
          item.hungerRecoverAmount,
          item.AOEsize,
          123,// This is to make sure "rusty"s don't get added more than necessary to the name
          item.SpeedMultiplier,
          "A " + item.name + ". It looks degraded" + (cursed ? ", and you feel a sense of unease around it." : "."),
          item.originalName,
          "You pour the " + (item1.category == PotionCategory ? item1.name : item2.name) + " over the " + item.name + ". It rusts" + (cursed ? ", becomes less durable, and shimmers slightly red for a moment." : " and becomes less durable."),
          item.rarity,
          item.oneTimeUse,
          item.effectType,
          item.armorValue - (item.armorValue < 0 ? 0 : 1),
          item.isEquipped,
          cursed,
          item.curseChance,
          true
        };
        if (item.isEquipped) {
          equippedArmorValue = item.armorValue;
        }
        return rustedItem;
      } else if (item1.category == EquipmentCategory ? !item1.canRust : !item2.canRust) {
        GameItem item = item1.category == EquipmentCategory ? item1 : item2;
        GameItem unrustedItem = {
          item.item,
          item.category,
          item.name,
          item.healthRecoverAmount,
          item.hungerRecoverAmount,
          item.AOEsize,
          item.AOEdamage,
          item.SpeedMultiplier,
          item.name,
          item.originalName,
          "You pour the " + (item1.category == PotionCategory ? item1.name : item2.name) + " over the " + item.name + ", but nothing happens."
        };
        return unrustedItem;
      }
    } else if ((item1.category == PotionCategory && item2.category == FoodCategory) || (item2.category == PotionCategory && item1.category == FoodCategory)) {
      if (item1.item == EmptyBottle || item2.item == EmptyBottle) {
        GameItem item = item1.item == EmptyBottle ? item2 : item1;
        item.itemResult = "The " + item.name + " cannot fit inside the bottle.";
        return item;
      }
      GameItem potion = item1.category == PotionCategory ? item1 : item2;
      GameItem food = item1.category == FoodCategory ? item1 : item2;
      food.healthRecoverAmount = potion.healthRecoverAmount;
      food.hungerRecoverAmount = potion.hungerRecoverAmount;
      food.AOEsize = potion.AOEsize;
      food.AOEdamage = potion.AOEdamage;
      food.SpeedMultiplier = potion.SpeedMultiplier;
      food.name = "Odd " + (item1.category == FoodCategory ? item1.name : item2.name);
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
    nullItem.name = "Null";
    return nullItem;
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

void randomizeRingEffects() {
    for (int i = NUM_RINGS - 1; i > 0; i--) {
        int j = random(i + 1);
        std::swap(ringEffects[i], ringEffects[j]);
        std::swap(ringCursed[i], ringCursed[j]);
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
    GameItems randomItem = getRandomItemByRarityAnyCategory(maxRarity);
    
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