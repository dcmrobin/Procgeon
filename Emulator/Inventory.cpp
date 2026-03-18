#include "Common.h"
#include "GameState.h"
#include "Item.h"
#include "Inventory.h"
#include "HelperFunctions.h"
#include "Player.h"
#include "GameAudio.h"
#include "Dungeon.h"
#include "Entities.h"

#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// Inventory.cpp — inventory data, navigation, item usage, rendering
// Logic identical to original; includes and magic numbers cleaned up.
// ─────────────────────────────────────────────────────────────────────────────

InventoryPage inventoryPages[] = {
    {{"Potions"},    PotionCategory},
    {{"Food"},       FoodCategory},
    {{"Equipment"},  EquipmentCategory},
    {{"Scrolls"},    ScrollsCategory},
    {{"Weapons"},    WeaponCategory},
};
int numInventoryPages = sizeof(inventoryPages) / sizeof(inventoryPages[0]);

// ─────────────────────────────────────────────────────────────────────────────
// identifyItem
// ─────────────────────────────────────────────────────────────────────────────
void identifyItem(GameItem& item) {
    if      (item.category == PotionCategory) updatePotionName(item);
    else if (item.item == Scroll)             updateScrollName(item);
    else if (item.item == Ring)               updateRingName(item);

    if (item.isCursed && strstr(item.description, "(Cursed)") == nullptr) {
        char tmp[110];
        snprintf(tmp, sizeof(tmp), "%s (Cursed)", item.description);
        snprintf(item.description, sizeof(item.description), "%s", tmp);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// consumeOneFromStack  (file-private)
// Returns true if the slot was cleared.
// ─────────────────────────────────────────────────────────────────────────────
static bool consumeOneFromStack(int page, int index) {
    GameItem& slot = inventoryPages[page].items[index];
    if (slot.stackCount > 1) { slot.stackCount--; return false; }
    slot = { Null, PotionCategory, "Empty" };
    slot.stackCount = 0;
    inventoryPages[page].itemCount--;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// addToInventory
// ─────────────────────────────────────────────────────────────────────────────
bool addToInventory(GameItem item, bool canBeCursed) {
    if (canBeCursed && random(0, 11) < item.curseChance)
        item.isCursed = true;

    // Resolve generic weapon
    if (item.category == WeaponCategory && strcmp(item.name, "Weapon") == 0) {
        int wi = random(0, 4);
        item.weapon = weaponList[wi];
        snprintf(item.description,  sizeof(item.description),  "%s", weaponList[wi].description);
        snprintf(item.name,         sizeof(item.name),          "%s", weaponList[wi].name);
        snprintf(item.originalName, sizeof(item.originalName),  "%s", weaponList[wi].name);
        item.canRust = weaponList[wi].canRust;
    }

    if (item.item != Null && item.stackCount < 1) item.stackCount = 1;

    for (int p = 0; p < numInventoryPages; p++) {
        if (inventoryPages[p].category != item.category) continue;

        // Try to merge into existing stack
        if (isStackable(item)) {
            for (int i = 0; i < INVENTORY_SIZE; i++) {
                GameItem& slot = inventoryPages[p].items[i];
                if (slot.item != Null && canStackWith(slot, item) &&
                    slot.stackCount < MAX_STACK_SIZE) {
                    slot.stackCount++;
                    return true;
                }
            }
        }

        // New slot
        if (inventoryPages[p].itemCount >= INVENTORY_SIZE) return false;
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            if (inventoryPages[p].items[i].item != Null) continue;
            item.stackCount = 1;
            inventoryPages[p].items[i] = item;
            inventoryPages[p].itemCount++;
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Navigation
// ─────────────────────────────────────────────────────────────────────────────
void handleInventoryNavigation() {
    if (currentUIState != UI_INVENTORY) return;
    const ButtonStates& b = g_state.buttons;

    if (b.leftPressed  && !b.leftPressedPrev)  {
        playRawSFX(8);
        currentInventoryPageIndex = (currentInventoryPageIndex - 1 + numInventoryPages) % numInventoryPages;
        selectedInventoryIndex = findFirstItemInCurrentCategory();
    }
    if (b.rightPressed && !b.rightPressedPrev) {
        playRawSFX(8);
        currentInventoryPageIndex = (currentInventoryPageIndex + 1) % numInventoryPages;
        selectedInventoryIndex = findFirstItemInCurrentCategory();
    }
    if (b.upPressed    && !b.upPressedPrev)    {
        playRawSFX(8);
        selectedInventoryIndex = findPreviousItemInCategory(selectedInventoryIndex);
    }
    if (b.downPressed  && !b.downPressedPrev)  {
        playRawSFX(8);
        selectedInventoryIndex = findNextItemInCategory(selectedInventoryIndex);
    }
}

int findFirstItemInCurrentCategory() {
    for (int i = 0; i < INVENTORY_SIZE; i++)
        if (inventoryPages[currentInventoryPageIndex].items[i].item != Null) return i;
    return 0;
}

int findNextItemInCategory(int current) {
    ItemCategory cat = inventoryPages[currentInventoryPageIndex].category;
    for (int i = current + 1; i < INVENTORY_SIZE; i++)
        if (inventoryPages[currentInventoryPageIndex].items[i].item != Null &&
            inventoryPages[currentInventoryPageIndex].items[i].category == cat) return i;
    for (int i = 0; i <= current; i++)
        if (inventoryPages[currentInventoryPageIndex].items[i].item != Null &&
            inventoryPages[currentInventoryPageIndex].items[i].category == cat) return i;
    return -1;
}

int findPreviousItemInCategory(int current) {
    ItemCategory cat = inventoryPages[currentInventoryPageIndex].category;
    for (int i = current - 1; i >= 0; i--)
        if (inventoryPages[currentInventoryPageIndex].items[i].item != Null &&
            inventoryPages[currentInventoryPageIndex].items[i].category == cat) return i;
    for (int i = INVENTORY_SIZE - 1; i >= current; i--)
        if (inventoryPages[currentInventoryPageIndex].items[i].item != Null &&
            inventoryPages[currentInventoryPageIndex].items[i].category == cat) return i;
    return -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// handleInventoryItemUsage
// ─────────────────────────────────────────────────────────────────────────────
void handleInventoryItemUsage() {
    const ButtonStates& b = g_state.buttons;

    // Identify mode: pressing B on any item identifies it
    if (b.bPressed && !b.bPressedPrev && identifyingItem && currentUIState == UI_INVENTORY) {
        GameItem& sel = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];
        if (strcmp(sel.name, "Empty") != 0 && strlen(sel.name) > 0) {
            identifyItem(sel);
            snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                     "Identified: %s%s", sel.name,
                     sel.isCursed ? ". It is cursed!" : ". Not cursed.");
            if (identifyScrollPage >= 0 && identifyScrollIndex >= 0)
                consumeOneFromStack(identifyScrollPage, identifyScrollIndex);
            identifyingItem    = false;
            identifyScrollPage = identifyScrollIndex = -1;
            currentUIState     = UI_ITEM_RESULT;
            playRawSFX(2);
        }
        return;
    }

    if (!(b.bPressed && !b.bPressedPrev && !identifyingItem && currentUIState == UI_INVENTORY)) return;

    GameItem& sel = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];
    if (strcmp(sel.name, "Empty") == 0 || strlen(sel.name) == 0) return;

    playRawSFX(7);

    if (!combiningTwoItems) {
        currentUIState    = UI_ITEM_ACTION;
        selectedActionIndex = 0;
        return;
    }

    // Combining mode
    combiningItem2 = sel;
    GameItem result = CombineTwoItemsToGetItem(combiningItem1, combiningItem2);

    if (strcmp(result.name, "Null") != 0) {
        int p1 = -1, i1 = -1, p2 = -1, i2 = -1;
        for (int p = 0; p < numInventoryPages; p++)
            for (int i = 0; i < INVENTORY_SIZE; i++) {
                if (p1 == -1 && inventoryPages[p].items[i].item == combiningItem1.item &&
                    strcmp(inventoryPages[p].items[i].name, combiningItem1.name) == 0)
                    { p1 = p; i1 = i; }
                if (p2 == -1 && inventoryPages[p].items[i].item == combiningItem2.item &&
                    strcmp(inventoryPages[p].items[i].name, combiningItem2.name) == 0)
                    { p2 = p; i2 = i; }
            }

        if (p1 != -1) consumeOneFromStack(p1, i1);
        if (p2 != -1) consumeOneFromStack(p2, i2);

        addToInventory(result, false);
        if (combiningItem1.category == PotionCategory ||
            combiningItem2.category == PotionCategory)
            addToInventory(getItem(EmptyBottle), false);
        selectedInventoryIndex = 0;
    }

    currentUIState = UI_ITEM_RESULT;
    if (result.category != EquipmentCategory) {
        snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                 strcmp(result.name, "Null") == 0
                 ? "%s" : "Combined! Result: %s",
                 strcmp(result.name, "Null") == 0
                 ? "These two items cannot be combined." : result.name);
    } else {
        snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                 "%s", result.itemResult);
    }
    combiningTwoItems = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// handleItemActionMenu
// ─────────────────────────────────────────────────────────────────────────────
void handleItemActionMenu() {
    const ButtonStates& b = g_state.buttons;
    if (b.upPressed   && !b.upPressedPrev)   { playRawSFX(8); selectedActionIndex--; }
    if (b.downPressed && !b.downPressedPrev) { playRawSFX(8); selectedActionIndex++; }
    if (selectedActionIndex == 5)  selectedActionIndex = 0;
    if (selectedActionIndex == -1) selectedActionIndex = 4;

    if (b.aPressed && !b.aPressedPrev) { currentUIState = UI_INVENTORY; return; }
    if (!(b.bPressed && !b.bPressedPrev)) return;

    playRawSFX(7);
    GameItem& sel = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];

    // ── Use / Drink / Read / Eat ──────────────────────────────────────────
    if (selectedActionIndex == 0) {

        if (sel.category == ScrollsCategory) {
            if (blinded) {
                snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                         "%s", "You can't read while blind!");
                currentUIState    = UI_ITEM_RESULT;
                g_state.buttons.bPressedPrev = true;
                return;
            }
            if (!sel.isScrollRevealed) updateScrollName(sel);
            playRawSFX(2);
            snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                     "%s", sel.itemResult);

            if (sel.effectType == ScrollProtectionEffect) {
                if (equippedArmor.item != Null) {
                    if (equippedArmor.armorValue == 0)
                        snprintf(equippedArmor.description, sizeof(equippedArmor.description),
                                 "%s", "Restored armor.");
                    equippedArmor.armorValue += 1;
                    equippedArmorValue       += 1;
                    if (seeAll) {
                        equippedArmor.armorValue += 9;
                        equippedArmorValue       += 9;
                        snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                                 "%s", "You read in between the lines. "
                                 "Your armor is covered by a very bright shimmering gold shield!");
                    }
                } else {
                    snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                             "%s", "The scroll disappears.");
                }
            } else if (sel.effectType == ScrollIdentifyEffect) {
                if (!seeAll) {
                    identifyingItem    = true;
                    identifyScrollPage = currentInventoryPageIndex;
                    identifyScrollIndex= selectedInventoryIndex;
                    currentUIState     = UI_INVENTORY;
                    return;
                } else {
                    for (int x = 0; x < numInventoryPages; x++)
                        for (int y = 0; y < INVENTORY_SIZE; y++)
                            identifyItem(inventoryPages[x].items[y]);
                    snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                             "%s", "You read in between the lines. Your entire inventory is revealed!");
                }
            } else if (sel.effectType == ScrollEnchantEffect) {
                playerAttackDamage += 2;
                snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                         "%s", "You feel more powerful! Your attacks do more damage.");
                if (seeAll) {
                    playerAttackDamage += 8;
                    snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                             "%s", "You read in between the lines. Your attacks do much more damage!");
                }
                for (int p = 0; p < numInventoryPages; p++) {
                    if (inventoryPages[p].category != WeaponCategory) continue;
                    for (int i = 0; i < INVENTORY_SIZE; i++) {
                        GameItem& it = inventoryPages[p].items[i];
                        if (it.item == Null || it.weapon.type == NoWeapon) continue;
                        if (!seeAll && !it.isEquipped) continue;
                        for (int w = 0; w < NUM_WEAPONS; w++) {
                            if (weaponList[w].type != it.weapon.type) continue;
                            WeaponType mt = weaponList[w].magicType;
                            if (it.weapon.type != mt) {
                                for (int m = 0; m < NUM_WEAPONS; m++) {
                                    if (weaponList[m].type != mt) continue;
                                    it.weapon  = weaponList[m];
                                    it.canRust = weaponList[m].canRust;
                                    snprintf(it.name,        sizeof(it.name),        "%s", weaponList[m].name);
                                    snprintf(it.originalName,sizeof(it.originalName),"%s", weaponList[m].name);
                                    snprintf(it.description, sizeof(it.description), "%s", weaponList[m].description);
                                    equippedWeapon     = it;
                                    playerAttackDamage = (int)equippedWeapon.weapon.damage;
                                    attackDelayFrames  = equippedWeapon.weapon.attackDelay;
                                    break;
                                }
                            } else {
                                it.weapon.damage      += 5;
                                it.weapon.attackDelay  = max(5, it.weapon.attackDelay - 2);
                            }
                            p = numInventoryPages; break;
                        }
                    }
                }
                consumeOneFromStack(currentInventoryPageIndex, selectedInventoryIndex);
                currentUIState = UI_ITEM_RESULT;
            } else if (sel.effectType == ScrollUncurseEffect) {
                for (int p = 0; p < numInventoryPages; p++)
                    for (int i = 0; i < INVENTORY_SIZE; i++) {
                        GameItem& it = inventoryPages[p].items[i];
                        if ((!it.isEquipped && !seeAll) || !it.isCursed) continue;
                        it.isCursed = false;
                        char* pos = strstr(it.description, " (Cursed)");
                        if (pos) memmove(pos, pos + 9, strlen(pos + 9) + 1);
                    }
                snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                         "%s", seeAll
                         ? "You read in between the lines. Your entire inventory is uncursed!"
                         : "You feel as if someone is watching over you.");
                currentUIState = UI_ITEM_RESULT;
            } else if (sel.effectType == ScrollEmptyEffect && seeAll) {
                addToInventory(getItem(KingArmor),    false);
                addToInventory(getItem(RiddleStone),  false);
                GameItem mw = getItem(Weapon);
                mw.weapon = weaponList[MagicStaff];
                snprintf(mw.description,  sizeof(mw.description),  "%s", weaponList[MagicStaff].description);
                snprintf(mw.name,         sizeof(mw.name),          "%s", weaponList[MagicStaff].name);
                snprintf(mw.originalName, sizeof(mw.originalName),  "%s", weaponList[MagicStaff].name);
                mw.canRust = weaponList[MagicStaff].canRust;
                addToInventory(mw, false);
                blinded=false; blindnessTimer=0; confused=false; confusionTimer=0;
                if (ridiculed) { ridiculed=false; ridiculeTimer=0; showDialogue=false; }
                paralyzed=false; paralysisTimer=0;
                if (currentSpeedMultiplier < 1) {
                    currentSpeedMultiplier=0; speedTimer=0; speeding=false; lastPotionSpeedModifier=0;
                }
                playerAttackDamage += 10;
                playerHP = playerMaxHP; playerFood = PLAYER_START_FOOD;
                snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                         "%s", "You read the invisible text on the scroll. "
                         "You feel restored! You have gained something!");
                currentUIState = UI_ITEM_RESULT;
            } else if (sel.effectType == ScrollMapEffect) {
                hasMap = true;
            } else if (sel.effectType == ScrollAmnesiaEffect) {
                resetPotionNames();
                for (int i = 0; i < NUM_SCROLLS; i++)
                    snprintf(scrollNamesRevealed[i], sizeof(scrollNamesRevealed[0]),
                             "%s", scrollNames[i]);
                for (int i = 0; i < INVENTORY_SIZE; i++) {
                    snprintf(inventoryPages[0].items[i].name, sizeof(inventoryPages[0].items[i].name),
                             "%s", inventoryPages[0].items[i].originalName);
                    snprintf(inventoryPages[3].items[i].name, sizeof(inventoryPages[3].items[i].name),
                             "%s", inventoryPages[3].items[i].originalName);
                    if (inventoryPages[2].items[i].item == Ring) {
                        snprintf(inventoryPages[2].items[i].name,
                                 sizeof(inventoryPages[2].items[i].name),
                                 "%s", inventoryPages[2].items[i].originalName);
                        snprintf(inventoryPages[2].items[i].description,
                                 sizeof(inventoryPages[2].items[i].description),
                                 "%s", "You don't remember what this does.");
                    }
                    snprintf(inventoryPages[0].items[i].description,
                             sizeof(inventoryPages[0].items[i].description),
                             "%s", "You don't remember what this does.");
                    snprintf(inventoryPages[3].items[i].description,
                             sizeof(inventoryPages[3].items[i].description),
                             "%s", "You don't remember what this does.");
                }
            } else if (sel.effectType == ScrollAggravateEffect) {
                ambientNoiseLevel = 100000;
            } else if (sel.effectType == ScrollDestroyEffect) {
                equippedArmor.armorValue = 0;
                equippedArmorValue       = 0;
            } else if (sel.effectType == ScrollTeleportEffect) {
                playRawSFX(14);
                int nx, ny;
                do { nx = random(0, MAP_WIDTH); ny = random(0, MAP_HEIGHT); }
                while (dungeonMap[ny][nx] != Floor);
                playerX = (float)nx; playerY = (float)ny;
            }

            if ((sel.effectType != ScrollIdentifyEffect || seeAll) &&
                sel.effectType != ScrollEnchantEffect &&
                inventoryPages[currentInventoryPageIndex]
                    .items[selectedInventoryIndex].oneTimeUse)
                consumeOneFromStack(currentInventoryPageIndex, selectedInventoryIndex);

            if (sel.effectType != ScrollIdentifyEffect || seeAll)
                currentUIState = UI_ITEM_RESULT;
            g_state.buttons.bPressedPrev = true;
            return;
        }

        // Potion / food
        if ((sel.category == PotionCategory || sel.category == FoodCategory) &&
            sel.item != EmptyBottle) {

            if (strcmp(sel.itemResult, "A lot happens.") == 0) {
                sel.healthRecoverAmount = random(-90, 101);
                sel.hungerRecoverAmount = random(-90, 101);
                sel.AOEsize             = random(0, 11);
                sel.AOEdamage           = random(-10, 21);
                sel.SpeedMultiplier     = (float)random(-20, 21) / 10.0f;
            }

            playRawSFX(sel.category == PotionCategory ? 6 : 5);
            playerFood += sel.hungerRecoverAmount;
            if (playerFood > 100) playerFood = 100;
            playerHP   += sel.healthRecoverAmount;
            int maxHPNow = playerMaxHP - (sicknessRingsNumber * 20);
            if (playerHP > maxHPNow) playerHP = maxHPNow;

            if (speeding) speedTimer += SPEED_TIMER_DEFAULT;
            if (sel.SpeedMultiplier != 0) {
                speeding                = true;
                lastPotionSpeedModifier = sel.SpeedMultiplier;
                currentSpeedMultiplier += sel.SpeedMultiplier;
            }

            checkIfDeadFrom("poison");

            if (sel.AOEsize > 0) {
                applyAOEEffect(playerX, playerY, sel.AOEsize, sel.AOEdamage);
                spawnParticles(playerX, playerY, 15, 0.4f, true);
                playRawSFX(10);
            }

            // Status effect triggers
            const char* res = sel.itemResult;
            if      (strcmp(res, "You are now more hungry.") == 0)                    playerFood -= 51;
            else if (strcmp(res, "You can now see that which was unseen for a limited time.") == 0)
                                                                                      { seeAll=true; seeAllTimer=SEE_ALL_TIMER_DEFAULT; blinded=false; blindnessTimer=0; }
            else if (strcmp(res, "What is going on?") == 0)                           { confused=true; confusionTimer=CONFUSION_TIMER_DEFAULT; }
            else if (strcmp(res, "You feel fabulous!") == 0)                          { if(ridiculed){ridiculed=false;ridiculeTimer=0;} glamoured=true; glamourTimer=GLAMOUR_TIMER_DEFAULT; }
            else if (strcmp(res, "You feel stupid.") == 0)                            { if(glamoured){glamoured=false;glamourTimer=0;} ridiculed=true; ridiculeTimer=RIDICULE_DURATION; }
            else if (strcmp(res, "A cloak of darkness falls around you.") == 0)       { blinded=true; blindnessTimer=BLINDNESS_TIMER_DEFAULT; }
            else if (strcmp(res, "You feel stronger.") == 0)                          playerAttackDamage += 3;
            else if (strcmp(res, "You feel restored!") == 0) {
                blinded=false; blindnessTimer=0; confused=false; confusionTimer=0;
                if (ridiculed) { ridiculed=false; ridiculeTimer=0; showDialogue=false; }
                paralyzed=false; paralysisTimer=0;
                if (currentSpeedMultiplier < 1) { currentSpeedMultiplier=0; speedTimer=0; speeding=false; lastPotionSpeedModifier=0; }
                playerAttackDamage += 3;
                playerHP += 15; if (playerHP > maxHPNow) playerHP = maxHPNow;
                playerFood += 15; if (playerFood > 100) playerFood = 100;
            }
            else if (strcmp(res, "You can't move!") == 0)                             { paralyzed=true; paralysisTimer=PARALYSIS_TIMER_DEFAULT; }

            // Update potion name on all stacks
            for (int i = 0; i < INVENTORY_SIZE; i++)
                if (inventoryPages[currentInventoryPageIndex].items[i].item == sel.item)
                    updatePotionName(inventoryPages[currentInventoryPageIndex].items[i]);

            snprintf(itemResultMessage, sizeof(g_state.itemResultMessage), "%s", sel.itemResult);
            if (sel.effectType == ArmorEffect || sel.category == WeaponCategory)
                snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                         "%s", "You can't use this, Try equipping it.");

            currentUIState = UI_ITEM_RESULT;

            if (sel.oneTimeUse) {
                bool emptied = consumeOneFromStack(currentInventoryPageIndex, selectedInventoryIndex);
                if (emptied && sel.category == PotionCategory)
                    addToInventory(getItem(EmptyBottle), false);
            }
            g_state.buttons.bPressedPrev = true;
            return;
        }

        if (sel.effectType == ArmorEffect ||
            sel.category == WeaponCategory || sel.item == Ring) {
            snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                     "%s", "You can't use this, try equipping it.");
            currentUIState = UI_ITEM_RESULT;
            g_state.buttons.bPressedPrev = true;
            return;
        }

        if (sel.item == RiddleStone) {
            currentUIState = UI_RIDDLE;
            consumeOneFromStack(currentInventoryPageIndex, selectedInventoryIndex);
            return;
        }

        playRawSFX(2);
        snprintf(itemResultMessage, sizeof(g_state.itemResultMessage), "%s", sel.itemResult);
        currentUIState = UI_ITEM_RESULT;
        g_state.buttons.bPressedPrev = true;
        return;
    }

    // ── Drop ─────────────────────────────────────────────────────────────
    if (selectedActionIndex == 1) {
        if (sel.isEquipped) {
            snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                     "%s", "You need to unequip it first.");
            currentUIState = UI_ITEM_RESULT;
        } else {
            consumeOneFromStack(currentInventoryPageIndex, selectedInventoryIndex);
            currentUIState = UI_INVENTORY;
        }
        g_state.buttons.bPressedPrev = true;
        return;
    }

    // ── Info ──────────────────────────────────────────────────────────────
    if (selectedActionIndex == 2) { currentUIState = UI_ITEM_INFO; return; }

    // ── Equip / Unequip ───────────────────────────────────────────────────
    if (selectedActionIndex == 3) {
        if (sel.category == EquipmentCategory || sel.category == WeaponCategory) {
            if (sel.isEquipped) {
                if (sel.isCursed) {
                    snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                             "%s", "You can't. It appears to be cursed.");
                    currentUIState = UI_ITEM_RESULT;
                } else if (strcmp(sel.originalName, "Washer") == 0) {
                    snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                             "%s", "You try to remove the washer from your finger, "
                             "but it fails to go past your knuckle.");
                    currentUIState = UI_ITEM_RESULT;
                } else {
                    sel.isEquipped = false;
                    if (sel.effectType == ArmorEffect) { equippedArmorValue = 0; equippedArmor = {}; }
                    if (sel.item == RiddleStone) equippedRiddleStone = false;
                    if (sel.item == Ring) {
                        int idx = sel.ringEffectIndex;
                        if      (strcmp(ringEffects[idx], "Ring of Swiftness")    == 0) swiftnessRingsNumber--;
                        else if (strcmp(ringEffects[idx], "Ring of Strength")     == 0) strengthRingsNumber--;
                        else if (strcmp(ringEffects[idx], "Ring of Weakness")     == 0) weaknessRingsNumber--;
                        else if (strcmp(ringEffects[idx], "Ring of Hunger")       == 0) hungerRingsNumber--;
                        else if (strcmp(ringEffects[idx], "Ring of Regeneration") == 0) regenRingsNumber--;
                        else if (strcmp(ringEffects[idx], "Ring of Sickness")     == 0) sicknessRingsNumber--;
                        else if (strcmp(ringEffects[idx], "Ring of Aggravation")  == 0) aggravateRingsNumber--;
                        else if (strcmp(ringEffects[idx], "Ring of Armor")        == 0) armorRingsNumber--;
                        else if (strcmp(ringEffects[idx], "Ring of Indigestion")  == 0) indigestionRingsNumber--;
                        else if (strcmp(ringEffects[idx], "Ring of Teleport")     == 0) teleportRingsNumber--;
                        else if (strcmp(ringEffects[idx], "Ring of Invisibility") == 0) invisibleRingsNumber--;
                    }
                    if (sel.category == WeaponCategory &&
                        equippedWeapon.item == sel.item &&
                        strcmp(equippedWeapon.name, sel.name) == 0) {
                        equippedWeapon = {};
                        equippedWeapon.weapon.type = NoWeapon;
                        playerAttackDamage = PLAYER_BASE_ATTACK;
                        attackDelayFrames  = DEFAULT_ATTACK_DELAY;
                    }
                    playRawSFX(2);
                    currentUIState = UI_INVENTORY;
                }
            } else {
                // Equip
                if (sel.effectType == ArmorEffect && equippedArmor.item != Null) {
                    snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                             "%s", "You need to unequip your current armor first.");
                    currentUIState = UI_ITEM_RESULT;
                    g_state.buttons.bPressedPrev = true; return;
                }
                if (sel.category == WeaponCategory) {
                    if (equippedWeapon.weapon.type != NoWeapon) {
                        snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                                 "%s", "You need to unequip your current weapon first.");
                        currentUIState = UI_ITEM_RESULT;
                        g_state.buttons.bPressedPrev = true; return;
                    }
                    for (int p = 0; p < numInventoryPages; p++) {
                        if (inventoryPages[p].category != WeaponCategory) continue;
                        for (int i = 0; i < INVENTORY_SIZE; i++)
                            if (inventoryPages[p].items[i].item != Null)
                                inventoryPages[p].items[i].isEquipped = false;
                    }
                    sel.isEquipped    = true;
                    equippedWeapon    = sel;
                    playerAttackDamage= (int)equippedWeapon.weapon.damage;
                    attackDelayFrames = equippedWeapon.weapon.attackDelay;
                } else {
                    sel.isEquipped = true;
                    if (sel.effectType == ArmorEffect) {
                        equippedArmorValue = sel.armorValue;
                        equippedArmor      = sel;
                    }
                }
                if (sel.item == Ring) {
                    if (sel.ringEffectIndex < 0 || sel.ringEffectIndex >= NUM_RINGS) {
                        sel.ringEffectIndex = random(0, NUM_RINGS);
                        sel.isCursed        = ringCursed[sel.ringEffectIndex];
                    }
                    int idx = sel.ringEffectIndex;
                    if      (strcmp(ringEffects[idx], "Ring of Swiftness")    == 0) swiftnessRingsNumber++;
                    else if (strcmp(ringEffects[idx], "Ring of Strength")     == 0) strengthRingsNumber++;
                    else if (strcmp(ringEffects[idx], "Ring of Weakness")     == 0) weaknessRingsNumber++;
                    else if (strcmp(ringEffects[idx], "Ring of Hunger")       == 0) hungerRingsNumber++;
                    else if (strcmp(ringEffects[idx], "Ring of Regeneration") == 0) regenRingsNumber++;
                    else if (strcmp(ringEffects[idx], "Ring of Sickness")     == 0) { sicknessRingsNumber++; playerHP = playerMaxHP - sicknessRingsNumber * 20; }
                    else if (strcmp(ringEffects[idx], "Ring of Aggravation")  == 0) aggravateRingsNumber++;
                    else if (strcmp(ringEffects[idx], "Ring of Armor")        == 0) armorRingsNumber++;
                    else if (strcmp(ringEffects[idx], "Ring of Indigestion")  == 0) indigestionRingsNumber++;
                    else if (strcmp(ringEffects[idx], "Ring of Teleport")     == 0) teleportRingsNumber++;
                    else if (strcmp(ringEffects[idx], "Ring of Invisibility") == 0) invisibleRingsNumber++;
                }
                playRawSFX(2);
                snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                         strcmp(sel.itemResult, "Solve this riddle!") == 0
                         ? "%s" : "You equip the %s.",
                         strcmp(sel.itemResult, "Solve this riddle!") == 0
                         ? "You equip the riddle stone." : sel.name);
                if (strcmp(sel.itemResult, "Solve this riddle!") == 0) equippedRiddleStone = true;
                currentUIState = UI_ITEM_RESULT;
            }
        } else {
            snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                     "%s", "This item cannot be equipped.");
            currentUIState = UI_ITEM_RESULT;
        }
        g_state.buttons.bPressedPrev = true;
        return;
    }

    // ── Combine ───────────────────────────────────────────────────────────
    if (selectedActionIndex == 4) {
        combiningTwoItems = true;
        combiningItem1    = sel;
        ingredient1index  = selectedInventoryIndex;
        currentUIState    = UI_INVENTORY;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// renderInventory
// ─────────────────────────────────────────────────────────────────────────────
static bool showTooltip = true;

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
        display.fillRect(0, 19, SCREEN_WIDTH, 9, 15);
        char pageName[40];
        snprintf(pageName, sizeof(pageName), "<%s>",
                 inventoryPages[currentInventoryPageIndex].name);
        display.println(pageName);
        display.setTextColor(15, 0);

        int yPos = 30;
        InventoryPage& page = inventoryPages[currentInventoryPageIndex];
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            GameItem& item = page.items[i];
            if (item.item == Null) continue;
            display.setCursor(15, yPos);
            if (i == selectedInventoryIndex) display.print("> ");

            const char* displayName = item.name;
            if (combiningTwoItems) {
                GameItem result = CombineTwoItemsToGetItem(combiningItem1, item);
                if (result.item != Null && i == selectedInventoryIndex) {
                    displayName = result.name;
                    display.print(">>");
                }
            }
            display.print(displayName);

            if (item.stackCount > 1) {
                char stk[6];
                snprintf(stk, sizeof(stk), "x%d", item.stackCount);
                int bx = SCREEN_WIDTH - (int)(strlen(stk) * 6) - (item.isEquipped ? 14 : 2);
                display.setCursor(bx, yPos);
                display.print(stk);
            }
            if (item.isEquipped) { display.setCursor(110, yPos); display.print("*"); }
            yPos += 12;
        }

        if (yPos == 30) { display.setCursor(15, 30); display.println("Empty"); }
        if (showTooltip) { display.setCursor(0, 120); display.print("* = Equipped"); }

    } else if (currentUIState == UI_ITEM_INFO) {
        display.setCursor(0, 120);
        display.println(inventoryPages[currentInventoryPageIndex]
                         .items[selectedInventoryIndex].originalName);
        display.setCursor(0, 10);
        display.print(inventoryPages[currentInventoryPageIndex]
                       .items[selectedInventoryIndex].description);

    } else if (currentUIState == UI_ITEM_RESULT) {
        display.setCursor(0, 10);
        display.println(itemResultMessage);
        if (g_state.buttons.bPressed && !g_state.buttons.bPressedPrev)
            currentUIState = UI_NORMAL;
        showTooltip = false;

    } else if (currentUIState == UI_ITEM_ACTION) {
        display.drawRect(50, 40, 65, 75, 15);
        display.fillRect(50, 40, 65, 12, 15);
        display.setTextColor(0, 15);
        display.setCursor(55, 42);
        display.println("Options:");
        display.setTextColor(15, 0);

        GameItem& sel = inventoryPages[currentInventoryPageIndex].items[selectedInventoryIndex];

        const char* equipText = (sel.isEquipped &&
            (sel.category == EquipmentCategory || sel.category == WeaponCategory))
            ? "Unequip" : "Equip";

        const char* useText = "Use";
        if (sel.item == Scroll) useText = "Read";
        else if (sel.category == PotionCategory && sel.item != EmptyBottle) useText = "Drink";
        else if (sel.category == FoodCategory   && sel.item != EmptyBottle) useText = "Eat";

        char dropLabel[12];
        snprintf(dropLabel, sizeof(dropLabel), sel.stackCount > 1 ? "Drop 1" : "Drop");

        char use[20], drop[20], equip[20];
        snprintf(use,   sizeof(use),   "%s %s", selectedActionIndex==0 ? ">" : "", useText);
        snprintf(drop,  sizeof(drop),  "%s %s", selectedActionIndex==1 ? ">" : "", dropLabel);
        snprintf(equip, sizeof(equip), "%s %s", selectedActionIndex==3 ? ">" : "", equipText);

        display.setCursor(55, 60);  display.println(use);
        display.setCursor(55, 70);  display.println(drop);
        display.setCursor(55, 80);  display.println(selectedActionIndex==2 ? "> Info"     : " Info");
        display.setCursor(55, 90);  display.println(equip);
        display.setCursor(55, 100); display.println(selectedActionIndex==4 ? "> Combine"  : " Combine");
    }

    display.display();
}

// ─────────────────────────────────────────────────────────────────────────────
// removeItemFromInventory
// ─────────────────────────────────────────────────────────────────────────────
void removeItemFromInventory(int page, int index) {
    InventoryPage& inv = inventoryPages[page];
    for (int i = index; i < INVENTORY_SIZE - 1; i++)
        inv.items[i] = inv.items[i + 1];
    GameItem nullItem = getItem(Null);
    nullItem.stackCount = 0;
    inv.items[INVENTORY_SIZE - 1] = nullItem;
    inv.itemCount--;
}