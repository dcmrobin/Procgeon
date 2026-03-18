#include "Shop.h"
#include "Item.h"
#include "Inventory.h"
#include "HelperFunctions.h"
#include "Player.h"
#include "GameAudio.h"
#include "Dungeon.h"
#include "Entities.h"

ShopItem shopItems[SHOP_MAX_ITEMS] = {};

void setupShopItems() {
    for (int i = 0; i < SHOP_MAX_ITEMS; i++) {
        // Pick a random item, re-rolling if it's Null or KingArmor
        int itemCount = sizeof(itemList) / sizeof(itemList[0]);
        int j;
        do {
            j = random(0, itemCount);
        } while (itemList[j].item == Null || itemList[j].item == KingArmor);

        shopItems[i].item = itemList[j];
        shopItems[i].price = (random(5, 10) * shopItems[i].item.rarity) + g_state.dungeon;

        if (itemList[j].item == Weapon) {
            int weaponCount = sizeof(weaponList) / sizeof(weaponList[0]);
            shopItems[i].item.weapon = weaponList[random(0, weaponCount)];
            snprintf(shopItems[i].item.name, sizeof(shopItems[i].item.name), "%d", shopItems[i].item.weapon.name);
        }
    }
    g_state.shopItemCount = SHOP_MAX_ITEMS;
}

void renderShop() {
    display.clearDisplay();

    // Show player gold in top-left
    char goldStr[16];
    snprintf(goldStr, sizeof(goldStr), "Gold: %d", g_state.goldCount);
    display.setCursor(2, 120);
    display.print(goldStr);

    for (int i = 0; i < SHOP_MAX_ITEMS; i++) {
        int y = 10 + i * 14;
        if (i == g_state.shopSelectedIndex) {
            display.fillRect(0, y - 2, 128, 12, 0x5555);
        }
        display.setCursor(5, y);
        display.print(shopItems[i].item.name);

        if (shopItems[i].price > 0) {
            char priceStr[10];
            snprintf(priceStr, sizeof(priceStr), "%dg", shopItems[i].price);
            display.setCursor(100, y);
            display.print(priceStr);
        }
    }

    display.display();
}

void handleShopNavigation() {
    // Handle scrolling up and down the wrapping list of items, and buying the selected item if the player has enough gold.

    if (g_state.buttons.upPressed && !g_state.buttons.upPressedPrev) {
        playRawSFX(8);
        g_state.shopSelectedIndex--;
        if (g_state.shopSelectedIndex < 0) {
            g_state.shopSelectedIndex = g_state.shopItemCount - 1;
        }
    }
    
    if (g_state.buttons.downPressed && !g_state.buttons.downPressedPrev) {
        playRawSFX(8);
        g_state.shopSelectedIndex++;
        if (g_state.shopSelectedIndex > g_state.shopItemCount - 1) {
            g_state.shopSelectedIndex = 0;
        }
    }

    if (g_state.buttons.bPressed && !g_state.buttons.bPressedPrev) {
        if (shopItems[g_state.shopSelectedIndex].item.item != Null && g_state.goldCount >= shopItems[g_state.shopSelectedIndex].price) {
            addToInventory(shopItems[g_state.shopSelectedIndex].item, true);
            playRawSFX(3);
            shopItems[g_state.shopSelectedIndex].item = { Null, PotionCategory, "Sold" };
            g_state.goldCount -= shopItems[g_state.shopSelectedIndex].price;
            shopItems[g_state.shopSelectedIndex].price = 0;
        }
    }
}