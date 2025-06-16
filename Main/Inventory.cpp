#include "Inventory.h"
#include "Player.h"
#include "GameAudio.h"

int selectedInventoryIndex = 0; // Currently selected inventory item
String itemResultMessage = "";

InventoryPage inventoryPages[] = {
  {"Potions", PotionCategory},
  {"Food", FoodCategory},
  {"Equipment", EquipmentCategory}
};
int currentInventoryPageIndex = 0;
int numInventoryPages = sizeof(inventoryPages)/sizeof(inventoryPages[0]);

bool addToInventory(GameItem item) {
  // Find the matching tab category
  for (int p = 0; p < numInventoryPages; p++) {
    if (inventoryPages[p].category == item.category) {
      // Check if the tab has space
      if (inventoryPages[p].itemCount >= 8) return false;

      // Find first empty slot in the tab
      for (int i = 0; i < 8; i++) {
        if (inventoryPages[p].items[i].item == Null) {
          inventoryPages[p].items[i] = item;
          inventoryPages[p].itemCount++;
          return true;
        }
      }
    }
  }
  return false; // No matching tab found
}

void handleInventoryNavigation() {
  if (currentUIState != UI_INVENTORY) return;

  // Handle page switching
  if (buttons.leftPressed && !buttons.leftPressedPrev) {
    playRawSFX(8);
    currentInventoryPageIndex = (currentInventoryPageIndex - 1 + numInventoryPages) % numInventoryPages;
    selectedInventoryIndex = findFirstItemInCurrentCategory();
  }
  if (buttons.rightPressed && !buttons.rightPressedPrev) {
    playRawSFX(8);
    currentInventoryPageIndex = (currentInventoryPageIndex + 1) % numInventoryPages;
    selectedInventoryIndex = findFirstItemInCurrentCategory();
  }

  if (buttons.upPressed && !buttons.upPressedPrev) {
    playRawSFX(8);
    selectedInventoryIndex = findPreviousItemInCategory(selectedInventoryIndex);
  }
  if (buttons.downPressed && !buttons.downPressedPrev) {
    playRawSFX(8);
    selectedInventoryIndex = findNextItemInCategory(selectedInventoryIndex);
  }
}

int findFirstItemInCurrentCategory() {
  InventoryPage &currentPage = inventoryPages[currentInventoryPageIndex];
  for (int i = 0; i < 8; i++) {
    if (currentPage.items[i].item != Null) {
      return i; // Index within the current tab
    }
  }
  return 0;
}

int findNextItemInCategory(int current) {
  int category = inventoryPages[currentInventoryPageIndex].category;

  // Check from current + 1 to the end
  for (int i = current + 1; i < inventorySize; i++) {
    if (inventoryPages[currentInventoryPageIndex].items[i].item != Null && inventoryPages[currentInventoryPageIndex].items[i].category == category) {
      return i;
    }
  }

  // Wrap around: check from start to current
  for (int i = 0; i <= current; i++) {
    if (inventoryPages[currentInventoryPageIndex].items[i].item != Null && inventoryPages[currentInventoryPageIndex].items[i].category == category) {
      return i;
    }
  }

  return -1; // No items found
}

int findPreviousItemInCategory(int current) {
  int category = inventoryPages[currentInventoryPageIndex].category;

  // Check from current - 1 down to 0
  for (int i = current - 1; i >= 0; i--) {
    if (inventoryPages[currentInventoryPageIndex].items[i].item != Null && inventoryPages[currentInventoryPageIndex].items[i].category == category) {
      return i;
    }
  }

  // Wrap around: check from end down to current
  for (int i = inventorySize - 1; i >= current; i--) {
    if (inventoryPages[currentInventoryPageIndex].items[i].item != Null && inventoryPages[currentInventoryPageIndex].items[i].category == category) {
      return i;
    }
  }

  return -1; // No items found
}

