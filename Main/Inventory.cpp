#include "Inventory.h"

GameItem inventory[inventorySize];
int selectedInventoryIndex = 0; // Currently selected inventory item
String itemResultMessage = "";

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

void handleItemActionMenu(int& playerHP, int& playerMaxHP, float playerX, float playerY, String& deathCause, bool& speeding, int& kills, int& speedTimer) {
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
        playerHP = playerHP > playerMaxHP ? playerMaxHP : playerHP;

        if (speeding) {
          speedTimer += 1000;
        }
        speeding = selectedItem.SpeedMultiplier > 0 ? true : false;

        if (playerHP <= 0) {
          deathCause = "poison";
          buttons.bPressedPrev = true;
        }
        
        if (selectedItem.AOEsize > 0) {
          applyAOEEffect(playerX, playerY, selectedItem.AOEsize, selectedItem.AOEdamage, kills);
        }
        
        for (int i = 0; i < inventorySize; i++) {
          if (inventory[i].item == selectedItem.item) {
            updatePotionName(inventory[i]);
          }
        }
      }

      itemResultMessage = selectedItem.itemResult;
      
      inventory[selectedInventoryIndex] = { Null, "Empty", 0, 0, 0 };
      currentUIState = UI_ITEM_RESULT; // Change to result screen
      buttons.bPressedPrev = true;
    } else if (selectedActionIndex == 1) { // Drop
      inventory[selectedInventoryIndex] = { Null, "Empty", 0, 0, 0 };
      currentUIState = UI_INVENTORY;
    } else { // Info
      currentUIState = UI_ITEM_INFO;
    }
  }
}

void renderInventory() {
  u8g2.clearBuffer();

  // Draw inventory title
  if (currentUIState == UI_INVENTORY) {
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(10, 15, "Inventory");

    // Draw inventory items
    u8g2.setFont(u8g2_font_profont12_tr);
    for (int i = 0; i < inventorySize; i++) {
      int yPos = 30 + (i * 12);
      if (i == selectedInventoryIndex) {
        u8g2.drawStr(5, yPos, ">");
      }
      u8g2.drawStr(15, yPos, inventory[i].name.c_str());
    }
  } else if (currentUIState == UI_ITEM_INFO) {
    u8g2.setFont(u8g2_font_profont12_tr);
    u8g2.drawStr(3, 125, inventory[selectedInventoryIndex].originalName.c_str());
    drawWrappedText(inventory[selectedInventoryIndex].description.c_str(), 3, 10, SCREEN_WIDTH - 6, 12);
  } else if (currentUIState == UI_ITEM_RESULT) {
    u8g2.setFont(u8g2_font_profont12_tr);
    drawWrappedText(itemResultMessage.c_str(), 3, 10, SCREEN_WIDTH - 6, 12);
    if (buttons.bPressed && !buttons.bPressedPrev) {
      currentUIState = UI_NORMAL;
    }
  } else if (currentUIState == UI_ITEM_ACTION) {    
    // Background
    u8g2.drawFrame(50, 40, 60, 50);
    u8g2.drawBox(50, 40, 60, 12);
    
    // Title
    u8g2.setFont(u8g2_font_profont12_tr);
    u8g2.setDrawColor(0);
    u8g2.drawStr(55, 50, "Options:");
    u8g2.setDrawColor(1);
    
    // Actions
    u8g2.drawStr(55, 63, selectedActionIndex == 0 ? "> Use" : "  Use");
    u8g2.drawStr(55, 73, selectedActionIndex == 1 ? "> Drop" : "  Drop");
    u8g2.drawStr(55, 83, selectedActionIndex == 2 ? "> Info" : "  Info");
  }

  u8g2.sendBuffer();
}