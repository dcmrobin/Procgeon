#ifndef INVENTORY_H 
#define INVENTORY_H

#include <Arduino.h>
#include "Item.h"
#include "HelperFunctions.h"
#include "Entities.h"

#define inventorySize 8

extern GameItem inventory[inventorySize];
extern int selectedInventoryIndex; // Currently selected inventory item
extern String itemResultMessage;

struct InventoryPage {
  String name;
  ItemCategory category;
};

extern InventoryPage inventoryPages[];
extern int currentInventoryPageIndex;
extern int numInventoryPages;

bool addToInventory(GameItem item);
void handleInventoryNavigation();
int findFirstItemInCurrentCategory();
int findPreviousItemInCategory(int current);
int findNextItemInCategory(int current);
void handleInventoryItemUsage();
void handleItemActionMenu();
void renderInventory();

#endif