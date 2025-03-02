#include "Inventory.h"
#include "Player.h"

GameItem inventory[inventorySize];
int selectedInventoryIndex = 0; // Currently selected inventory item
String itemResultMessage = "";

InventoryPage inventoryPages[] = {
  {"Potions", PotionCategory},
  {"Materials", OtherCategory}
};
int currentInventoryPageIndex = 0;
int numInventoryPages = sizeof(inventoryPages)/sizeof(inventoryPages[0]);

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
  if (currentUIState != UI_INVENTORY) return;

  // Handle page switching
  if (buttons.leftPressed && !buttons.leftPressedPrev) {
    currentInventoryPageIndex = (currentInventoryPageIndex - 1 + numInventoryPages) % numInventoryPages;
    selectedInventoryIndex = findFirstItemInCurrentCategory();
  }
  if (buttons.rightPressed && !buttons.rightPressedPrev) {
    currentInventoryPageIndex = (currentInventoryPageIndex + 1) % numInventoryPages;
    selectedInventoryIndex = findFirstItemInCurrentCategory();
  }

  // Handle item navigation within category
  if (buttons.upPressed && !buttons.upPressedPrev) {
    int newIndex = findPreviousItemInCategory(selectedInventoryIndex);
    if (newIndex != -1) selectedInventoryIndex = newIndex;
  }
  if (buttons.downPressed && !buttons.downPressedPrev) {
    int newIndex = findNextItemInCategory(selectedInventoryIndex);
    if (newIndex != -1) selectedInventoryIndex = newIndex;
  }
}

int findFirstItemInCurrentCategory() {
  for (int i = 0; i < inventorySize; i++) {
    if (inventory[i].item != Null && 
      inventory[i].category == inventoryPages[currentInventoryPageIndex].category) {
      return i;
    }
  }
  return 0;
}

int findPreviousItemInCategory(int current) {
  for (int i = current - 1; i >= 0; i--) {
    if (inventory[i].item != Null && 
      inventory[i].category == inventoryPages[currentInventoryPageIndex].category) {
      return i;
    }
  }
  return -1;
}

int findNextItemInCategory(int current) {
  for (int i = current + 1; i < inventorySize; i++) {
    if (inventory[i].item != Null && 
      inventory[i].category == inventoryPages[currentInventoryPageIndex].category) {
      return i;
    }
  }
  return -1;
}

void handleInventoryItemUsage() {
  if (buttons.bPressed && !buttons.bPressedPrev && currentUIState == UI_INVENTORY) {
    GameItem &selectedItem = inventory[selectedInventoryIndex];
    
    if (strcmp(selectedItem.name.c_str(), "Empty") != 0) {
      if (!combiningTwoItems) {
        currentUIState = UI_ITEM_ACTION;
        selectedActionIndex = 0;
      } else {
        combiningItem2 = selectedItem;
        GameItem resultItem = CombineTwoItemsToGetItem(combiningItem1, combiningItem2);
        inventory[selectedInventoryIndex] = resultItem.name == "Null" ? inventory[selectedInventoryIndex] : resultItem;
        inventory[ingredient1index] = resultItem.name == "Null" ? inventory[ingredient1index] : GameItem{ Null, PotionCategory, "Empty", 0, 0, 0, 0, String(""), String(""), String("") };
        currentUIState = UI_ITEM_RESULT;
        itemResultMessage = resultItem.name == "Null" ? "These two items cannot be combined." : "Combining two items!";
        combiningTwoItems = false;
      }
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

  selectedActionIndex = selectedActionIndex == 4 ? 0 : selectedActionIndex == -1 ? 3 : selectedActionIndex;

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
      
      inventory[selectedInventoryIndex] = { Null, PotionCategory, "Empty", 0, 0, 0, 0, String(""), String(""), String("") };
      currentUIState = UI_ITEM_RESULT; // Change to result screen
      buttons.bPressedPrev = true;
    } else if (selectedActionIndex == 1) { // Drop
      inventory[selectedInventoryIndex] = { Null, PotionCategory, "Empty", 0, 0, 0, 0, String(""), String(""), String("") };
      currentUIState = UI_INVENTORY;
    } else if (selectedActionIndex == 2) { // Info
      currentUIState = UI_ITEM_INFO;
    } else if (selectedActionIndex == 3) {
      combiningTwoItems = true;
      combiningItem1 = selectedItem;
      ingredient1index = selectedInventoryIndex;
      currentUIState = UI_INVENTORY;
    }
  }
}

void renderInventory() {
  display.clearDisplay();
  display.setTextSize(1);

  if (currentUIState == UI_INVENTORY) {
    display.setCursor(combiningTwoItems ? 0 : 10, combiningTwoItems ? 0 : 3);
    display.setTextSize(combiningTwoItems ? 1 : 2);
    display.println(combiningTwoItems ? "Select second item tocombine..." : "Inventory");
    display.setTextSize(1);
    display.setCursor(10, 20);
    display.setTextColor(0);
    display.fillRect(0, 19, 128, 9, 15);
    String pageName = "<" + inventoryPages[currentInventoryPageIndex].name + ">";
    display.println(pageName.c_str());
    display.setTextColor(15);

    // Draw inventory items
    int yPos = 30;
    for (int i = 0; i < inventorySize; i++) {
      GameItem &item = inventory[i];
      if (item.item == Null || item.category != inventoryPages[currentInventoryPageIndex].category) 
        continue;

      display.setCursor(15, yPos);
      if (i == selectedInventoryIndex) display.print("> ");
      display.println(item.name);
      yPos += 12;
    }

    if (yPos == 30) { // No items
      display.setCursor(15, 30);
      display.println("Empty");
    }
  } 
  else if (currentUIState == UI_ITEM_INFO) {
    display.setCursor(0, 120);
    display.println(inventory[selectedInventoryIndex].originalName);
    display.setCursor(0, 10);
    display.print(inventory[selectedInventoryIndex].description);
  } 
  else if (currentUIState == UI_ITEM_RESULT) {
    display.setCursor(0, 10);
    display.println(itemResultMessage);
    if (buttons.bPressed && !buttons.bPressedPrev) {
      currentUIState = UI_NORMAL;
    }
  } 
  else if (currentUIState == UI_ITEM_ACTION) {    
    // Draw options menu box
    display.drawRect(50, 40, 65, 65, 15);
    display.fillRect(50, 40, 65, 12, 15);

    // Title
    display.setTextColor(0);
    display.setCursor(55, 42);
    display.println("Options:");
    display.setTextColor(15);

    // Options
    display.setCursor(55, 60);
    display.println(selectedActionIndex == 0 ? "> Use" : " Use");
    display.setCursor(55, 70);
    display.println(selectedActionIndex == 1 ? "> Drop" : " Drop");
    display.setCursor(55, 80);
    display.println(selectedActionIndex == 2 ? "> Info" : " Info");
    display.setCursor(55, 90);
    display.println(selectedActionIndex == 3 ? "> Combine" : " Combine");
  }

  display.display();
}
