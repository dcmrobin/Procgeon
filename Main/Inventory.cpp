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
  } else if (item.item == Ring) {
    updateRingName(item);
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

        if (resultItem.name != "Null") {
          bool ingredient1IsPotion = combiningItem1.category == PotionCategory;
          bool ingredient2IsPotion = combiningItem2.category == PotionCategory;

          // Find which page each ingredient is on
          int ingredient1Page = -1;
          int ingredient1Index = -1;
          int ingredient2Page = -1;
          int ingredient2Index = -1;

          // Search for ingredient1 across all pages
          for (int p = 0; p < numInventoryPages; p++) {
            for (int i = 0; i < inventorySize; i++) {
              if (inventoryPages[p].items[i].item == combiningItem1.item && 
                  inventoryPages[p].items[i].name == combiningItem1.name) {
                ingredient1Page = p;
                ingredient1Index = i;
                break;
              }
            }
            if (ingredient1Page != -1) break;
          }

          // Search for ingredient2 across all pages
          for (int p = 0; p < numInventoryPages; p++) {
            for (int i = 0; i < inventorySize; i++) {
              if (inventoryPages[p].items[i].item == combiningItem2.item && 
                  inventoryPages[p].items[i].name == combiningItem2.name) {
                ingredient2Page = p;
                ingredient2Index = i;
                break;
              }
            }
            if (ingredient2Page != -1) break;
          }

          // Remove both ingredients from their respective pages
          if (ingredient1Page != -1 && ingredient1Index != -1) {
            removeItemFromInventory(ingredient1Page, ingredient1Index);
            
            // If both ingredients are on the same page and ingredient2 comes after ingredient1,
            // adjust ingredient2's index since items shifted left
            if (ingredient2Page == ingredient1Page && ingredient2Index > ingredient1Index) {
                ingredient2Index--;
            }
          }
          if (ingredient2Page != -1 && ingredient2Index != -1) {
            removeItemFromInventory(ingredient2Page, ingredient2Index);
          }

          // Add result item to correct page
          addToInventory(resultItem, false);

          // Add empty bottle if needed
          if (ingredient1IsPotion || ingredient2IsPotion) {
            addToInventory(getItem(EmptyBottle), false);
          }
          selectedInventoryIndex = 0;
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
        if (blinded) {
          itemResultMessage = "You can't read while blind!";
          currentUIState = UI_ITEM_RESULT;
          buttons.bPressedPrev = true;
          return;
        }
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
        if (selectedItem.effectType != ScrollIdentifyEffect && inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].oneTimeUse) {
          inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = { Null, PotionCategory, "Empty"};
          inventoryPages[currentInventoryPageIndex].itemCount--;
        }
        if (selectedItem.effectType != ScrollIdentifyEffect) {
          currentUIState = UI_ITEM_RESULT;
        }
        buttons.bPressedPrev = true;
      } else if (selectedItem.category == PotionCategory && selectedItem.item != EmptyBottle) {
        if (selectedItem.itemResult == "A lot happens.") { // random effect applied to the potion before anything else so that all the effects can be applied after
          selectedItem.healthRecoverAmount = random(-90, 101); // -90 to +100
          selectedItem.hungerRecoverAmount = random(-90, 101); // -90 to +100
          selectedItem.AOEsize = random(0, 11); // 0 to 10
          selectedItem.AOEdamage = random(-10, 21); // -10 to +20
          selectedItem.SpeedMultiplier = (random(-20, 21)) / 10.0; // -2.0 to +2.0
        }

        // Handle potion drinking
        playRawSFX(6);
        playerHP += selectedItem.healthRecoverAmount;
        playerHP = playerHP > playerMaxHP ? playerMaxHP : playerHP;

        if (speeding) {
          speedTimer += 500;
        }

        if (selectedItem.SpeedMultiplier != 0) {
          speeding = true;
          lastPotionSpeedModifier = selectedItem.SpeedMultiplier;
          currentSpeedMultiplier += selectedItem.SpeedMultiplier;
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
        } else if (selectedItem.itemResult == "You feel fabulous!") {
          if (ridiculed) {
            ridiculed = false;
            ridiculeTimer = 0;
          }
          glamoured = true;
          glamourTimer = 1000;
        } else if (selectedItem.itemResult == "You feel stupid.") {
          if (glamoured) {
            glamoured = false;
            glamourTimer = 0;
          }
          ridiculed = true;
          ridiculeTimer = RIDICULE_DURATION;
        } else if (selectedItem.itemResult == "A cloak of darkness falls around you.") {
          blinded = true;
          blindnessTimer = 700;
        } else if (selectedItem.itemResult == "You feel stronger.") {
          playerAttackDamage += 3;
        } else if (selectedItem.itemResult == "You feel restored!") {
          //cures blindness, confusion, ridicule, and glamoured, and restores speed, and raises player attack damage, and restores player hp and food
          blinded = false;
          blindnessTimer = 0;
          confused = false;
          confusionTimer = 0;
          if (ridiculed) {
            ridiculed = false;
            ridiculeTimer = 0;
            showDialogue = false;
          }
          paralyzed = false;
          paralysisTimer = 0;
          if (currentSpeedMultiplier < 1) {
            currentSpeedMultiplier = 0;
            speedTimer = 0;
            speeding = false;
            lastPotionSpeedModifier = 0;
          }
          playerAttackDamage += 3;
          playerHP += 15;
          playerHP = playerHP > playerMaxHP ? playerMaxHP : playerHP;
          playerFood += 15;
          playerFood = playerFood > 100 ? 100 : playerFood;
        } else if (selectedItem.itemResult == "You can't move!") {
          paralyzed = true;
          paralysisTimer = 1000;
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
      } else if (selectedItem.effectType == ArmorEffect || selectedItem.item == Ring) {
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
            // --- Remove ring effects when unequipped ---
            if (selectedItem.item == Ring) {
              int idx = selectedItem.ringEffectIndex;
              if (ringEffects[idx] == "Ring of Swiftness") {
                ringOfSwiftnessActive = false;
                currentSpeedMultiplier -= 0.5;
              } else if (ringEffects[idx] == "Ring of Strength") {
                ringOfStrengthActive = false;
                playerAttackDamage -= 5;
              } else if (ringEffects[idx] == "Ring of Weakness") {
                ringOfWeaknessActive = false;
                playerAttackDamage += 5;
              } else if (ringEffects[idx] == "Ring of Hunger") {
                ringOfHungerActive = false;
              } else if (ringEffects[idx] == "Ring of Regeneration") {
                ringOfRegenActive = false;
              }
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
            // --- Apply ring effects when equipped ---
            if (selectedItem.item == Ring) {
              int idx = selectedItem.ringEffectIndex;
              if (ringEffects[idx] == "Ring of Swiftness") {
                ringOfSwiftnessActive = true;
                currentSpeedMultiplier += 0.5;
              } else if (ringEffects[idx] == "Ring of Strength") {
                ringOfStrengthActive = true;
                playerAttackDamage += 5;
              } else if (ringEffects[idx] == "Ring of Weakness") {
                ringOfWeaknessActive = true;
                playerAttackDamage -= 5;
              } else if (ringEffects[idx] == "Ring of Hunger") {
                ringOfHungerActive = true;
              } else if (ringEffects[idx] == "Ring of Regeneration") {
                ringOfRegenActive = true;
              }
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
      // Highlight combinable items when combiningTwoItems is true
      if (combiningTwoItems) {
        GameItem result = CombineTwoItemsToGetItem(combiningItem1, item);
        if (result.item != Null) {
          display.print(">>"); // Marker for combinable
        } else {
          display.print("");
        }
      }
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

// Helper function to remove an item from inventory and shift others left
void removeItemFromInventory(int page, int index) {
    InventoryPage &invPage = inventoryPages[page];
    for (int i = index; i < inventorySize - 1; i++) {
        invPage.items[i] = invPage.items[i + 1];
    }
    invPage.items[inventorySize - 1] = getItem(Null);
    invPage.itemCount--;
}
