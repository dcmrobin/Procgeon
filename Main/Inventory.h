#ifndef INVENTORY_H 
#define INVENTORY_H

#include <Arduino.h>
#include "Item.h"
#include "HelperFunctions.h"
#include "Entities.h"

#define inventorySize 8

struct InventoryPage {
  String name;
  ItemCategory category;
  GameItem items[8]; // Each tab holds up to 8 items
  int itemCount = 0; // Track how many items are in this tab
};

extern InventoryPage inventoryPages[];
extern int selectedInventoryIndex; // Currently selected inventory item
extern char itemResultMessage[100];

//extern InventoryPage inventoryPages[];
extern int currentInventoryPageIndex;
extern int numInventoryPages;

extern bool identifyingItem;
extern int identifyScrollPage;
extern int identifyScrollIndex;

bool addToInventory(GameItem item, bool canBeCursed);
void handleInventoryNavigation();
int findFirstItemInCurrentCategory();
int findPreviousItemInCategory(int current);
int findNextItemInCategory(int current);
void handleInventoryItemUsage();
void handleItemActionMenu();
void renderInventory();
void identifyItem(GameItem &item);
void removeItemFromInventory(int page, int index);

#endif