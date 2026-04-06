#include "Common.h"
#include "GameState.h"
#include "Sprites.h"
#include "Dungeon.h"
#include "Entities.h"
#include "HelperFunctions.h"
#include "Item.h"
#include "Inventory.h"
#include "GameAudio.h"
#include "Puzzles.h"
#include "Player.h"

#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// UIHelpers.cpp
//
// Everything that touches the display but belongs to no single game system:
//   • updateButtonStates      — reads SDL keyboard → ButtonStates
//   • handleUIStateTransitions — routes A/Start presses between UI modes
//   • renderUI                — the bottom HUD bar
//   • updateAnimations        — advances enemy + damsel sprite frames
//   • drawWrappedText         — wraps a string to a pixel width
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// updateButtonStates
// ─────────────────────────────────────────────────────────────────────────────
void updateButtonStates() {
    ButtonStates& b = g_state.buttons;

    b.upPressedPrev    = b.upPressed;
    b.downPressedPrev  = b.downPressed;
    b.aPressedPrev     = b.aPressed;
    b.bPressedPrev     = b.bPressed;
    b.leftPressedPrev  = b.leftPressed;
    b.rightPressedPrev = b.rightPressed;
    b.startPressedPrev = b.startPressed;

    const Uint8* ks = SDL_GetKeyboardState(NULL);
    b.upPressed    = ks[SDL_SCANCODE_UP]    || ks[SDL_SCANCODE_W];
    b.downPressed  = ks[SDL_SCANCODE_DOWN]  || ks[SDL_SCANCODE_S];
    b.leftPressed  = ks[SDL_SCANCODE_LEFT]  || ks[SDL_SCANCODE_A];
    b.rightPressed = ks[SDL_SCANCODE_RIGHT] || ks[SDL_SCANCODE_D];
    b.aPressed     = ks[SDL_SCANCODE_Z]     || ks[SDL_SCANCODE_BACKSPACE] ||
                     ks[SDL_SCANCODE_I]     || ks[SDL_SCANCODE_E] ||
                     ks[SDL_SCANCODE_LSHIFT];
    b.bPressed     = ks[SDL_SCANCODE_X]     || ks[SDL_SCANCODE_LCTRL] ||
                     ks[SDL_SCANCODE_TAB]   || ks[SDL_SCANCODE_SPACE];
    b.startPressed = ks[SDL_SCANCODE_RETURN]|| ks[SDL_SCANCODE_P] ||
                     ks[SDL_SCANCODE_ESCAPE];
}

