#ifndef INVENTORY_H 
#define INVENTORY_H

#include <Arduino.h>
#include "Item.h"
#include "HelperFunctions.h"
#include "Entities.h"

#define inventorySize 8

extern GameItem inventory[inventorySize];
extern int selectedInventoryIndex; // Currently selected inventory item

bool addToInventory(GameItem item);
void handleInventoryNavigation();
void handleInventoryItemUsage();
void handleItemActionMenu(int& playerHP, float playerX, float playerY, String& deathCause, bool& speeding, int& kills, int& speedTimer);

#endif