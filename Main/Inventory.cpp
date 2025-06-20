#include "Inventory.h"
#include "Player.h"
#include "GameAudio.h"

int selectedInventoryIndex = 0; // Currently selected inventory item
String itemResultMessage = "";

InventoryPage inventoryPages[] = {
  {"Potions", PotionCategory},
  {"Food", FoodCategory},
  {"Equipment", EquipmentCategory},
  {"Scrolls", ScrollsCategory}
};
int currentInventoryPageIndex = 0;
int numInventoryPages = sizeof(inventoryPages)/sizeof(inventoryPages[0]);

// --- Identify Scroll Mechanic ---
bool identifyingItem = false;
int identifyScrollPage = -1;
int identifyScrollIndex = -1;

// Helper function to reveal true name and curse status
void identifyItem(GameItem &item) {
  if (item.category == PotionCategory) {
    updatePotionName(item);
  } else if (item.item == Scroll) {
    updateScrollName(item);
  }
  //item.name = item.originalName;
  // If the description already contains (Cursed), don't append again
  if (item.isCursed && item.description.indexOf("(Cursed)") == -1) {
    item.description += " (Cursed)";
  }
}

bool addToInventory(GameItem item, bool canBeCursed) {
  // Chance to curse the item if cursable
  if (canBeCursed && random(0, 11) < item.curseChance) {
    item.isCursed = true;
  }
  
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
  if (buttons.bPressed && !buttons.bPressedPrev && identifyingItem && currentUIState == UI_INVENTORY) {
    // Select item to identify
    GameItem &selectedItem = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];
    if (selectedItem.name != "Empty" && selectedItem.name != "") {
      identifyItem(selectedItem);
      itemResultMessage = "Identified: " + selectedItem.name + (selectedItem.isCursed ? ". It is cursed!" : ". Not cursed.");
      // Remove the scroll
      if (identifyScrollPage >= 0 && identifyScrollIndex >= 0) {
        inventoryPages[identifyScrollPage].items[identifyScrollIndex] = { Null, PotionCategory, "Empty"};
        inventoryPages[identifyScrollPage].itemCount--;
      }
      identifyingItem = false;
      identifyScrollPage = -1;
      identifyScrollIndex = -1;
      currentUIState = UI_ITEM_RESULT;
      playRawSFX(2);
    }
    return;
  }
  if (buttons.bPressed && !buttons.bPressedPrev && !identifyingItem && currentUIState == UI_INVENTORY) {
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

  selectedActionIndex = selectedActionIndex == 5 ? 0 : selectedActionIndex == -1 ? 4 : selectedActionIndex;

  // Cancel with A
  if (buttons.aPressed && !buttons.aPressedPrev) {
    currentUIState = UI_INVENTORY;
  }

  // Confirm with B
  if (buttons.bPressed && !buttons.bPressedPrev) {
    playRawSFX(7);
    GameItem &selectedItem = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];
    
    if (selectedActionIndex == 0) { // Use/Read/Drink
      if (selectedItem.category == ScrollsCategory) {
        // Handle scroll reading - scrolls are destroyed after first use
        if (!selectedItem.isScrollRevealed) {
          // First time reading - reveal name and apply effect
          if (selectedItem.effectType == ScrollProtectionEffect && equippedArmor.item == Null) {
            // Don't update the name as there is no armor equipped
          } else {
            updateScrollName(selectedItem);
          }
        }

        playRawSFX(2);
        itemResultMessage = selectedItem.itemResult;
        
        // Apply scroll effects based on type
        if (selectedItem.effectType == ScrollProtectionEffect) {
          if (equippedArmor.item != Null) {
            equippedArmor.armorValue += 1;
            equippedArmorValue += 1; // Increase armor protection
          } else {
            itemResultMessage = "The scroll disappears.";
          }
        } else if (selectedItem.effectType == ScrollIdentifyEffect) {
          // Start identify flow
          identifyingItem = true;
          identifyScrollPage = currentInventoryPageIndex;
          identifyScrollIndex = selectedInventoryIndex;
          currentUIState = UI_INVENTORY;
          return;
        } else if (selectedItem.effectType == ScrollEnchantEffect) {
          // Enchant scroll: increase player attack damage
          playerAttackDamage += 2; // Increase by 2, adjust as desired
          itemResultMessage = "You feel more powerful! Your attacks do more damage.";
          // Destroy the enchant scroll
          inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = { Null, PotionCategory, "Empty"};
          inventoryPages[currentInventoryPageIndex].itemCount--;
          currentUIState = UI_ITEM_RESULT;
        }
        
        // Destroy the scroll after reading (unless it's identify, which is handled after identification)
        if (selectedItem.effectType != ScrollIdentifyEffect) {
          inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = { Null, PotionCategory, "Empty"};
          inventoryPages[currentInventoryPageIndex].itemCount--;
        }
        if (selectedItem.effectType != ScrollIdentifyEffect) {
          currentUIState = UI_ITEM_RESULT;
        }
        buttons.bPressedPrev = true;
      } else if (selectedItem.category == PotionCategory && selectedItem.item != EmptyBottle) {
        // Handle potion drinking
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

        itemResultMessage = selectedItem.itemResult;

        if (selectedItem.effectType == ArmorEffect) {
          itemResultMessage = "You can't use this, Try equipping it.";
        }

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
      } else if (selectedItem.category == FoodCategory) {
        playRawSFX(5);
        playerFood += selectedItem.hungerRecoverAmount;
        playerFood = playerFood > 100 ? 100 : playerFood;
        
        itemResultMessage = selectedItem.itemResult;
        currentUIState = UI_ITEM_RESULT;
        
        if (inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].oneTimeUse) {
          inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = { Null, PotionCategory, "Empty"};
          inventoryPages[currentInventoryPageIndex].itemCount--;
        }
        
        buttons.bPressedPrev = true;
      } else if (selectedItem.effectType == ArmorEffect) {
        itemResultMessage = "You can't use this, try equipping it.";
        currentUIState = UI_ITEM_RESULT;
        buttons.bPressedPrev = true;
      } else {
        playRawSFX(2);
        itemResultMessage = selectedItem.itemResult;
        currentUIState = UI_ITEM_RESULT;
        buttons.bPressedPrev = true;
      }
    } else if (selectedActionIndex == 1) { // Drop
      // Prevent dropping equipped items
      if (selectedItem.isEquipped) {
        playRawSFX(13);
        itemResultMessage = "You need to unequip it first.";
        currentUIState = UI_ITEM_RESULT;
        buttons.bPressedPrev = true;
      } else {
        inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = { Null, PotionCategory, "Empty"};
        inventoryPages[currentInventoryPageIndex].itemCount--;
        currentUIState = UI_INVENTORY;
      }
    } else if (selectedActionIndex == 2) { // Info
      currentUIState = UI_ITEM_INFO;
    } else if (selectedActionIndex == 3) { // Equip/Unequip
      if (selectedItem.category == EquipmentCategory) {
        if (selectedItem.isEquipped) {
          // Unequip the item
          if (selectedItem.isCursed) {
            itemResultMessage = "You can't. It appears to be cursed.";
            currentUIState = UI_ITEM_RESULT;
          } else {
            selectedItem.isEquipped = false;
            if (selectedItem.effectType == ArmorEffect) {
              equippedArmorValue = 0;
              equippedArmor = {};
            }
            if (selectedItem.item == RiddleStone) {
              equippedRiddleStone = false;
            }
            
            playRawSFX(2);
            currentUIState = UI_INVENTORY;
          }
        } else {
          // Check if trying to equip armor when armor is already equipped
          if (selectedItem.effectType == ArmorEffect && equippedArmor.item != Null) {
            playRawSFX(13);
            itemResultMessage = "You need to unequip the armor first.";
            currentUIState = UI_ITEM_RESULT;
            buttons.bPressedPrev = true;
          } else {
            // Equip the item
            selectedItem.isEquipped = true;
            if (selectedItem.effectType == ArmorEffect) {
                equippedArmorValue = selectedItem.armorValue;
                equippedArmor = selectedItem;
            }
            
            playRawSFX(2);
            itemResultMessage = selectedItem.itemResult == "Solve this riddle!" ? "You equip the riddle stone." : selectedItem.itemResult; // override riddle stone text
            equippedRiddleStone = selectedItem.itemResult == "Solve this riddle!" ? true : equippedRiddleStone;
            currentUIState = UI_ITEM_RESULT;
          }
        }
      } else {
        playRawSFX(2);
        itemResultMessage = "This item cannot be equipped.";
        currentUIState = UI_ITEM_RESULT;
      }
      buttons.bPressedPrev = true;
    } else if (selectedActionIndex == 4) { // Combine
      combiningTwoItems = true;
      combiningItem1 = selectedItem;
      ingredient1index = selectedInventoryIndex;
      currentUIState = UI_INVENTORY;
    }
  }
}