// ─────────────────────────────────────────────────────────────────────────────
// handleUIStateTransitions
// ─────────────────────────────────────────────────────────────────────────────
void handleUIStateTransitions() {
    // Puzzles handle their own input — don't intercept
    if (currentUIState == UI_PICROSS || currentUIState == UI_LIGHTSOUT) return;

    const ButtonStates& b = g_state.buttons;

    if (b.aPressed && !b.aPressedPrev) {
        switch (currentUIState) {
            case UI_NORMAL:
                if (!statusScreen) {
                    if (!showDialogue) {
                        currentUIState = UI_INVENTORY;
                        playRawSFX(12);
                    } else {
                        showDialogue = false;
                    }
                }
                break;
            case UI_INVENTORY:
                // Cancel identify scroll if active (wasting it)
                if (identifyingItem) {
                    if (identifyScrollPage >= 0 && identifyScrollIndex >= 0) {
                        inventoryPages[identifyScrollPage]
                            .items[identifyScrollIndex] = { Null, PotionCategory, "Empty" };
                        inventoryPages[identifyScrollPage].itemCount--;
                    }
                    identifyingItem    = false;
                    identifyScrollPage = -1;
                    identifyScrollIndex= -1;
                }
                if (hasMap) {
                    currentUIState = UI_MINIMAP;
                } else {
                    playRawSFX(13);
                    currentUIState = UI_NORMAL;
                }
                break;
            case UI_MINIMAP:
                playRawSFX(13);
                currentUIState = UI_NORMAL;
                break;
            case UI_ITEM_ACTION:
                playRawSFX(12);
                currentUIState = UI_INVENTORY;
                break;
            case UI_ITEM_INFO:
                playRawSFX(12);
                currentUIState = UI_INVENTORY;
                break;
            case UI_ITEM_RESULT:
                playRawSFX(12);
                currentUIState = UI_NORMAL;
                break;
            case UI_SECRET:
                currentUIState = UI_SPLASH;
                break;
            case UI_SHOP:
                playRawSFX(13);
                currentUIState = UI_NORMAL;
                break;
            // These states handle their own A-press:
            case UI_PAUSE:
            case UI_RIDDLE:
            case UI_SPLASH:
            case UI_INTRO:
            case UI_PICROSS:
            case UI_LIGHTSOUT:
                break;
        }
    }

    if (b.startPressed && !b.startPressedPrev) {
        playRawSFX(9);
        if (currentUIState == UI_SPLASH) {
            playWav1.stop();
            shouldRestartGame = true;
        }
        // Toggle pause from normal; dismiss pause back to normal
        if      (currentUIState == UI_NORMAL) currentUIState = UI_PAUSE;
        else if (currentUIState == UI_PAUSE)  currentUIState = UI_NORMAL;
        else if (currentUIState == UI_SPLASH) currentUIState = UI_NORMAL;
    }

    // Resolve pending chest if puzzle just finished
    if (puzzleFinished && pendingChestActive) {
        finishPendingChest(puzzleSuccess);
        puzzleFinished = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// renderUI  (the bottom HUD bar)
// ─────────────────────────────────────────────────────────────────────────────
void renderUI() {
    static int blinkTick = 0;
    static int textColor = 15;

    char HP[4], FOOD[7];
    snprintf(HP,   sizeof(HP),   "%d", playerHP);
    snprintf(FOOD, sizeof(FOOD), "%d", playerFood);

    // Background bar
    display.fillRect(0, UI_BOTTOM_BAR_Y, SCREEN_WIDTH, UI_BOTTOM_BAR_H, 1);
    // Reload indicator stripe
    display.fillRect(0, UI_BOTTOM_BAR_Y - 1, reloadBarWidth, 1, 15);
    display.drawRect(0, UI_BOTTOM_BAR_Y, SCREEN_WIDTH, UI_BOTTOM_BAR_H, SSD1327_WHITE);

    // HP
    display.setTextColor(15, 1);
    display.setTextSize(1);
    display.setCursor(5, UI_TEXT_Y);
    display.print("HP:");
    display.setCursor(21, UI_TEXT_Y);
    display.print(HP);

    // Food — blinks red when starving
    if (starving) {
        blinkTick++;
        if (blinkTick >= 35) {
            textColor = (textColor == 15) ? 3 : 15;
            blinkTick = 0;
        }
    } else {
        textColor = 15;
    }
    display.setTextColor(textColor, 1);
    display.setCursor(46, UI_TEXT_Y);
    display.print("FOOD:");
    display.setCursor(74, UI_TEXT_Y);
    display.print(FOOD);

    // Status icons
    display.setTextColor(15, 1);
    if (hasMap)
        display.drawBitmap(100, UI_TEXT_Y - 1, mapSprite,       8, 8, SSD1327_WHITE);
    if (speeding || paralyzed)
        display.drawBitmap(109, UI_TEXT_Y - 1, fastbootSprite,  8, 8, SSD1327_WHITE);
    if (seeAll || blinded)
        display.drawBitmap(118, UI_TEXT_Y - 1, eyeSprite,       8, 8, SSD1327_WHITE);
    if (confused)
        display.drawBitmap( 91, UI_TEXT_Y - 1, confusionSprite, 8, 8, SSD1327_WHITE);
}

// ─────────────────────────────────────────────────────────────────────────────
// updateAnimations
// ─────────────────────────────────────────────────────────────────────────────
void updateAnimations() {
    // Damsel sprite flip
    static int damselAnimCounter = 0;
    damselAnimCounter++;
    if (damselAnimCounter >= random(50, 90)) {
        if (damsel[0].dead) damselSprite = damselSpriteDead;
        damselAnimCounter = 0;
    }

    // Enemy animation
    static int frameIndex[MAX_ENEMIES] = {};
    static int frameTimer[MAX_ENEMIES] = {};

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy& e = enemies[i];
        const Frame* anim  = nullptr;
        int          animLen = 1;

        if      (strcmp(e.name, "blob")      == 0) { anim = blobAnimation;       animLen = blobAnimationLength;       }
        else if (strcmp(e.name, "teleporter")== 0) { anim = teleporterAnimation; animLen = teleporterAnimationLength; }
        else if (strcmp(e.name, "batguy")    == 0) { anim = batguyAnimation;     animLen = batguyAnimationLength;     }
        else if (strcmp(e.name, "shooter")   == 0) { anim = shooterAnimation;    animLen = shooterAnimationLength;    }
        else if (strcmp(e.name, "clock")     == 0) { anim = clockAnimation;      animLen = clockAnimationLength;      }
        else if (strcmp(e.name, "jukebox")   == 0) { anim = jukeboxAnimation;    animLen = jukeboxAnimationLength;    }
        else if (strcmp(e.name, "fairy")   == 0) { anim = fairyAnimation;    animLen = fairyAnimationLength;    }
        else if (strcmp(e.name, "boss")      == 0) {
            switch (bossState) {
                case Idle:
                    anim    = (playerX < enemies[0].x) ? bossIdleAnimationFlipped : bossIdleAnimation;
                    animLen = bossIdleAnimationLength;
                    break;
                case Shooting:
                    anim    = (playerX < enemies[0].x) ? bossIdleAnimationFlipped : bossIdleAnimation;
                    animLen = bossIdleAnimationLength;
                    break;
                case Floating:
                case Summoning:
                case Enraged:
                    anim    = bossFightAnimation;
                    animLen = bossFightAnimationLength;
                    break;
                case Beaten:
                    anim    = bossBeatenAnimation;
                    animLen = bossBeatenAnimationLength;
                    break;
            }
        }

        if (!anim) continue;

        frameTimer[i]++;
        if (frameTimer[i] >= anim[frameIndex[i]].length) {
            bool isBeaten = (anim == bossBeatenAnimation);
            frameTimer[i] = 0;
            if (!isBeaten) {
                frameIndex[i] = (frameIndex[i] + 1) % animLen;
            } else if (frameIndex[i] < animLen - 1) {
                frameIndex[i]++;
            }
            e.sprite = anim[frameIndex[i]].frame;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// drawWrappedText
// ─────────────────────────────────────────────────────────────────────────────
void drawWrappedText(int x, int y, int maxWidth, const char* text) {
    const int lineHeight = 10;
    int   cursorY        = y;
    char  currentLine[300] = "";
    char  word[100]        = "";
    size_t wordLen         = 0;
    size_t currentLineLen  = 0;
    size_t textLen         = strlen(text);

    u8g2_for_adafruit_gfx.setCursor(x, y);

    for (size_t i = 0; i <= textLen; i++) {
        char c = (i < textLen) ? text[i] : ' ';   // treat end-of-string as a space

        if (c == ' ' || c == '\n' || i == textLen) {
            // Check if appending word exceeds width
            char test[400];
            snprintf(test, sizeof(test), "%s%s ", currentLine, word);
            int w = (int)(strlen(test) * 6);   // ~6px per char at textSize 1

            if (w > maxWidth) {
                u8g2_for_adafruit_gfx.setCursor(x, cursorY);
                u8g2_for_adafruit_gfx.print(currentLine);
                cursorY += lineHeight;
                snprintf(currentLine, sizeof(currentLine), "%s ", word);
                currentLineLen = strlen(currentLine);
            } else {
                snprintf(currentLine + currentLineLen,
                         sizeof(currentLine) - currentLineLen, "%s ", word);
                currentLineLen = strlen(currentLine);
            }
            word[0]  = '\0';
            wordLen  = 0;

            if (c == '\n') {
                u8g2_for_adafruit_gfx.setCursor(x, cursorY);
                u8g2_for_adafruit_gfx.print(currentLine);
                cursorY += lineHeight;
                currentLine[0]  = '\0';
                currentLineLen  = 0;
            }
        } else {
            if (wordLen < sizeof(word) - 1) {
                word[wordLen++] = c;
                word[wordLen]   = '\0';
            }
        }
    }

    // Flush remainder
    if (currentLineLen > 0 || wordLen > 0) {
        char final[400];
        snprintf(final, sizeof(final), "%s%s", currentLine, word);
        u8g2_for_adafruit_gfx.setCursor(x, cursorY);
        u8g2_for_adafruit_gfx.print(final);
    }
}