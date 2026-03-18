#include "Common.h"
#include "GameState.h"
#include "Sprites.h"
#include "Dungeon.h"
#include "Entities.h"
#include "HelperFunctions.h"
#include "Item.h"
#include "Inventory.h"
#include "GameAudio.h"

#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// Dialogue.cpp
//
// Everything related to showing in-game dialogue bubbles and the riddle screen:
//   • handleDialogue()   — draws the bubble, drives damsel conversation,
//                          glamour/ridicule ticker
//   • handleRiddles()    — the riddle answer screen
//   • playDamselSFX()    — thin wrapper that suppresses SFX near succubus
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// playDamselSFX
// ─────────────────────────────────────────────────────────────────────────────
void playDamselSFX(const char* tone) {
    if (nearSuccubus) return;
    if      (strcmp(tone, "normal")   == 0) playRawSFX3D(16, damsel[0].x, damsel[0].y);
    else if (strcmp(tone, "annoying") == 0) playRawSFX3D(21, damsel[0].x, damsel[0].y);
    else if (strcmp(tone, "alone")    == 0) playRawSFX3D(16, damsel[0].x, damsel[0].y);
}

// ─────────────────────────────────────────────────────────────────────────────
// handleDialogue
// ─────────────────────────────────────────────────────────────────────────────
void handleDialogue() {
    static int dialogueTimer      = 0;
    static int dialogueDelayTimer = 0;

    // ── Glamour dialogue ticker ───────────────────────────────────────────
    if (glamoured) {
        static int lastGlamourIndex = -1;
        if (!showDialogue && dialogueDelayTimer <= 0) {
            int len   = sizeof(glamourDialogue) / sizeof(glamourDialogue[0]);
            if (len <= 0) goto draw_bubble;
            int index = random(0, len);
            if (index == lastGlamourIndex && len > 1)
                index = (index + 1) % len;
            lastGlamourIndex   = index;
            dialogueTimeLength = glamourDialogue[index].duration;
            snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                     "%s", glamourDialogue[index].message);
            showDialogue          = true;
            isRidiculeDialogue    = true;
            dialogueDelayTimer    = 50;
        }
    }

    // ── Ridicule dialogue ticker ──────────────────────────────────────────
    if (ridiculed) {
        if (!showDialogue && dialogueDelayTimer <= 0) {
            int len   = sizeof(ridiculeDialogue) / sizeof(ridiculeDialogue[0]);
            if (len <= 0) goto draw_bubble;
            int index = random(0, len);
            if (index == g_state.lastRidiculeIndex && len > 1)
                index = (index + 1) % len;
            g_state.lastRidiculeIndex = index;
            dialogueTimeLength = ridiculeDialogue[index].duration;
            snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                     "%s", ridiculeDialogue[index].message);
            showDialogue       = true;
            isRidiculeDialogue = true;
            dialogueDelayTimer = 50;
        }
    }

    if (dialogueDelayTimer > 0) dialogueDelayTimer--;

    // ── Draw dialogue bubble ─────────────────────────────────────────────
    draw_bubble:
    if (showDialogue) {
        dialogueTimer++;
        if (dialogueTimer >= dialogueTimeLength) {
            dialogueTimer = 0;
            if (dialogueTimeLength == SUCCUBUS_DIALOGUE_TIMER) dialogueTimeLength = 0;
            showDialogue = false;
        }
        u8g2_for_adafruit_gfx.setFont(u8g2_font_profont10_mf);
        display.fillRect(25, 10, 100, 34, 0);
        display.setCursor(28, 13);
        display.print(currentDialogue);
        display.drawRect(25, 10, 100, 34, 15);

        // Portrait (hidden for ridicule/glamour unless it's the "wait up" line)
        if (!isRidiculeDialogue ||
            strcmp(currentDialogue, "Hey! Wait up!") == 0) {
            display.drawBitmap(9, 11, currentDamselPortrait, 16, 32, 15);
            display.drawRect(8, 10, 18, 34, 15);
        }
    }

    // ── Damsel walking dialogue ───────────────────────────────────────────
    if (!damsel[0].followingPlayer || damsel[0].dead || succubusIsFriend) return;

    timeTillNextDialogue--;
    if (timeTillNextDialogue > 0) return;

    // Pick a random unsaid line from a dialogue set
    auto pickDialogue = [](Dialogue set[], int len) -> int {
        int unsaid[20], count = 0;
        for (int i = 0; i < len && count < 20; i++)
            if (!set[i].alreadyBeenSaid) unsaid[count++] = i;
        return (count > 0) ? unsaid[random(0, count)] : -1;
    };

    if (!damsel[0].completelyRescued) {
        if (damsel[0].beingCarried) {
            int len = sizeof(damselCarryDialogue) / sizeof(damselCarryDialogue[0]);
            int idx = pickDialogue(damselCarryDialogue, len);
            if (idx != -1) {
                currentDamselPortrait = damselPortraitCarrying;
                dialogueTimeLength    = damselCarryDialogue[idx].duration;
                isRidiculeDialogue    = false;
                snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                         "%s", damselCarryDialogue[idx].message);
                if (!damselCarryDialogue[idx].alreadyBeenSaid) playRawSFX(18);
                damselCarryDialogue[idx].alreadyBeenSaid = true;
                showDialogue = true;
            }
        } else {
            int love = damsel[0].levelOfLove;

            auto setLine = [&](Dialogue set[], int len, const char* sfxTone) {
                int idx = pickDialogue(set, len);
                if (idx == -1) return;
                if (!set[idx].alreadyBeenSaid) playDamselSFX(set[idx].tone);
                // Choose portrait based on tone
                const char* tone = set[idx].tone;
                if      (strcmp(tone, "annoying") == 0) currentDamselPortrait = damselPortraitScared;
                else if (strcmp(tone, "alone")    == 0) currentDamselPortrait = damselPortraitAlone;
                else                                    currentDamselPortrait = damselPortraitNormal;
                dialogueTimeLength = set[idx].duration;
                isRidiculeDialogue = false;
                snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                         "%s", set[idx].message);
                set[idx].alreadyBeenSaid = true;
                showDialogue = true;
            };

            if (love >= 1 && love < 3) {
                int len = sizeof(damselAnnoyingDialogue) / sizeof(damselAnnoyingDialogue[0]);
                setLine(damselAnnoyingDialogue, len, "annoying");
            } else if (love >= 3 && love < 6) {
                int len = sizeof(damselPassiveDialogue) / sizeof(damselPassiveDialogue[0]);
                setLine(damselPassiveDialogue, len, "normal");
            } else if (love >= 6) {
                if (!knowsDamselName) {
                    dialogueTimeLength = 500;
                    playDamselSFX("normal");
                    snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                             "By the way, my name is %s...", damsel[0].name);
                    knowsDamselName = true;
                    showDialogue    = true;
                } else {
                    int len = sizeof(damselGoodDialogue) / sizeof(damselGoodDialogue[0]);
                    setLine(damselGoodDialogue, len, "normal");
                }
            }
        }
    }

    if (showDialogue) {
        timeTillNextDialogue = random(DAMSEL_DIALOGUE_MIN, DAMSEL_DIALOGUE_MAX + 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// handleRiddles
// ─────────────────────────────────────────────────────────────────────────────
void handleRiddles() {
    if (!riddleGenerated) generateRiddleUI();

    const ButtonStates& b = g_state.buttons;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1327_WHITE, 0);

    display.setCursor(0, 10);
    display.print("Solve this riddle!");
    display.setCursor(0, 22);
    display.print(g_state.currentRiddle.riddle);

    for (int i = 0; i < 4; i++) {
        int optY = 70 + (i * 12);
        if (i == selectedRiddleOption) {
            display.setTextColor(SSD1327_BLACK, SSD1327_WHITE);
        } else {
            display.setTextColor(SSD1327_WHITE, SSD1327_BLACK);
        }
        display.setCursor(10, optY);
        display.print(g_state.currentRiddle.options[i]);
    }
    display.display();

    // Navigation
    if (b.upPressed   && !b.upPressedPrev)   {
        playRawSFX(8);
        if (selectedRiddleOption > 0) selectedRiddleOption--;
    }
    if (b.downPressed && !b.downPressedPrev) {
        playRawSFX(8);
        if (selectedRiddleOption < 3) selectedRiddleOption++;
    }

    // Confirm answer
    if (b.bPressed && !b.bPressedPrev) {
        playRawSFX(7);
        if (selectedRiddleOption == g_state.currentRiddle.correctOption) {
            playRawSFX(6);
            if (playerHP > 0) {
                snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                         "%s", "Correct! You are rewarded.");
            } else {
                snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                         "%s", "Correct! You are revived.");
                playerHP = playerMaxHP / 2;
            }
            // Three random reward items
            for (int i = 0; i < 3; i++) {
                int cat = random(0, 5);
                GameItem reward;
                if      (cat == 0) { reward = getItem(getRandomPotion(random(0, NUM_POTIONS), false)); }
                else if (cat == 1) { reward = getItem(Scroll); }
                else if (cat == 2) { reward = getItem(Ring);   }
                else if (cat == 3) {
                    GameItems armorTypes[] = { LeatherArmor, IronArmor, MagicRobe, Cloak };
                    reward = getItem(armorTypes[random(0, 4)]);
                } else             { reward = getItem(Mushroom); }
                addToInventory(reward, false);
            }
        } else {
            playRawSFX(13);
            snprintf(itemResultMessage, sizeof(g_state.itemResultMessage),
                     "%s", "Wrong answer! You suffer.");
            playerHP -= 10;
            checkIfDeadFrom("stupidity");
        }

        currentUIState  = UI_ITEM_RESULT;
        riddleGenerated = false;
    }
}