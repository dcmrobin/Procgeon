#include "Item.h"

GameItem itemList[] = {
  { RedPotion,  "Red Potion",  0,  0,  0 },
  { GreenPotion, "Green Potion", 0,  0,  0 },
  { BluePotion,  "Blue Potion",  0,  0,  0 },
  { BlackPotion, "Black Potion", 0,  0,  0 },
  { WhitePotion, "White Potion", 0,  0,  0 },
  { YellowPotion, "Yellow Potion", 0,  0,  0 }
};

// Possible potion effects
struct PotionEffect {
  int healthChange;
  int AOEsize;
  int AOEdamage;
  const char* effectName;
};

// Possible effect pool
PotionEffect potionEffects[] = {
  { 20,  0,  0, "Healing Potion" },     // Heals player
  { -20, 0,  0, "Hurt Potion" },        // Damages player
  { 0,   4, 10, "Explosion Potion" },   // Damages enemies in AOE
  { 0,   4, -10, "Annoying Potion" }, // Heals enemies in AOE
  { 70,  0,  0, "Mega Heal Potion" },    // Large player heal
  { -50,  10,  -20, "Bad Potion" }    // bad
};

// Randomize potion effects at game start
void randomizePotionEffects() {
  for (int i = 0; i < 5; i++) {
    int effectIndex = random(0, 5);  // Pick a random effect
    itemList[i].healthRecoverAmount = potionEffects[effectIndex].healthChange;
    itemList[i].AOEsize = potionEffects[effectIndex].AOEsize;
    itemList[i].AOEdamage = potionEffects[effectIndex].AOEdamage;
    itemList[i].name = potionEffects[effectIndex].effectName;
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
      potion.name = effect.effectName;  // Change the name to match effect
      break;
    }
  }
}

GameItems getRandomPotion(int randInt) {
  switch (randInt) {
    case 0:
      return RedPotion;
      break;

    case 1:
      return GreenPotion;
      break;

    case 2:
      return BluePotion;
      break;

    case 3:
      return BlackPotion;
      break;

    case 4:
      return WhitePotion;
      break;

    case 5:
      return YellowPotion;
      break;
  }

  return RedPotion;
}
