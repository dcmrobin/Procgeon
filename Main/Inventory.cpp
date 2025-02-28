#include "Inventory.h"
#include "Player.h"

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

void handleItemActionMenu() {
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
          applyAOEEffect(playerX, playerY, selectedItem.AOEsize, selectedItem.AOEdamage);
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
  display.clearDisplay();
  display.setTextSize(1);

  if (currentUIState == UI_INVENTORY) {
    display.setCursor(10, 3);
    display.setTextSize(2);
    display.println("Inventory");
    display.setTextSize(1);
    display.setCursor(10, 20);
    display.println("<Potions>");

    // Draw inventory items
    for (int i = 0; i < inventorySize; i++) {
      int yPos = 30 + (i * 12);
      display.setCursor(15, yPos);
      if (i == selectedInventoryIndex) {
        display.print("> ");
      }
      display.println(inventory[i].name);
    }
  } 
  else if (currentUIState == UI_ITEM_INFO) {
    display.setCursor(3, 120);
    display.println(inventory[selectedInventoryIndex].originalName);
    display.setCursor(3, 10);
    display.print(inventory[selectedInventoryIndex].description);
  } 
  else if (currentUIState == UI_ITEM_RESULT) {
    display.setCursor(3, 10);
    display.println(itemResultMessage);
    if (buttons.bPressed && !buttons.bPressedPrev) {
      currentUIState = UI_NORMAL;
    }
  } 
  else if (currentUIState == UI_ITEM_ACTION) {    
    // Draw options menu box
    display.drawRect(50, 40, 60, 50, 15);
    display.fillRect(50, 40, 60, 12, 15);

    // Title
    display.setTextColor(0);
    display.setCursor(55, 45);
    display.println("Options:");
    display.setTextColor(15);

    // Options
    display.setCursor(55, 60);
    display.println(selectedActionIndex == 0 ? "> Use" : "  Use");
    display.setCursor(55, 70);
    display.println(selectedActionIndex == 1 ? "> Drop" : "  Drop");
    display.setCursor(55, 80);
    display.println(selectedActionIndex == 2 ? "> Info" : "  Info");
  }

  display.display();
}
