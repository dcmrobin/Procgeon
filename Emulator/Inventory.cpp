#include "Inventory.h"
#include "Player.h"
#include "GameAudio.h"
#include <string.h>
#include "Translation.h"

int selectedInventoryIndex = 0;
char itemResultMessage[150] = "";

InventoryPage inventoryPages[] = {
  {{"Potions"}, PotionCategory},
  {{"Food"}, FoodCategory},
  {{"Equipment"}, EquipmentCategory},
  {{"Scrolls"}, ScrollsCategory},
  {{"Weapons"}, WeaponCategory},
};
int currentInventoryPageIndex = 0;
int numInventoryPages = sizeof(inventoryPages)/sizeof(inventoryPages[0]);

bool identifyingItem = false;
int identifyScrollPage = -1;
int identifyScrollIndex = -1;

// ─────────────────────────────────────────────────────────────────────────────
// identifyItem — unchanged
// ─────────────────────────────────────────────────────────────────────────────
void identifyItem(GameItem &item) {
  if (item.category == PotionCategory) {
    updatePotionName(item);
  } else if (item.item == Scroll) {
    updateScrollName(item);
  } else if (item.item == Ring) {
    updateRingName(item);
  }
  if (item.isCursed && strstr(item.description, "(Cursed)") == NULL) {
    char temp[110];
    snprintf(temp, sizeof(temp), "%s (Cursed)", item.description);
    snprintf(item.description, sizeof(item.description), "%s", temp);
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// addToInventory — stacking-aware
//
// Algorithm:
//   1. If the incoming item is stackable, scan the matching tab for an existing
//      stack of the same type/name with room left (stackCount < MAX_STACK_SIZE).
//      If found, just increment that stack's count and return true.
//   2. Otherwise, find the first Null slot and place it there (stackCount = 1).
// ─────────────────────────────────────────────────────────────────────────────
bool addToInventory(GameItem item, bool canBeCursed) {
  // Chance to curse
  if (canBeCursed && random(0, 11) < item.curseChance) {
    item.isCursed = true;
  }

  // Resolve generic weapon into a specific weapon type
  if (item.category == WeaponCategory && strcmp(item.name, "Weapon") == 0) {
    item.weapon = weaponList[random(0, 4)];
    snprintf(item.description, sizeof(item.description), "%s", weaponList[item.weapon.type].description);
    snprintf(item.name, sizeof(item.name), "%s", weaponList[item.weapon.type].name);
    snprintf(item.originalName, sizeof(item.originalName), "%s", weaponList[item.weapon.type].name);
    item.canRust = weaponList[item.weapon.type].canRust;
  }

  // Ensure stackCount is at least 1 for a valid item
  if (item.item != Null && item.stackCount < 1) item.stackCount = 1;

  for (int p = 0; p < numInventoryPages; p++) {
    if (inventoryPages[p].category != item.category) continue;

    // ── Pass 1: try to merge into an existing stack ──────────────────────────
    if (isStackable(item)) {
      for (int i = 0; i < inventorySize; i++) {
        GameItem &slot = inventoryPages[p].items[i];
        if (slot.item == Null) continue;
        if (canStackWith(slot, item) && slot.stackCount < MAX_STACK_SIZE) {
          slot.stackCount++;
          // itemCount doesn't change — slot was already occupied
          return true;
        }
      }
    }

    // ── Pass 2: place into first empty slot ──────────────────────────────────
    if (inventoryPages[p].itemCount >= inventorySize) return false; // tab full

    for (int i = 0; i < inventorySize; i++) {
      if (inventoryPages[p].items[i].item == Null) {
        item.stackCount = 1; // always start a new slot at 1
        inventoryPages[p].items[i] = item;
        inventoryPages[p].itemCount++;
        return true;
      }
    }
  }
  return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Navigation — unchanged
// ─────────────────────────────────────────────────────────────────────────────
void handleInventoryNavigation() {
  if (currentUIState != UI_INVENTORY) return;

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
  for (int i = 0; i < inventorySize; i++) {
    if (currentPage.items[i].item != Null) return i;
  }
  return 0;
}

int findNextItemInCategory(int current) {
  int category = inventoryPages[currentInventoryPageIndex].category;
  for (int i = current + 1; i < inventorySize; i++) {
    if (inventoryPages[currentInventoryPageIndex].items[i].item != Null &&
        inventoryPages[currentInventoryPageIndex].items[i].category == category)
      return i;
  }
  for (int i = 0; i <= current; i++) {
    if (inventoryPages[currentInventoryPageIndex].items[i].item != Null &&
        inventoryPages[currentInventoryPageIndex].items[i].category == category)
      return i;
  }
  return -1;
}

int findPreviousItemInCategory(int current) {
  int category = inventoryPages[currentInventoryPageIndex].category;
  for (int i = current - 1; i >= 0; i--) {
    if (inventoryPages[currentInventoryPageIndex].items[i].item != Null &&
        inventoryPages[currentInventoryPageIndex].items[i].category == category)
      return i;
  }
  for (int i = inventorySize - 1; i >= current; i--) {
    if (inventoryPages[currentInventoryPageIndex].items[i].item != Null &&
        inventoryPages[currentInventoryPageIndex].items[i].category == category)
      return i;
  }
  return -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// consumeOneFromStack — helper used by use/drop logic.
// Decrements the stack. If the stack empties, clears the slot.
// Returns true if the slot was fully emptied.
// ─────────────────────────────────────────────────────────────────────────────
static bool consumeOneFromStack(int page, int index) {
  GameItem &slot = inventoryPages[page].items[index];
  if (slot.stackCount > 1) {
    slot.stackCount--;
    return false; // slot still has items
  }
  // Last item in the stack — clear the slot
  if (slot.category == PotionCategory) {
    // Potions leave an empty bottle (handled by caller where needed)
  }
  slot = { Null, PotionCategory, "Empty" };
  slot.stackCount = 0;
  inventoryPages[page].itemCount--;
  return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// handleInventoryItemUsage
// ─────────────────────────────────────────────────────────────────────────────
void handleInventoryItemUsage() {
  if (buttons.bPressed && !buttons.bPressedPrev && identifyingItem && currentUIState == UI_INVENTORY) {
    GameItem &selectedItem = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];
    if (strcmp(selectedItem.name, "Empty") != 0 && strlen(selectedItem.name) > 0) {
      identifyItem(selectedItem);
      snprintf(itemResultMessage, sizeof(itemResultMessage), "Identified: %s%s",
               selectedItem.name, selectedItem.isCursed ? ". It is cursed!" : ". Not cursed.");
      if (identifyScrollPage >= 0 && identifyScrollIndex >= 0) {
        consumeOneFromStack(identifyScrollPage, identifyScrollIndex);
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

          int ingredient1Page = -1, ingredient1Index = -1;
          int ingredient2Page = -1, ingredient2Index = -1;

          for (int p = 0; p < numInventoryPages; p++) {
            for (int i = 0; i < inventorySize; i++) {
              if (inventoryPages[p].items[i].item == combiningItem1.item &&
                  strcmp(inventoryPages[p].items[i].name, combiningItem1.name) == 0) {
                ingredient1Page = p; ingredient1Index = i; break;
              }
            }
            if (ingredient1Page != -1) break;
          }
          for (int p = 0; p < numInventoryPages; p++) {
            for (int i = 0; i < inventorySize; i++) {
              if (inventoryPages[p].items[i].item == combiningItem2.item &&
                  strcmp(inventoryPages[p].items[i].name, combiningItem2.name) == 0) {
                ingredient2Page = p; ingredient2Index = i; break;
              }
            }
            if (ingredient2Page != -1) break;
          }

          // Consume one from each ingredient stack
          if (ingredient1Page != -1 && ingredient1Index != -1) {
            consumeOneFromStack(ingredient1Page, ingredient1Index);
            if (ingredient2Page == ingredient1Page && ingredient2Index > ingredient1Index) {
              // If the slot was cleared, the indices above it didn't shift
              // (we don't compact, just set to Null), so no adjustment needed.
            }
          }
          if (ingredient2Page != -1 && ingredient2Index != -1) {
            consumeOneFromStack(ingredient2Page, ingredient2Index);
          }

          addToInventory(resultItem, false);

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
            snprintf(itemResultMessage, sizeof(itemResultMessage), "Combined! Result: %s", resultItem.name);
          }
        } else {
          snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", resultItem.itemResult);
        }
        combiningTwoItems = false;
      }
    }
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// handleItemActionMenu
// ─────────────────────────────────────────────────────────────────────────────
void handleItemActionMenu() {
  if (buttons.upPressed && !buttons.upPressedPrev) { playRawSFX(8); selectedActionIndex--; }
  if (buttons.downPressed && !buttons.downPressedPrev) { playRawSFX(8); selectedActionIndex++; }

  selectedActionIndex = selectedActionIndex == 5 ? 0 : selectedActionIndex == -1 ? 4 : selectedActionIndex;

  if (buttons.aPressed && !buttons.aPressedPrev) {
    currentUIState = UI_INVENTORY;
  }

  if (buttons.bPressed && !buttons.bPressedPrev) {
    playRawSFX(7);
    GameItem &selectedItem = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];

    if (selectedActionIndex == 0) { // Use / Drink / Read / Eat
      if (selectedItem.category == ScrollsCategory) {
        if (blinded) {
          snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You can't read while blind!");
          currentUIState = UI_ITEM_RESULT;
          buttons.bPressedPrev = true;
          return;
        }
        if (!selectedItem.isScrollRevealed) {
          if (selectedItem.effectType == ScrollProtectionEffect && equippedArmor.item == Null) {
            // Don't reveal yet
          } else {
            updateScrollName(selectedItem);
          }
        }

        playRawSFX(2);
        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", selectedItem.itemResult);

        if (selectedItem.effectType == ScrollProtectionEffect) {
          if (equippedArmor.item != Null) {
            if (equippedArmor.armorValue == 0)
              snprintf(equippedArmor.description, sizeof(equippedArmor.description), "%s", "Restored armor.");
            equippedArmor.armorValue += 1;
            equippedArmorValue += 1;
            if (seeAll) {
              equippedArmor.armorValue += 9;
              equippedArmorValue += 9;
              snprintf(itemResultMessage, sizeof(itemResultMessage), "%s",
                "You read in between the lines. Your armor is covered by a very bright shimmering gold shield!");
            }
          } else {
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "The scroll disappears.");
          }
        } else if (selectedItem.effectType == ScrollIdentifyEffect) {
          if (!seeAll) {
            identifyingItem = true;
            identifyScrollPage = currentInventoryPageIndex;
            identifyScrollIndex = selectedInventoryIndex;
            currentUIState = UI_INVENTORY;
            return;
          } else {
            for (int x = 0; x < numInventoryPages; x++)
              for (int y = 0; y < inventorySize; y++)
                identifyItem(inventoryPages[x].items[y]);
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s",
              "You read in between the lines. Your entire inventory is revealed!");
          }
        } else if (selectedItem.effectType == ScrollEnchantEffect) {
          playerAttackDamage += 2;
          snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You feel more powerful! Your attacks do more damage.");
          if (seeAll) {
            playerAttackDamage += 8;
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s",
              "You read in between the lines. Your attacks do much more damage!");
          }
          for (int p = 0; p < numInventoryPages; p++) {
            if (inventoryPages[p].category != WeaponCategory) continue;
            for (int i = 0; i < inventorySize; i++) {
              GameItem &invItem = inventoryPages[p].items[i];
              if (invItem.item != Null && (seeAll ? true : invItem.isEquipped) && invItem.weapon.type != NoWeapon) {
                for (int w = 0; w < NUM_WEAPONS; w++) {
                  if (weaponList[w].type == invItem.weapon.type) {
                    WeaponType magicType = weaponList[w].magicType;
                    if (invItem.weapon.type != magicType) {
                      for (int m = 0; m < NUM_WEAPONS; m++) {
                        if (weaponList[m].type == magicType) {
                          invItem.weapon = weaponList[m];
                          invItem.canRust = weaponList[m].canRust;
                          snprintf(invItem.name, sizeof(invItem.name), "%s", weaponList[m].name);
                          snprintf(invItem.originalName, sizeof(invItem.originalName), "%s", weaponList[m].name);
                          snprintf(invItem.description, sizeof(invItem.description), "%s", weaponList[m].description);
                          equippedWeapon = invItem;
                          playerAttackDamage = (int)equippedWeapon.weapon.damage;
                          attackDelayFrames = equippedWeapon.weapon.attackDelay;
                          break;
                        }
                      }
                    } else {
                      invItem.weapon.damage += 5;
                      invItem.weapon.attackDelay = max(5, invItem.weapon.attackDelay - 2);
                    }
                    break;
                  }
                }
                p = numInventoryPages;
                break;
              }
            }
          }
          // Consume the enchant scroll
          consumeOneFromStack(currentInventoryPageIndex, selectedInventoryIndex);
          currentUIState = UI_ITEM_RESULT;
        } else if (selectedItem.effectType == ScrollUncurseEffect) {
          for (int p = 0; p < numInventoryPages; p++) {
            for (int i = 0; i < inventorySize; i++) {
              GameItem &item = inventoryPages[p].items[i];
              if ((item.isEquipped || seeAll) && item.isCursed) {
                item.isCursed = false;
                char *cursedPos = strstr(item.description, " (Cursed)");
                if (cursedPos != NULL) {
                  size_t removeLen = 9;
                  size_t remainingLen = strlen(cursedPos + removeLen);
                  memmove(cursedPos, cursedPos + removeLen, remainingLen + 1);
                }
              }
            }
          }
          snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You feel as if someone is watching over you.");
          if (seeAll)
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s",
              "You read in between the lines. Your entire inventory is uncursed!");
          currentUIState = UI_ITEM_RESULT;
        } else if (selectedItem.effectType == ScrollEmptyEffect && seeAll) {
          addToInventory(getItem(KingArmor), false);
          addToInventory(getItem(RiddleStone), false);
          GameItem newWeapon = getItem(Weapon);
          newWeapon.weapon = weaponList[MagicStaff];
          snprintf(newWeapon.description, sizeof(newWeapon.description), "%s", weaponList[MagicStaff].description);
          snprintf(newWeapon.name, sizeof(newWeapon.name), "%s", weaponList[MagicStaff].name);
          snprintf(newWeapon.originalName, sizeof(newWeapon.originalName), "%s", weaponList[MagicStaff].name);
          newWeapon.canRust = weaponList[MagicStaff].canRust;
          addToInventory(newWeapon, false);
          blinded = false; blindnessTimer = 0;
          confused = false; confusionTimer = 0;
          if (ridiculed) { ridiculed = false; ridiculeTimer = 0; showDialogue = false; }
          paralyzed = false; paralysisTimer = 0;
          if (currentSpeedMultiplier < 1) {
            currentSpeedMultiplier = 0; speedTimer = 0; speeding = false; lastPotionSpeedModifier = 0;
          }
          playerAttackDamage += 10;
          playerHP = playerMaxHP;
          playerFood = 100;
          snprintf(itemResultMessage, sizeof(itemResultMessage), "%s",
            "You read the invisible text on the scroll. You feel restored! You have gained something!");
          currentUIState = UI_ITEM_RESULT;
        } else if (selectedItem.effectType == ScrollMapEffect) {
          hasMap = true;
        } else if (selectedItem.effectType == ScrollAmnesiaEffect) {
          resetPotionNames();
          for (int i = 0; i < NUM_SCROLLS; i++)
            snprintf(scrollNamesRevealed[i], sizeof(scrollNamesRevealed[i]), "%s", scrollNames[i]);
          for (int i = 0; i < inventorySize; i++) {
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
          do { newX = random(0, mapWidth); newY = random(0, mapHeight); }
          while (dungeonMap[newY][newX] != Floor);
          playerX = newX; playerY = newY;
        }

        // Consume the scroll after reading (unless identify handled separately)
        if ((selectedItem.effectType != ScrollIdentifyEffect || seeAll) &&
            (selectedItem.effectType != ScrollEnchantEffect) && // already consumed above
            inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].oneTimeUse) {
          consumeOneFromStack(currentInventoryPageIndex, selectedInventoryIndex);
        }
        if (selectedItem.effectType != ScrollIdentifyEffect || seeAll) {
          currentUIState = UI_ITEM_RESULT;
        }
        buttons.bPressedPrev = true;

      } else if ((selectedItem.category == PotionCategory || selectedItem.category == FoodCategory)
                 && selectedItem.item != EmptyBottle) {
        if (strcmp(selectedItem.itemResult, "A lot happens.") == 0) {
          selectedItem.healthRecoverAmount = random(-90, 101);
          selectedItem.hungerRecoverAmount = random(-90, 101);
          selectedItem.AOEsize = random(0, 11);
          selectedItem.AOEdamage = random(-10, 21);
          selectedItem.SpeedMultiplier = (random(-20, 21)) / 10.0;
        }

        playRawSFX(selectedItem.category == PotionCategory ? 6 : 5);
        playerFood += selectedItem.hungerRecoverAmount;
        playerFood = playerFood > 100 ? 100 : playerFood;
        playerHP += selectedItem.healthRecoverAmount;
        playerHP = playerHP > (playerMaxHP - (sicknessRingsNumber*20)) ? (playerMaxHP - (sicknessRingsNumber*20)) : playerHP;

        if (speeding) speedTimer += 500;

        if (selectedItem.SpeedMultiplier != 0) {
          speeding = true;
          lastPotionSpeedModifier = selectedItem.SpeedMultiplier;
          currentSpeedMultiplier += selectedItem.SpeedMultiplier;
        }

        checkIfDeadFrom("poison");

        if (selectedItem.AOEsize > 0) {
          applyAOEEffect(playerX, playerY, selectedItem.AOEsize, selectedItem.AOEdamage);
          spawnParticles(playerX, playerY, 15, 0.4f, true);
          playRawSFX(10);
        }

        if (strcmp(selectedItem.itemResult, "You are now more hungry.") == 0) {
          playerFood -= 51;
        } else if (strcmp(selectedItem.itemResult, "You can now see that which was unseen for a limited time.") == 0) {
          seeAll = true; seeAllTimer = 1000; blinded = false; blindnessTimer = 0;
        } else if (strcmp(selectedItem.itemResult, "What is going on?") == 0) {
          confused = true; confusionTimer = 1000;
        } else if (strcmp(selectedItem.itemResult, "You feel fabulous!") == 0) {
          if (ridiculed) { ridiculed = false; ridiculeTimer = 0; }
          glamoured = true; glamourTimer = 1000;
        } else if (strcmp(selectedItem.itemResult, "You feel stupid.") == 0) {
          if (glamoured) { glamoured = false; glamourTimer = 0; }
          ridiculed = true; ridiculeTimer = RIDICULE_DURATION;
        } else if (strcmp(selectedItem.itemResult, "A cloak of darkness falls around you.") == 0) {
          blinded = true; blindnessTimer = 700;
        } else if (strcmp(selectedItem.itemResult, "You feel stronger.") == 0) {
          playerAttackDamage += 3;
        } else if (strcmp(selectedItem.itemResult, "You feel restored!") == 0) {
          blinded = false; blindnessTimer = 0;
          confused = false; confusionTimer = 0;
          if (ridiculed) { ridiculed = false; ridiculeTimer = 0; showDialogue = false; }
          paralyzed = false; paralysisTimer = 0;
          if (currentSpeedMultiplier < 1) {
            currentSpeedMultiplier = 0; speedTimer = 0; speeding = false; lastPotionSpeedModifier = 0;
          }
          playerAttackDamage += 3;
          playerHP += 15;
          playerHP = playerHP > (playerMaxHP - (sicknessRingsNumber*20)) ? (playerMaxHP - (sicknessRingsNumber*20)) : playerHP;
          playerFood += 15;
          playerFood = playerFood > 100 ? 100 : playerFood;
        } else if (strcmp(selectedItem.itemResult, "You can't move!") == 0) {
          paralyzed = true; paralysisTimer = 1000;
        }

        // Update potion name on all stacks of the same type
        for (int i = 0; i < inventorySize; i++) {
          if (inventoryPages[currentInventoryPageIndex].items[i].item == selectedItem.item) {
            updatePotionName(inventoryPages[currentInventoryPageIndex].items[i]);
          }
        }

        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", selectedItem.itemResult);

        if (selectedItem.effectType == ArmorEffect || selectedItem.category == WeaponCategory)
          snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You can't use this, Try equipping it.");

        currentUIState = UI_ITEM_RESULT;

        if (inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].oneTimeUse) {
          if (selectedItem.category == PotionCategory) {
            // Consume one from the stack; if slot empties, put an empty bottle there
            bool slotEmptied = consumeOneFromStack(currentInventoryPageIndex, selectedInventoryIndex);
            if (slotEmptied) {
              // Place empty bottle in the now-clear slot (or stack it elsewhere)
              addToInventory(getItem(EmptyBottle), false);
            }
          } else {
            consumeOneFromStack(currentInventoryPageIndex, selectedInventoryIndex);
          }
        }

        buttons.bPressedPrev = true;

      } else if (selectedItem.effectType == ArmorEffect || selectedItem.category == WeaponCategory || selectedItem.item == Ring) {
        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You can't use this, try equipping it.");
        currentUIState = UI_ITEM_RESULT;
        buttons.bPressedPrev = true;
      } else if (selectedItem.item == RiddleStone) {
        currentUIState = UI_RIDDLE;
        consumeOneFromStack(currentInventoryPageIndex, selectedInventoryIndex);
      } else {
        playRawSFX(2);
        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", selectedItem.itemResult);
        currentUIState = UI_ITEM_RESULT;
        buttons.bPressedPrev = true;
      }

    } else if (selectedActionIndex == 1) { // Drop
      if (selectedItem.isEquipped) {
        playRawSFX(13);
        snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You need to unequip it first.");
        currentUIState = UI_ITEM_RESULT;
        buttons.bPressedPrev = true;
      } else {
        // Drop one from the stack
        consumeOneFromStack(currentInventoryPageIndex, selectedInventoryIndex);
        currentUIState = UI_INVENTORY;
      }

    } else if (selectedActionIndex == 2) { // Info
      currentUIState = UI_ITEM_INFO;

    } else if (selectedActionIndex == 3) { // Equip / Unequip
      if (selectedItem.category == EquipmentCategory || selectedItem.category == WeaponCategory) {
        if (selectedItem.isEquipped) {
          if (selectedItem.isCursed) {
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You can't. It appears to be cursed.");
            currentUIState = UI_ITEM_RESULT;
          } else if (strcmp(selectedItem.originalName, "Washer") == 0) {
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s",
              "You try to remove the washer from your finger, but it fails to go past your knuckle.");
            currentUIState = UI_ITEM_RESULT;
          } else {
            selectedItem.isEquipped = false;
            if (selectedItem.effectType == ArmorEffect) { equippedArmorValue = 0; equippedArmor = {}; }
            if (selectedItem.item == RiddleStone) equippedRiddleStone = false;
            if (selectedItem.item == Ring) {
              int idx = selectedItem.ringEffectIndex;
              if (strcmp(ringEffects[idx], "Ring of Swiftness") == 0)      swiftnessRingsNumber -= 1;
              else if (strcmp(ringEffects[idx], "Ring of Strength") == 0)  strengthRingsNumber -= 1;
              else if (strcmp(ringEffects[idx], "Ring of Weakness") == 0)  weaknessRingsNumber -= 1;
              else if (strcmp(ringEffects[idx], "Ring of Hunger") == 0)    hungerRingsNumber -= 1;
              else if (strcmp(ringEffects[idx], "Ring of Regeneration") == 0) regenRingsNumber -= 1;
              else if (strcmp(ringEffects[idx], "Ring of Sickness") == 0)  sicknessRingsNumber -= 1;
              else if (strcmp(ringEffects[idx], "Ring of Aggravation") == 0) aggravateRingsNumber -= 1;
              else if (strcmp(ringEffects[idx], "Ring of Armor") == 0)     armorRingsNumber -= 1;
              else if (strcmp(ringEffects[idx], "Ring of Indigestion") == 0) indigestionRingsNumber -= 1;
              else if (strcmp(ringEffects[idx], "Ring of Teleport") == 0)  teleportRingsNumber -= 1;
              else if (strcmp(ringEffects[idx], "Ring of Invisibility") == 0) invisibleRingsNumber -= 1;
            }
            if (selectedItem.category == WeaponCategory) {
              if (equippedWeapon.item == selectedItem.item &&
                  strcmp(equippedWeapon.name, selectedItem.name) == 0) {
                equippedWeapon = {};
                equippedWeapon.weapon.type = NoWeapon;
                playerAttackDamage = 10;
                attackDelayFrames = 10;
              }
            }
            playRawSFX(2);
            currentUIState = UI_INVENTORY;
          }
        } else {
          if (selectedItem.effectType == ArmorEffect && equippedArmor.item != Null) {
            playRawSFX(13);
            snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You need to unequip your current armor first.");
            currentUIState = UI_ITEM_RESULT;
            buttons.bPressedPrev = true;
          } else {
            if (selectedItem.category == WeaponCategory) {
              if (equippedWeapon.weapon.type == NoWeapon) {
                for (int p = 0; p < numInventoryPages; p++) {
                  if (inventoryPages[p].category != WeaponCategory) continue;
                  for (int i = 0; i < inventorySize; i++) {
                    if (inventoryPages[p].items[i].item != Null && inventoryPages[p].items[i].isEquipped)
                      inventoryPages[p].items[i].isEquipped = false;
                  }
                }
                selectedItem.isEquipped = true;
                equippedWeapon = selectedItem;
                playerAttackDamage = (int)equippedWeapon.weapon.damage;
                attackDelayFrames = equippedWeapon.weapon.attackDelay;
              } else {
                playRawSFX(13);
                snprintf(itemResultMessage, sizeof(itemResultMessage), "%s", "You need to unequip your current weapon first.");
                currentUIState = UI_ITEM_RESULT;
                buttons.bPressedPrev = true;
              }
            } else {
              selectedItem.isEquipped = true;
              if (selectedItem.effectType == ArmorEffect) {
                equippedArmorValue = selectedItem.armorValue;
                equippedArmor = selectedItem;
              }
            }
            if (selectedItem.item == Ring) {
              if (selectedItem.ringEffectIndex < 0 || selectedItem.ringEffectIndex >= NUM_RINGS) {
                int assignIdx = random(0, NUM_RINGS);
                selectedItem.ringEffectIndex = assignIdx;
                selectedItem.isCursed = ringCursed[assignIdx];
              }
              int idx = selectedItem.ringEffectIndex;
              if (strcmp(ringEffects[idx], "Ring of Swiftness") == 0)      swiftnessRingsNumber += 1;
              else if (strcmp(ringEffects[idx], "Ring of Strength") == 0)  strengthRingsNumber += 1;
              else if (strcmp(ringEffects[idx], "Ring of Weakness") == 0)  weaknessRingsNumber += 1;
              else if (strcmp(ringEffects[idx], "Ring of Hunger") == 0)    hungerRingsNumber += 1;
              else if (strcmp(ringEffects[idx], "Ring of Regeneration") == 0) regenRingsNumber += 1;
              else if (strcmp(ringEffects[idx], "Ring of Sickness") == 0) {
                sicknessRingsNumber += 1;
                playerHP = playerMaxHP - (sicknessRingsNumber*20);
              }
              else if (strcmp(ringEffects[idx], "Ring of Aggravation") == 0) aggravateRingsNumber += 1;
              else if (strcmp(ringEffects[idx], "Ring of Armor") == 0)     armorRingsNumber += 1;
              else if (strcmp(ringEffects[idx], "Ring of Indigestion") == 0) indigestionRingsNumber += 1;
              else if (strcmp(ringEffects[idx], "Ring of Teleport") == 0)  teleportRingsNumber += 1;
              else if (strcmp(ringEffects[idx], "Ring of Invisibility") == 0) invisibleRingsNumber += 1;
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

// ─────────────────────────────────────────────────────────────────────────────
// renderInventory — shows stack count badge on stacked items
// ─────────────────────────────────────────────────────────────────────────────
bool showTooltip = true;
void renderInventory() {
  display.clearDisplay();
  display.setTextSize(1);

  if (currentUIState == UI_INVENTORY) {
    display.setCursor(combiningTwoItems ? 0 : 10, combiningTwoItems ? 0 : 3);
    display.setTextSize(combiningTwoItems || identifyingItem ? 1 : 2);
    display.println(combiningTwoItems ? "Select second item tocombine..."
                   : identifyingItem  ? "Select item to identify..."
                                      : "Inventory");
    display.setTextSize(1);
    display.setCursor(10, 20);
    display.setTextColor(0, 15);
    display.fillRect(0, 19, 128, 9, 15);
    char pageName[40];
    snprintf(pageName, sizeof(pageName), "<%s>", inventoryPages[currentInventoryPageIndex].name);
    display.println(pageName);
    display.setTextColor(15, 0);

    int yPos = 30;
    InventoryPage &currentPage = inventoryPages[currentInventoryPageIndex];
    for (int i = 0; i < inventorySize; i++) {
      GameItem &item = currentPage.items[i];
      if (item.item == Null) continue;

      display.setCursor(15, yPos);
      if (i == selectedInventoryIndex) display.print("> ");

      const char* displayName = item.name;
      if (combiningTwoItems) {
        GameItem result = CombineTwoItemsToGetItem(combiningItem1, item);
        if (result.item != Null) {
          if (i == selectedInventoryIndex) {
            displayName = result.name;
            display.print(">>");
          }
        }
      }
      display.print(displayName);

      // Stack count badge — show "x N" to the right when stack > 1
      if (item.stackCount > 1) {
        char stackStr[6];
        snprintf(stackStr, sizeof(stackStr), "x%d", item.stackCount);
        // Right-align: each char is ~6px wide at textSize 1
        int badgeX = 128 - (strlen(stackStr) * 6) - (item.isEquipped ? 14 : 2);
        display.setCursor(badgeX, yPos);
        display.print(stackStr);
      }

      if (item.isEquipped) {
        display.setCursor(110, yPos);
        display.print("*");
      }

      yPos += 12;
    }

    if (yPos == 30) {
      display.setCursor(15, 30);
      display.println("Empty");
    }

    if (showTooltip) {
      display.setCursor(0, 120);
      display.print("* = Equipped");
    }

  } else if (currentUIState == UI_ITEM_INFO) {
    display.setCursor(0, 120);
    display.println(inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].originalName);
    display.setCursor(0, 10);
    display.print(inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex].description);

  } else if (currentUIState == UI_ITEM_RESULT) {
    display.setCursor(0, 10);
    display.println(itemResultMessage);
    if (buttons.bPressed && !buttons.bPressedPrev) {
      currentUIState = UI_NORMAL;
    }
    showTooltip = false;

  } else if (currentUIState == UI_ITEM_ACTION) {
    display.drawRect(50, 40, 65, 75, 15);
    display.fillRect(50, 40, 65, 12, 15);

    display.setTextColor(0, 15);
    display.setCursor(55, 42);
    display.println("Options:");
    display.setTextColor(15, 0);

    GameItem &selectedItem = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];
    const char* equipText = (selectedItem.isEquipped &&
      (selectedItem.category == EquipmentCategory || selectedItem.category == WeaponCategory))
      ? "Unequip" : "Equip";

    const char* useText = "Use";
    if (selectedItem.item == Scroll)                                          useText = "Read";
    else if (selectedItem.category == PotionCategory && selectedItem.item != EmptyBottle) useText = "Drink";
    else if (selectedItem.category == FoodCategory   && selectedItem.item != EmptyBottle) useText = "Eat";

    // Show "Drop 1" instead of "Drop" when the stack has more than one item
    char dropLabel[12];
    if (selectedItem.stackCount > 1) snprintf(dropLabel, sizeof(dropLabel), "Drop 1");
    else                             snprintf(dropLabel, sizeof(dropLabel), "Drop");

    char useLine[20], equipLine[20], dropLine[20];
    snprintf(useLine,   sizeof(useLine),   "%s %s", (selectedActionIndex == 0 ? ">" : ""), useText);
    snprintf(dropLine,  sizeof(dropLine),  "%s %s", (selectedActionIndex == 1 ? ">" : ""), dropLabel);
    snprintf(equipLine, sizeof(equipLine), "%s %s", (selectedActionIndex == 3 ? ">" : ""), equipText);

    display.setCursor(55, 60); display.println(useLine);
    display.setCursor(55, 70); display.println(dropLine);
    display.setCursor(55, 80); display.println(selectedActionIndex == 2 ? "> Info"    : " Info");
    display.setCursor(55, 90); display.println(equipLine);
    display.setCursor(55, 100); display.println(selectedActionIndex == 4 ? "> Combine" : " Combine");
  }

  display.display();
}

// ─────────────────────────────────────────────────────────────────────────────
// removeItemFromInventory — removes a whole slot and shifts others left.
// Use consumeOneFromStack instead when you just want to decrement a stack.
// ─────────────────────────────────────────────────────────────────────────────
void removeItemFromInventory(int page, int index) {
  InventoryPage &invPage = inventoryPages[page];
  for (int i = index; i < inventorySize - 1; i++) {
    invPage.items[i] = invPage.items[i + 1];
  }
  GameItem nullItem = getItem(Null);
  nullItem.stackCount = 0;
  invPage.items[inventorySize - 1] = nullItem;
  invPage.itemCount--;
}