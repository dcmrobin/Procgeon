#include "Inventory.h"

GameItem inventory[inventorySize];
int selectedInventoryIndex = 0; // Currently selected inventory item

// Add an item to the first empty slot
bool addToInventory(GameItem item) {
  for (int i = 0; i < inventorySize; i++) {
    if (strcmp(inventory[i].name.c_str(), "Empty") == 0) { // Check for empty slot
        inventory[i] = item;
        return true; // Successfully added
    }
  }
  return false; // Inventory full
}

void handleInventoryNavigation() {
  if (buttons.upPressed && !buttons.upPressedPrev && selectedInventoryIndex > 0) {
    selectedInventoryIndex--;
  }
  if (buttons.downPressed && !buttons.downPressedPrev && selectedInventoryIndex < inventorySize - 1) {
    selectedInventoryIndex++;
  }
}

void handleInventoryItemUsage() {
  if (buttons.bPressed && !buttons.bPressedPrev && currentUIState == UI_INVENTORY) {
    GameItem &selectedItem = inventory[selectedInventoryIndex];
    
    if (strcmp(selectedItem.name.c_str(), "Empty") != 0) {
      currentUIState = UI_ITEM_ACTION;
      selectedActionIndex = 0;
    }
  }
}

void handleItemActionMenu(int& playerHP, float playerX, float playerY, String& deathCause, bool& speeding, int& kills) {
  // Navigation
  if (buttons.upPressed && !buttons.upPressedPrev) {
    selectedActionIndex--;
  }
  if (buttons.downPressed && !buttons.downPressedPrev) {
    selectedActionIndex++;
  }

  selectedActionIndex = selectedActionIndex == 3 ? 0 : selectedActionIndex == -1 ? 0 : selectedActionIndex;

  // Cancel with A
  if (buttons.aPressed && !buttons.aPressedPrev) {
    currentUIState = UI_INVENTORY;
  }

  // Confirm with B
  if (buttons.bPressed && !buttons.bPressedPrev) {
    GameItem &selectedItem = inventory[selectedInventoryIndex];
    
    if (selectedActionIndex == 0) { // Use
      if (selectedItem.item >= RedPotion && selectedItem.item <= OrangePotion) {
        playerHP += selectedItem.healthRecoverAmount;
        speeding = selectedItem.SpeedMultiplier > 0 ? true : false;

        if (playerHP <= 0) deathCause = "poison";
        
        if (selectedItem.AOEsize > 0) {
          applyAOEEffect(playerX, playerY, selectedItem.AOEsize, selectedItem.AOEdamage, kills);
        }
        
        for (int i = 0; i < inventorySize; i++) {
          if (inventory[i].item == selectedItem.item) {
            updatePotionName(inventory[i]);
          }
        }
      }
      inventory[selectedInventoryIndex] = { Null, "Empty", 0, 0, 0 };
    }
    else if (selectedActionIndex == 1) { // Drop
      inventory[selectedInventoryIndex] = { Null, "Empty", 0, 0, 0 };
    } else { // Info
      currentUIState = UI_ITEM_INFO;
    }
    
    if (currentUIState != UI_ITEM_INFO) {
      currentUIState = UI_INVENTORY;
    }
  }
}