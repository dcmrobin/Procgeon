#ifndef ITEM_H 
#define ITEM_H

#include <Arduino.h>

enum GameItems { RedPotion, GreenPotion, BluePotion, BlackPotion, WhitePotion, YellowPotion };

struct GameItem {
    GameItems item;
    String name;
    int healthRecoverAmount;
    int AOEsize;
    int AOEdamage;
};

void randomizePotionEffects();  // Call this once at game start
GameItem getItem(GameItems item);
void updatePotionName(GameItem &potion);  // Changes potion name when used
GameItems getRandomPotion(int randInt);
void resetPotionNames();

#endif // ITEM_H
