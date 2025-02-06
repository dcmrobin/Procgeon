#include "Item.h"
#include "Inventory.h"

GameItem itemList[] = {
  { RedPotion,  String("Red Potion"),  0,  0,  0, 0, String("Drink it to find out.") },
  { GreenPotion, String("Green Potion"), 0,  0,  0, 0, String("Drink it to find out.") },
  { BluePotion,  String("Blue Potion"),  0,  0,  0, 0, String("Drink it to find out.") },
  { BlackPotion, String("Black Potion"), 0,  0,  0, 0, String("Drink it to find out.") },
  { WhitePotion, String("White Potion"), 0,  0,  0, 0, String("Drink it to find out.") },
  { YellowPotion, String("Yellow Potion"), 0,  0,  0, 0, String("Drink it to find out.") },
  { WhitePotion, String("Orange Potion"), 0,  0,  0, 0, String("Drink it to find out.") }
};

// Possible potion effects
struct PotionEffect {
  int healthChange;
  int AOEsize;
  int AOEdamage;
  int SpeedMultiplier;
  String effectName;
  String effectDescription;
};

// Possible effect pool
PotionEffect potionEffects[] = {
  { 20,  0,  0, 0, String("Healing Potion"), String("Healing. Heals 20 of your HP.") },
  { -20, 0,  0, 0, String("Diluted Poison"), String("Deducts 20 of your HP. Don't drink. Unless your guilty of something...") },
  { 0,   2, 40, 0, String("Explosion Potion"), String("Bomb. Deals 40 damage to enemies around you.") },
  { 40,   2, -30, 0, String("Buffing Potion"), String("Heals 40 of your HP, but also heals 30 HP of enemies around you.") },
  { 70,  0,  0, 0, String("Mega Heal Potion"), String("Healing, but mega. Heals 70 of your HP.") },
  { -50,  4,  -20, 0, String("Bad Potion"), String("It deducts 50 of your HP, and gives enemies around you 20 HP. Maybe don't drink this.") },
  { 0,  0,  0, 2, String("Speed Potion"), String("Drink this, and you'll go twice as fast.") }
};

void randomizePotionEffects() {
  // Shuffle the potion effects array
  for (int i = 6; i > 0; i--) {
    int j = random(i + 1);  // Random index from 0 to i inclusive
    // Swap effects
    PotionEffect temp = potionEffects[i];
    potionEffects[i] = potionEffects[j];
    potionEffects[j] = temp;
  }

  // Assign shuffled effects to potions
  for (int i = 0; i < 7; i++) {
    itemList[i].healthRecoverAmount = potionEffects[i].healthChange;
    itemList[i].AOEsize = potionEffects[i].AOEsize;
    itemList[i].AOEdamage = potionEffects[i].AOEdamage;
    itemList[i].SpeedMultiplier = potionEffects[i].SpeedMultiplier;
  }
}

GameItem getItem(GameItems item) {
  return itemList[item];
}

void updatePotionName(GameItem &potion) {
  for (PotionEffect effect : potionEffects) {
    if (potion.healthRecoverAmount == effect.healthChange &&
        potion.AOEsize == effect.AOEsize &&
        potion.AOEdamage == effect.AOEdamage && 
        potion.SpeedMultiplier == effect.SpeedMultiplier) {

      // Update potion name in the master item list
      for (int i = 0; i < 7; i++) {
        if (itemList[i].item == potion.item) {  
          itemList[i].name = effect.effectName;
          itemList[i].description = effect.effectDescription;
        }
      }

      // Update all instances of the potion in the inventory
      for (int i = 0; i < inventorySize; i++) {
        if (inventory[i].item == potion.item) {
          inventory[i].name = effect.effectName;
          inventory[i].description = effect.effectDescription;
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
}


GameItems getRandomPotion(int randInt) {
  GameItems potions[] = { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion, OrangePotion };
  return potions[randInt % 7];  // Ensure it's within bounds
}

void applyAOEEffect(float centerX, float centerY, int aoeRadius, int aoeDamage, int& kills) {
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