void handleInventoryItemUsage() {
  if (buttons.bPressed && !buttons.bPressedPrev && currentUIState == UI_INVENTORY) {
    GameItem &selectedItem = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];
  
    if (selectedItem.name != "Empty" && selectedItem.name != "") {
      playRawSFX(7);
      if (!combiningTwoItems) {
        currentUIState = UI_ITEM_ACTION;
        selectedActionIndex = 0;
      } else {
        combiningItem2 = selectedItem;
        GameItem resultItem = CombineTwoItemsToGetItem(combiningItem1, combiningItem2);

        inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = resultItem.name == "Null" ? inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] : resultItem;// Assign the result item to the selected item's inventory index if the crafting worked
        inventoryPages[currentInventoryPageIndex].items[ingredient1index] = resultItem.name == "Null" ? inventoryPages[currentInventoryPageIndex].items[ingredient1index] : GameItem{ Null, PotionCategory, "Empty"};// Assign Null to the first ingredient's inventory index if the crafting worked
        
        if (inventoryPages[currentInventoryPageIndex].items[ingredient1index].category == PotionCategory && inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].category == PotionCategory) {
          inventoryPages[currentInventoryPageIndex].items[ingredient1index] = resultItem.name == "Null" ? inventoryPages[currentInventoryPageIndex].items[ingredient1index] : getItem(EmptyBottle);// If both ingredients were potions, have a left over bottle item after the crafting, in addition to the result item
        }
        currentUIState = UI_ITEM_RESULT;
        itemResultMessage = resultItem.name == "Null" ? "These two items cannot be combined." : "Combined two items! The result was: " + resultItem.name;
        combiningTwoItems = false;
      }
    }
  }
}

void handleItemActionMenu() {
  // Navigation
  if (buttons.upPressed && !buttons.upPressedPrev) {
    playRawSFX(8);
    selectedActionIndex--;
  }
  if (buttons.downPressed && !buttons.downPressedPrev) {
    playRawSFX(8);
    selectedActionIndex++;
  }

  selectedActionIndex = selectedActionIndex == 4 ? 0 : selectedActionIndex == -1 ? 3 : selectedActionIndex;

  // Cancel with A
  if (buttons.aPressed && !buttons.aPressedPrev) {
    currentUIState = UI_INVENTORY;
  }

  // Confirm with B
  if (buttons.bPressed && !buttons.bPressedPrev) {
    playRawSFX(7);
    GameItem &selectedItem = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];
    
    if (selectedActionIndex == 0) { // Use
      if (selectedItem.category == PotionCategory && selectedItem.item != EmptyBottle) {
        playRawSFX(6);
        playerHP += selectedItem.healthRecoverAmount;
        playerHP = playerHP > playerMaxHP ? playerMaxHP : playerHP;

        if (speeding) {
          speedTimer += 1000;
        }

        if (selectedItem.SpeedMultiplier != 0) {
          speeding = true;
          currentSpeedMultiplier = selectedItem.SpeedMultiplier;
        }

        if (playerHP <= 0) {
          playRawSFX(10);
          deathCause = "poison";
          buttons.bPressedPrev = true;
        }
        
        if (selectedItem.AOEsize > 0) {
          applyAOEEffect(playerX, playerY, selectedItem.AOEsize, selectedItem.AOEdamage);
        }

        if (selectedItem.itemResult == "You are now more hungry.") {
          playerFood -= 51;//heheheh u want it to be 50 don't u >:D
        } else if (selectedItem.itemResult == "You can now see that which was unseen for a limited time.") {
          seeAll = true;
          seeAllTimer = 1000;
        } else if (selectedItem.itemResult == "What is going on?") {
          confused = true;
          confusionTimer = 1000;
        }
        
        for (int i = 0; i < inventorySize; i++) {
          if (inventoryPages[currentInventoryPageIndex].items[i].item == selectedItem.item) {
            updatePotionName(inventoryPages[currentInventoryPageIndex].items[i]);
          }
        }
      } else if (selectedItem.category == FoodCategory) {
        playRawSFX(5);
        playerFood += selectedItem.hungerRecoverAmount;
        playerFood = playerFood > 100 ? 100 : playerFood;
      } else {
        playRawSFX(2);
      }

      itemResultMessage = selectedItem.itemResult;

      if (selectedItem.item == RiddleStone) {
        currentUIState = UI_RIDDLE; // Riddlesssss
      } else {
        currentUIState = UI_ITEM_RESULT; // Change to result screen
      }

      if (inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].oneTimeUse) {
        if (inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].category == PotionCategory) {
          inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = getItem(EmptyBottle);
        } else {
          inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = { Null, PotionCategory, "Empty"};
          inventoryPages[currentInventoryPageIndex].itemCount--;
        }
      }

      buttons.bPressedPrev = true;
    } else if (selectedActionIndex == 1) { // Drop
      inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = { Null, PotionCategory, "Empty"};
      inventoryPages[currentInventoryPageIndex].itemCount--;
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

    int yPos = 30;
    InventoryPage &currentPage = inventoryPages[currentInventoryPageIndex];
    // Draw items in the current tab
    for (int i = 0; i < 8; i++) {
      GameItem &item = currentPage.items[i];
      if (item.item == Null) continue;
      
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
    display.println(inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].originalName);
    display.setCursor(0, 10);
    display.print(inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].description);
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
