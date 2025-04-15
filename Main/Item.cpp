#include "Item.h"
#include "Inventory.h"
#include "HelperFunctions.h"
#include "Player.h"

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
  { Mushroom, FoodCategory, String("Mushroom"), 0,  20,  0, 0, 0, String("It is edible."), String("Mushroom"), String("You become less hungry.")},
  { EmptyBottle, PotionCategory, String("Empty Bottle"), 0,  20,  0, 0, 0, String("It is an empty bottle."), String("Empty Bottle"), String("Nothing happens. It's an empty bottle."), false},
  { RiddleStone, EquipmentCategory, String("Riddle Stone"), 0,  0,  0, 0, 0, String("Looks like it could be used for many things..."), String("Riddle Stone"), String("Solve this riddle!")}
};

// Possible effect pool
PotionEffect potionEffects[] = {
  { 20,  0,  0, 0, String("Healing Potion"), String("Healing. Heals 20 of your HP."), String("You feel better.") },
  { -20, 0,  0, 0, String("Diluted Poison"), String("Deducts 20 of your HP. Don't drink. Unless your guilty of something..."), String("You lose 20 HP.") },
  { 0,   4, 40, 0, String("Explosion Potion"), String("Bomb. Deals 40 damage to enemies around you."), String("The enemies around you lose 40 HP.") },
  { 40,   4, -30, 0, String("Buffing Potion"), String("Heals 40 of your HP, but also heals 30 HP of enemies around you."), String("You feel better, but so do the enemies close to you.") },
  { 70,  0,  0, 0, String("Mega Heal Potion"), String("Healing, but mega. Heals 70 of your HP."), String("You feel much better.") },
  { -50,  4,  -20, 0, String("Bad Potion"), String("It deducts 50 of your HP, and gives enemies around you 20 HP. Maybe don't drink this."), String("You lose 50 HP, and the enemies around you gain 20 HP.") },
  { 0,  0,  0, 2, String("Speed Potion"), String("Drink this, and you'll go twice as fast."), String("Your are faster now, but only for a limited amount of time.") },
  { 0,  0,  0, 0.4, String("Slowing Potion"), String("Drink this, and you'll go half as fast."), String("Your are slower now, but only for a limited amount of time.") },
  { 0,  0,  0, 0, String("Hunger Potion"), String("Makes you more hungry."), String("You are now more hungry.") },
  { 0,  0,  0, 0, String("See-all Potion"), String("Opens your eyes to the unseen."), String("You can now see that which was unseen for a limited time.") }
};

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
  }
}

GameItem getItem(GameItems item) {
  GameItem newItem = itemList[item];
  return newItem;
}

void updatePotionName(GameItem &potion) {
  for (PotionEffect effect : potionEffects) {
    if (potion.healthRecoverAmount == effect.healthChange &&
        potion.AOEsize == effect.AOEsize &&
        potion.AOEdamage == effect.AOEdamage && 
        potion.SpeedMultiplier == effect.SpeedMultiplier) {

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

void resetPotionNames() {
  itemList[0].name = "Red Potion";
  itemList[1].name = "Green Potion";
  itemList[2].name = "Blue Potion";
  itemList[3].name = "Black Potion";
  itemList[4].name = "White Potion";
  itemList[5].name = "Yellow Potion";
  itemList[6].name = "Orange Potion";
  itemList[7].name = "Purple Potion";
  itemList[8].name = "Cyan Potion";
}


GameItems getRandomPotion(int randInt) {
  GameItems potions[] = { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion };// Only pick a potion out of the primary color potions
  return potions[randInt % 6];  // Ensure it's within bounds
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

GameItem CombineTwoItemsToGetItem(GameItem item1, GameItem item2) {
  if ((areItemsEqual(item1, getItem(BluePotion)) && areItemsEqual(item2, getItem(YellowPotion))) || (areItemsEqual(item2, getItem(BluePotion)) && areItemsEqual(item1, getItem(YellowPotion)))) {return getItem(GreenPotion);}
  if ((areItemsEqual(item1, getItem(RedPotion)) && areItemsEqual(item2, getItem(GreenPotion))) || (areItemsEqual(item2, getItem(RedPotion)) && areItemsEqual(item1, getItem(GreenPotion)))) {return getItem(YellowPotion);}
  if ((areItemsEqual(item1, getItem(RedPotion)) && areItemsEqual(item2, getItem(YellowPotion))) || (areItemsEqual(item2, getItem(RedPotion)) && areItemsEqual(item1, getItem(YellowPotion)))) {return getItem(OrangePotion);}
  if ((areItemsEqual(item1, getItem(RedPotion)) && areItemsEqual(item2, getItem(BluePotion))) || (areItemsEqual(item2, getItem(RedPotion)) && areItemsEqual(item1, getItem(BluePotion)))) {return getItem(PurplePotion);}
  if ((areItemsEqual(item1, getItem(GreenPotion)) && areItemsEqual(item2, getItem(BluePotion))) || (areItemsEqual(item2, getItem(GreenPotion)) && areItemsEqual(item1, getItem(BluePotion)))) {return getItem(CyanPotion);}
  return {};
}