#include "Inventory.h"
#include "Player.h"
#include "GameAudio.h"
#include <string.h>
#include "Translation.h"

int selectedInventoryIndex = 0; // Currently selected inventory item
char itemResultMessage[150] = "";

InventoryPage inventoryPages[] = {
  {{"Potions"}, PotionCategory},
  {{"Food"}, FoodCategory},
  {{"Equipment"}, EquipmentCategory},
  {{"Scrolls"}, ScrollsCategory}
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
  if (item.isCursed && strstr(item.description, "(Cursed)") == NULL) {// what the heck does strstr do
    char temp[110];
    snprintf(temp, sizeof(temp), "%s (Cursed)", item.description);
    snprintf(item.description, sizeof(item.description), "%s", temp);
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
    if (strcmp(selectedItem.name, "Empty") != 0 && strlen(selectedItem.name) > 0) {
      identifyItem(selectedItem);
      snprintf(itemResultMessage, sizeof(itemResultMessage), "Identified: %s%s", selectedItem.name, selectedItem.isCursed ? ". It is cursed!" : ". Not cursed.");
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
  
    if (strcmp(selectedItem.name, "Empty") != 0 && strlen(selectedItem.name) > 0) {
      playRawSFX(7);
      if (!combiningTwoItems) {
        currentUIState = UI_ITEM_ACTION;
        selectedActionIndex = 0;
      } else {
        combiningItem2 = selectedItem;
        GameItem resultItem = CombineTwoItemsToGetItem(combiningItem1, combiningItem2);

        if (strcmp(resultItem.name, "Null") != 0) {
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
                  strcmp(inventoryPages[p].items[i].name, combiningItem1.name) == 0) {
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
                  strcmp(inventoryPages[p].items[i].name, combiningItem2.name) == 0) {
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
        if (resultItem.category != EquipmentCategory) {
          if (strcmp(resultItem.name, "Null") == 0) {
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "These two items cannot be combined.");
          } else {
            snprintf(itemResultMessage, sizeof(itemResultMessage), "Combined two items! The result was: %s", resultItem.name);
          }
        } else {
          snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", resultItem.itemResult);
        }
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
          snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You can't read while blind!");
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
        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", selectedItem.itemResult);
        
        // Apply scroll effects based on type
        if (selectedItem.effectType == ScrollProtectionEffect) {
          if (equippedArmor.item != Null) {
            if (equippedArmor.armorValue == 0) {
              snprintf(equippedArmor.description, sizeof(equippedArmor.description), "%s", "Restored armor.");
            }
            equippedArmor.armorValue += 1;
            equippedArmorValue += 1; // Increase armor protection
            if (seeAll) {
              equippedArmor.armorValue += 9;
              equippedArmorValue += 9;
              snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You read in between the lines. Your armor is covered by a very bright shimmering gold shield!");
            }
          } else {
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "The scroll disappears.");
          }
        } else if (selectedItem.effectType == ScrollIdentifyEffect) {
          if (!seeAll) {
            // Start identify flow
            identifyingItem = true;
            identifyScrollPage = currentInventoryPageIndex;
            identifyScrollIndex = selectedInventoryIndex;
            currentUIState = UI_INVENTORY;
            return;
          } else {
            for (int x = 0; x < numInventoryPages; x++) {
              for (int y = 0; y < 8; y++) {
                identifyItem(inventoryPages[x].items[y]);
              }
            }
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You read in between the lines. Your entire inventory is revealed!");
          }
        } else if (selectedItem.effectType == ScrollEnchantEffect) {
          // Enchant scroll: increase player attack damage
          playerAttackDamage += 2; // Increase by 2, adjust as desired
          snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You feel more powerful! Your attacks do more damage.");
          if (seeAll) {
            playerAttackDamage += 8;
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You read in between the lines. Your attacks do much more damage!");
          }
          // Destroy the enchant scroll
          inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = { Null, PotionCategory, "Empty"};
          inventoryPages[currentInventoryPageIndex].itemCount--;
          currentUIState = UI_ITEM_RESULT;
        } else if (selectedItem.effectType == ScrollUncurseEffect) {
            // Uncurse all equipped items
            for (int p = 0; p < numInventoryPages; p++) {
                for (int i = 0; i < inventorySize; i++) {
                    GameItem &item = inventoryPages[p].items[i];
                    if ((item.isEquipped || seeAll) && item.isCursed) {
                        item.isCursed = false;
                        char *cursedPos = strstr(item.description, " (Cursed)");
                        if (cursedPos != NULL) {
                            // Remove " (Cursed)" by shifting the remaining string left
                            size_t removeLen = 9; // length of " (Cursed)"
                            size_t remainingLen = strlen(cursedPos + removeLen);
                            memmove(cursedPos, cursedPos + removeLen, remainingLen + 1); // +1 for null terminator
                        }
                    }
                }
            }
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You feel as if someone is watching over you.");
            if (seeAll) {
              snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You read in between the lines. Your entire inventory is uncursed!");
            }
            currentUIState = UI_ITEM_RESULT;
        } else if (selectedItem.effectType == ScrollEmptyEffect && seeAll) {
          addToInventory(getItem(KingArmor), false);
          addToInventory(getItem(RiddleStone), false);
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
          playerAttackDamage += 10;
          playerHP = playerMaxHP;
          playerFood = 100;
          snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You read the invisible text on the scroll. You feel restored! You have gained something!");
          currentUIState = UI_ITEM_RESULT;
        } else if (selectedItem.effectType == ScrollMapEffect) {
          hasMap = true;
        } else if (selectedItem.effectType == ScrollAmnesiaEffect) {
          resetPotionNames();
          for (int i = 0; i < NUM_SCROLLS; i++)
          {
            snprintf(scrollNamesRevealed[i], sizeof(scrollNamesRevealed[i]), "%s", scrollNames[i]);
          }

          for (int i = 0; i < 8; i++) {
            snprintf(inventoryPages[0].items[i].name, sizeof(inventoryPages[0].items[i].name), "%s", inventoryPages[0].items[i].originalName);
            snprintf(inventoryPages[3].items[i].name, sizeof(inventoryPages[3].items[i].name), "%s", inventoryPages[3].items[i].originalName);
            if (inventoryPages[2].items[i].item == Ring) {
              snprintf(inventoryPages[2].items[i].name, sizeof(inventoryPages[2].items[i].name), "%s", inventoryPages[2].items[i].originalName);
              snprintf(inventoryPages[2].items[i].description, sizeof(inventoryPages[2].items[i].description), "%s", "You don't remember what this does.");
            }
            snprintf(inventoryPages[0].items[i].description, sizeof(inventoryPages[0].items[i].description), "%s", "You don't remember what this does.");
            snprintf(inventoryPages[3].items[i].description, sizeof(inventoryPages[3].items[i].description), "%s", "You don't remember what this does.");
          }
        } else if (selectedItem.effectType == ScrollAggravateEffect) {
          ambientNoiseLevel = 100000;
        } else if (selectedItem.effectType == ScrollDestroyEffect) {
          equippedArmor.armorValue = 0;
          equippedArmorValue = 0;
        } else if (selectedItem.effectType == ScrollTeleportEffect) {
          playRawSFX(14);
          int newX, newY;
          do {
            newX = random(0, mapWidth);
            newY = random(0, mapHeight);
          } while (dungeonMap[newY][newX] != Floor);
          playerX = newX;
          playerY = newY;
        }
        
        // Destroy the scroll after reading (unless it's identify, which is handled after identification)
        if ((selectedItem.effectType != ScrollIdentifyEffect || seeAll) && inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].oneTimeUse) {
          inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = { Null, PotionCategory, "Empty"};
          inventoryPages[currentInventoryPageIndex].itemCount--;
        }
        if (selectedItem.effectType != ScrollIdentifyEffect || seeAll) {
          currentUIState = UI_ITEM_RESULT;
        }
        buttons.bPressedPrev = true;
      } else if ((selectedItem.category == PotionCategory || selectedItem.category == FoodCategory) && selectedItem.item != EmptyBottle) {
        if (strcmp(selectedItem.itemResult, "A lot happens.") == 0) { // random effect applied to the potion before anything else so that all the effects can be applied after
          selectedItem.healthRecoverAmount = random(-90, 101); // -90 to +100
          selectedItem.hungerRecoverAmount = random(-90, 101); // -90 to +100
          selectedItem.AOEsize = random(0, 11); // 0 to 10
          selectedItem.AOEdamage = random(-10, 21); // -10 to +20
          selectedItem.SpeedMultiplier = (random(-20, 21)) / 10.0; // -2.0 to +2.0
        }

        // Handle potion drinking
        playRawSFX(selectedItem.category == PotionCategory ? 6 : 5);
        playerFood += selectedItem.hungerRecoverAmount;
        playerFood = playerFood > 100 ? 100 : playerFood;
        playerHP += selectedItem.healthRecoverAmount;
        playerHP = playerHP > (playerMaxHP - (sicknessRingsNumber*20)) ? (playerMaxHP - (sicknessRingsNumber*20)) : playerHP;

        if (speeding) {
          speedTimer += 500;
        }

        if (selectedItem.SpeedMultiplier != 0) {
          speeding = true;
          lastPotionSpeedModifier = selectedItem.SpeedMultiplier;
          currentSpeedMultiplier += selectedItem.SpeedMultiplier;
        }

        checkIfDeadFrom("poison");
        
        if (selectedItem.AOEsize > 0) {
          applyAOEEffect(playerX, playerY, selectedItem.AOEsize, selectedItem.AOEdamage);
        }

        if (strcmp(selectedItem.itemResult, "You are now more hungry.") == 0) {
          playerFood -= 51;//heheheh u want it to be 50 don't u >:D
        } else if (strcmp(selectedItem.itemResult, "You can now see that which was unseen for a limited time.") == 0) {
          seeAll = true;
          seeAllTimer = 1000;
          blinded = false;
          blindnessTimer = 0;
        } else if (strcmp(selectedItem.itemResult, "What is going on?") == 0) {
          confused = true;
          confusionTimer = 1000;
        } else if (strcmp(selectedItem.itemResult, "You feel fabulous!") == 0) {
          if (ridiculed) {
            ridiculed = false;
            ridiculeTimer = 0;
          }
          glamoured = true;
          glamourTimer = 1000;
        } else if (strcmp(selectedItem.itemResult, "You feel stupid.") == 0) {
          if (glamoured) {
            glamoured = false;
            glamourTimer = 0;
          }
          ridiculed = true;
          ridiculeTimer = RIDICULE_DURATION;
        } else if (strcmp(selectedItem.itemResult, "A cloak of darkness falls around you.") == 0) {
          blinded = true;
          blindnessTimer = 700;
        } else if (strcmp(selectedItem.itemResult, "You feel stronger.") == 0) {
          playerAttackDamage += 3;
        } else if (strcmp(selectedItem.itemResult, "You feel restored!") == 0) {
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
          playerHP = playerHP > (playerMaxHP - (sicknessRingsNumber*20)) ? (playerMaxHP - (sicknessRingsNumber*20)) : playerHP;
          playerFood += 15;
          playerFood = playerFood > 100 ? 100 : playerFood;
        } else if (strcmp(selectedItem.itemResult, "You can't move!") == 0) {
          paralyzed = true;
          paralysisTimer = 1000;
        }
        
        for (int i = 0; i < inventorySize; i++) {
          if (inventoryPages[currentInventoryPageIndex].items[i].item == selectedItem.item) {
            updatePotionName(inventoryPages[currentInventoryPageIndex].items[i]);
          }
        }

        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", selectedItem.itemResult);

        if (selectedItem.effectType == ArmorEffect) {
          snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You can't use this, Try equipping it.");
        }

        currentUIState = UI_ITEM_RESULT; // Change to result screen

        if (inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].oneTimeUse) {
          if (inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].category == PotionCategory) {
            inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = getItem(EmptyBottle);
          } else {
            inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = { Null, PotionCategory, "Empty"};
            inventoryPages[currentInventoryPageIndex].itemCount--;
          }
        }

        buttons.bPressedPrev = true;
      } else if (selectedItem.effectType == ArmorEffect || selectedItem.item == Ring) {
        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You can't use this, try equipping it.");
        currentUIState = UI_ITEM_RESULT;
        buttons.bPressedPrev = true;
      } else if (selectedItem.item == RiddleStone) {
        currentUIState = UI_RIDDLE;
        inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex] = { Null, PotionCategory, "Empty"};
        inventoryPages[currentInventoryPageIndex].itemCount--;
      } else {
        playRawSFX(2);
        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", selectedItem.itemResult);
        currentUIState = UI_ITEM_RESULT;
        buttons.bPressedPrev = true;
      }
    } else if (selectedActionIndex == 1) { // Drop
      // Prevent dropping equipped items
      if (selectedItem.isEquipped) {
        playRawSFX(13);
        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You need to unequip it first.");
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
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You can't. It appears to be cursed.");
            currentUIState = UI_ITEM_RESULT;
          } else if (strcmp(selectedItem.originalName, "Washer") == 0) {
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You try to remove the washer from your finger, but it fails to go past your knuckle.");
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
              if (strcmp(ringEffects[idx], "Ring of Swiftness") == 0) {
                swiftnessRingsNumber -= 1;
              } else if (strcmp(ringEffects[idx], "Ring of Strength") == 0) {
                strengthRingsNumber -= 1;
              } else if (strcmp(ringEffects[idx], "Ring of Weakness") == 0) {
                weaknessRingsNumber -= 1;
              } else if (strcmp(ringEffects[idx], "Ring of Hunger") == 0) {
                hungerRingsNumber -= 1;
              } else if (strcmp(ringEffects[idx], "Ring of Regeneration") == 0) {
                regenRingsNumber -= 1;
              } else if (strcmp(ringEffects[idx], "Ring of Sickness") == 0) {
                sicknessRingsNumber -= 1;
              } else if (strcmp(ringEffects[idx], "Ring of Aggravation") == 0) {
                aggravateRingsNumber -= 1;
              } else if (strcmp(ringEffects[idx], "Ring of Armor") == 0) {
                armorRingsNumber -= 1;
              } else if (strcmp(ringEffects[idx], "Ring of Indigestion") == 0) {
                indigestionRingsNumber -= 1;
              } else if (strcmp(ringEffects[idx], "Ring of Teleport") == 0) {
                teleportRingsNumber -= 1;
              } else if (strcmp(ringEffects[idx], "Ring of Invisibility") == 0) {
                invisibleRingsNumber -= 1;
              }
            }
            playRawSFX(2);
            currentUIState = UI_INVENTORY;
          }
        } else {
          // Check if trying to equip armor when armor is already equipped
          if (selectedItem.effectType == ArmorEffect && equippedArmor.item != Null) {
            playRawSFX(13);
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You need to unequip the armor first.");
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
              // If this ring has not yet been assigned an effect, assign one now for this instance
              if (selectedItem.ringEffectIndex < 0 || selectedItem.ringEffectIndex >= NUM_RINGS) {
                int assignIdx = random(0, NUM_RINGS);
                selectedItem.ringEffectIndex = assignIdx;
                selectedItem.isCursed = ringCursed[assignIdx];
              }
              int idx = selectedItem.ringEffectIndex;
              if (strcmp(ringEffects[idx], "Ring of Swiftness") == 0) {
                swiftnessRingsNumber += 1;
              } else if (strcmp(ringEffects[idx], "Ring of Strength") == 0) {
                strengthRingsNumber += 1;
              } else if (strcmp(ringEffects[idx], "Ring of Weakness") == 0) {
                weaknessRingsNumber += 1;
              } else if (strcmp(ringEffects[idx], "Ring of Hunger") == 0) {
                hungerRingsNumber += 1;
              } else if (strcmp(ringEffects[idx], "Ring of Regeneration") == 0) {
                regenRingsNumber += 1;
              } else if (strcmp(ringEffects[idx], "Ring of Sickness") == 0) {
                sicknessRingsNumber += 1;
                playerHP = playerMaxHP - (sicknessRingsNumber*20);
              } else if (strcmp(ringEffects[idx], "Ring of Aggravation") == 0) {
                aggravateRingsNumber += 1;
              } else if (strcmp(ringEffects[idx], "Ring of Armor") == 0) {
                armorRingsNumber += 1;
              } else if (strcmp(ringEffects[idx], "Ring of Indigestion") == 0) {
                indigestionRingsNumber += 1;
              } else if (strcmp(ringEffects[idx], "Ring of Teleport") == 0) {
                teleportRingsNumber += 1;
              } else if (strcmp(ringEffects[idx], "Ring of Invisibility") == 0) {
                invisibleRingsNumber += 1;
              }
            }
            playRawSFX(2);
            if (strcmp(selectedItem.itemResult, "Solve this riddle!") == 0) {
              snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You equip the riddle stone.");
              equippedRiddleStone = true;
            } else {
              snprintf(itemResultMessage, sizeof(itemResultMessage), "You equip the %s.", selectedItem.name);
            }
            currentUIState = UI_ITEM_RESULT;
          }
        }
      } else {
        playRawSFX(2);
        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "This item cannot be equipped.");
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
    char pageName[40];
    snprintf(pageName, sizeof(pageName), "<%s>", inventoryPages[currentInventoryPageIndex].name);
    display.println(pageName);
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
      const char* displayName = item.name;
      if (combiningTwoItems) {
        GameItem result = CombineTwoItemsToGetItem(combiningItem1, item);
        if (result.item != Null) {
          if (i == selectedInventoryIndex) {
            displayName = result.name; // Show result name only when selected
            display.print(">>"); // Marker for combinable
          }
        } else {
          display.print("");
        }
      }
      display.print(displayName);
      
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
    const char* equipText = (selectedItem.isEquipped && selectedItem.category == EquipmentCategory) ? "Unequip" : "Equip";
    
    // Determine the use text based on item type
    const char* useText = "Use";
    if (selectedItem.item == Scroll) {
      useText = "Read";
    } else if (selectedItem.category == PotionCategory && selectedItem.item != EmptyBottle) {
      useText = "Drink";
    } else if (selectedItem.category == FoodCategory && selectedItem.item != EmptyBottle) {
      useText = "Eat";
    }

    // Options
    char useLine[20], equipLine[20];
    snprintf(useLine, sizeof(useLine), "%s %s", (selectedActionIndex == 0 ? ">" : ""), useText);
    snprintf(equipLine, sizeof(equipLine), "%s %s", (selectedActionIndex == 3 ? ">" : ""), equipText);
    display.setCursor(55, 60);
    display.println(useLine);
    display.setCursor(55, 70);
    display.println(selectedActionIndex == 1 ? "> Drop" : " Drop");
    display.setCursor(55, 80);
    display.println(selectedActionIndex == 2 ? "> Info" : " Info");
    display.setCursor(55, 90);
    display.println(equipLine);
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