#include "Shop.h"
#include "Item.h"
#include "Inventory.h"
#include "HelperFunctions.h"
#include "Player.h"
#include "GameAudio.h"
#include "Dungeon.h"
#include "Entities.h"

ShopItem shopItems[SHOP_MAX_ITEMS] = {};

struct InventoryEntry {
    int page;
    int index;
};

static bool shopSellMode = false;
static int  shopSellScrollOffset = 0;

static int getShopValue(const GameItem& item) {
    if (item.item == Null) return 0;
    int value = (7 * item.rarity) + g_state.dungeon;
    if (value < 1) value = 1;
    return value;
}

static void buildSellEntries(InventoryEntry entries[], int& outCount) {
    outCount = 0;
    for (int p = 0; p < numInventoryPages; p++) {
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            GameItem& item = inventoryPages[p].items[i];
            if (item.item == Null || item.isEquipped) continue;
            entries[outCount].page = p;
            entries[outCount].index = i;
            outCount++;
        }
    }
}

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
            snprintf(shopItems[i].item.name, sizeof(shopItems[i].item.name), "%s", shopItems[i].item.weapon.name);
        }
    }
    g_state.shopItemCount = SHOP_MAX_ITEMS;
}

void renderShop() {
    display.clearDisplay();

    // Show shop mode label
    display.setCursor(2, 2);
    display.print(shopSellMode ? "<Sell Mode>" : "<Buy Mode>");

    // Show player gold in top-left
    char goldStr[16];
    snprintf(goldStr, sizeof(goldStr), "Gold: %d", g_state.goldCount);
    display.setCursor(2, 120);
    display.print(goldStr);

    if (shopSellMode) {
        InventoryEntry entries[INVENTORY_SIZE * 6];
        int entryCount;
        buildSellEntries(entries, entryCount);

        if (entryCount == 0) {
            display.setCursor(5, 30);
            display.print("Nothing to sell");
        } else {
            if (g_state.shopSelectedIndex >= entryCount) g_state.shopSelectedIndex = entryCount - 1;
            if (g_state.shopSelectedIndex < 0) g_state.shopSelectedIndex = 0;

            const int visibleLines = SHOP_MAX_ITEMS;
            const int maxOffset = entryCount > visibleLines ? entryCount - visibleLines : 0;
            if (shopSellScrollOffset > maxOffset) shopSellScrollOffset = maxOffset;
            if (shopSellScrollOffset < 0) shopSellScrollOffset = 0;
            if (g_state.shopSelectedIndex < shopSellScrollOffset) shopSellScrollOffset = g_state.shopSelectedIndex;
            if (g_state.shopSelectedIndex >= shopSellScrollOffset + visibleLines)
                shopSellScrollOffset = g_state.shopSelectedIndex - visibleLines + 1;

            for (int i = 0; i < visibleLines; i++) {
                int entryIndex = shopSellScrollOffset + i;
                if (entryIndex >= entryCount) break;

                int y = 10 + i * 14;
                if (entryIndex == g_state.shopSelectedIndex) {
                    display.fillRect(0, y - 2, 128, 12, 0x5555);
                }

                GameItem& item = inventoryPages[entries[entryIndex].page].items[entries[entryIndex].index];
                display.setCursor(5, y);
                display.print(item.name);

                int sellPrice = getShopValue(item) / 2;
                if (sellPrice < 1) sellPrice = 1;
                char priceStr[12];
                snprintf(priceStr, sizeof(priceStr), "%dg", sellPrice);
                display.setCursor(100, y);
                display.print(priceStr);
            }
        }
    } else {
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
    }

    display.display();
}

void handleShopNavigation() {
    if (g_state.buttons.leftPressed && !g_state.buttons.leftPressedPrev) {
        playRawSFX(8);
        shopSellMode = !shopSellMode;
        shopSellScrollOffset = 0;
        g_state.shopSelectedIndex = 0;
    }

    if (g_state.buttons.rightPressed && !g_state.buttons.rightPressedPrev) {
        playRawSFX(8);
        shopSellMode = !shopSellMode;
        shopSellScrollOffset = 0;
        g_state.shopSelectedIndex = 0;
    }

    if (shopSellMode) {
        InventoryEntry entries[INVENTORY_SIZE * 6];
        int entryCount;
        buildSellEntries(entries, entryCount);
        if (entryCount == 0) return;

        if (g_state.buttons.upPressed && !g_state.buttons.upPressedPrev) {
            playRawSFX(8);
            g_state.shopSelectedIndex--;
            if (g_state.shopSelectedIndex < 0) g_state.shopSelectedIndex = entryCount - 1;
        }

        if (g_state.buttons.downPressed && !g_state.buttons.downPressedPrev) {
            playRawSFX(8);
            g_state.shopSelectedIndex++;
            if (g_state.shopSelectedIndex >= entryCount) g_state.shopSelectedIndex = 0;
        }

        if (g_state.buttons.bPressed && !g_state.buttons.bPressedPrev) {
            InventoryEntry entry = entries[g_state.shopSelectedIndex];
            GameItem& item = inventoryPages[entry.page].items[entry.index];
            int sellPrice = getShopValue(item) / 2;
            if (sellPrice < 1) sellPrice = 1;
            g_state.goldCount += sellPrice;
            playRawSFX(3);
            if (item.stackCount > 1) {
                item.stackCount--;
            } else {
                removeItemFromInventory(entry.page, entry.index);
            }
            entryCount--;
            if (g_state.shopSelectedIndex >= entryCount) g_state.shopSelectedIndex = entryCount - 1;
            if (g_state.shopSelectedIndex < 0) g_state.shopSelectedIndex = 0;
        }
    } else {
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
}