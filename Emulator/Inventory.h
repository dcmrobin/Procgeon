#ifndef INVENTORY_H
#define INVENTORY_H

#include "Common.h"
#include "GameState.h"
#include "Item.h"

// ─────────────────────────────────────────────────────────────────────────────
// InventoryPage
// ─────────────────────────────────────────────────────────────────────────────
struct InventoryPage {
    char        name[30]                = "";
    ItemCategory category               = PotionCategory;
    GameItem    items[INVENTORY_SIZE]   = {};
    int         itemCount               = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// Global inventory (defined in Inventory.cpp)
// ─────────────────────────────────────────────────────────────────────────────
extern InventoryPage inventoryPages[];
extern int           numInventoryPages;

// ─────────────────────────────────────────────────────────────────────────────
// Inventory functions
// ─────────────────────────────────────────────────────────────────────────────
bool addToInventory(GameItem item, bool canBeCursed);
void removeItemFromInventory(int page, int index);

void handleInventoryNavigation();
void handleInventoryItemUsage();
void handleItemActionMenu();
void renderInventory();

void identifyItem(GameItem& item);

int  findFirstItemInCurrentCategory();
int  findNextItemInCategory(int current);
int  findPreviousItemInCategory(int current);

#endif // INVENTORY_H