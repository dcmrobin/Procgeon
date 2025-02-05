#ifndef INVENTORY_H 
#define INVENTORY_H

#include <Arduino.h>
#include "Item.h"

#define inventorySize 8

extern GameItem inventory[inventorySize];
extern int selectedInventoryIndex; // Currently selected inventory item

#endif