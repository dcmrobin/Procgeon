#ifndef ITEM_H 
#define ITEM_H

#include <Arduino.h>

enum GameItems { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion, OrangePotion };

struct GameItem {
    GameItems item;
    String name;
    int healthRecoverAmount;
    int AOEsize;
    int AOEdamage;
    int SpeedMultiplier;
    String description;
};

void randomizePotionEffects();  // Call this once at game start
GameItem getItem(GameItems item);
void updatePotionName(GameItem &potion);  // Changes potion name when used
GameItems getRandomPotion(int randInt);
void resetPotionNames();

#endif // ITEM_H