bool showTooltip = true;
void renderInventory() {
  display.clearDisplay();
  display.setTextSize(1);

  if (currentUIState == UI_INVENTORY) {
    display.setCursor(combiningTwoItems ? 0 : 10, combiningTwoItems ? 0 : 3);
    display.setTextSize(combiningTwoItems || identifyingItem ? 1 : 2);
    display.println(combiningTwoItems ? "Select second item tocombine..." : identifyingItem ? "Select item to identify..." : "Inventory");
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
      display.print(item.name);
      
      // Add equipped indicator
      if (item.isEquipped) {
        display.setCursor(110, yPos);
        display.print("*");
      }
      
      yPos += 12;
    }

    if (yPos == 30) { // No items
      display.setCursor(15, 30);
      display.println("Empty");
    }
    
    if (showTooltip) {
      // Add legend for equipped items
      display.setCursor(0, 120);
      display.print("* = Equipped");
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
    showTooltip = false;
  } 
  else if (currentUIState == UI_ITEM_ACTION) {    
    // Draw options menu box
    display.drawRect(50, 40, 65, 75, 15);
    display.fillRect(50, 40, 65, 12, 15);

    // Title
    display.setTextColor(0);
    display.setCursor(55, 42);
    display.println("Options:");
    display.setTextColor(15);

    // Get the selected item to check if it's equipped
    GameItem &selectedItem = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];
    String equipText = (selectedItem.isEquipped && selectedItem.category == EquipmentCategory) ? "Unequip" : "Equip";
    
    // Determine the use text based on item type
    String useText = "Use";
    if (selectedItem.item == Scroll) {
      useText = "Read";
    } else if (selectedItem.category == PotionCategory && selectedItem.item != EmptyBottle) {
      useText = "Drink";
    } else if (selectedItem.category == FoodCategory && selectedItem.item != EmptyBottle) {
      useText = "Eat";
    }

    // Options
    display.setCursor(55, 60);
    display.println(selectedActionIndex == 0 ? "> " + useText : " " + useText);
    display.setCursor(55, 70);
    display.println(selectedActionIndex == 1 ? "> Drop" : " Drop");
    display.setCursor(55, 80);
    display.println(selectedActionIndex == 2 ? "> Info" : " Info");
    display.setCursor(55, 90);
    display.println(selectedActionIndex == 3 ? "> " + equipText : " " + equipText);
    display.setCursor(55, 100);
    display.println(selectedActionIndex == 4 ? "> Combine" : " Combine");
  }

  display.display();
}
