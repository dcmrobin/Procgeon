#include "Item.h"

GameItem itemList[] = {
  { RedPotion,  String("Red Potion"),  0,  0,  0 },
  { GreenPotion, String("Green Potion"), 0,  0,  0 },
  { BluePotion,  String("Blue Potion"),  0,  0,  0 },
  { BlackPotion, String("Black Potion"), 0,  0,  0 },
  { WhitePotion, String("White Potion"), 0,  0,  0 },
  { YellowPotion, String("Yellow Potion"), 0,  0,  0 }
};

// Possible potion effects
struct PotionEffect {
  int healthChange;
  int AOEsize;
  int AOEdamage;
  String effectName;
};

// Possible effect pool
PotionEffect potionEffects[] = {
  { 20,  0,  0, String("Healing Potion") },     // Heals player
  { -20, 0,  0, String("Diluted Poison") },        // Damages player
  { 0,   2, 40, String("Explosion Potion") },   // Damages enemies in AOE
  { 40,   2, -30, String("Buffing Potion") }, // Heals enemies in AOE
  { 70,  0,  0, String("Mega Heal Potion") },    // Large player heal
  { -50,  4,  -20, String("Bad Potion") }    // bad
};

// Randomize potion effects at game start
void randomizePotionEffects() {
  for (int i = 0; i < 6; i++) {
    int effectIndex = random(0, 6);  // Pick a random effect
    itemList[i].healthRecoverAmount = potionEffects[effectIndex].healthChange;
    itemList[i].AOEsize = potionEffects[effectIndex].AOEsize;
    itemList[i].AOEdamage = potionEffects[effectIndex].AOEdamage;
  }
}

GameItem getItem(GameItems item) {
  return itemList[item];
}

// When a potion is used, update its name
void updatePotionName(GameItem &potion) {
  for (PotionEffect effect : potionEffects) {
    if (potion.healthRecoverAmount == effect.healthChange &&
        potion.AOEsize == effect.AOEsize &&
        potion.AOEdamage == effect.AOEdamage) {

      // Update the potion's name in the master item list
      for (int i = 0; i < 6; i++) {
        if (itemList[i].item == potion.item) {  
          itemList[i].name = effect.effectName;
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
}


GameItems getRandomPotion(int randInt) {
  GameItems potions[] = { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion };
  return potions[randInt % 6];  // Ensure it's within bounds
}
