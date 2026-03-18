#ifndef SHOP_H
#define SHOP_H

#include "Common.h"
#include "GameState.h"
#include "Item.h"

struct ShopItem {
    GameItem item = {Null, PotionCategory, "Null"};
    int price = 0;
};

extern ShopItem shopItems[SHOP_MAX_ITEMS];

void setupShopItems();
void renderShop();
void handleShopNavigation();

#endif