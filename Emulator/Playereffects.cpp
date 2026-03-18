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
#include "SaveLogic.h"
#include "Player.h"

#include <cstring>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// PlayerEffects.cpp
//
// Per-tick effects that are NOT direct input responses:
//   • Hunger / starvation
//   • Timed status effects (confusion, speed, blindness, etc.)
//   • Ring passive effects (regen, teleport)
//   • Chaos armour fluctuation
//   • Succubus pull
//   • Pause screen
//   • Carry-damsel progress bar
//   • Chest / puzzle gate (OpenChest / finishPendingChest)
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// startCarryingDamsel  (called from handleInput, lives here as it's an effect)
// ─────────────────────────────────────────────────────────────────────────────
static int carryingDelay = 0;

void startCarryingDamsel(bool resetPickupTimer) {
    if (resetPickupTimer) {
        carryingDelay = 0;
        return;
    }
    carryingDelay++;
    // Draw a loading bar across the top of the screen
    display.fillRect(0, 0, carryingDelay, 15,
                     (int)(carryingDelay / 8));
    display.display();

    if (carryingDelay >= SCREEN_WIDTH) {
        damsel[0].beingCarried = !damsel[0].beingCarried;

        if (damsel[0].beingCarried) {
            currentDamselPortrait = damselPortraitCarrying;
            dialogueTimeLength = 300;
            snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                     "%s", "Oh! Thanks...");
            showDialogue = true;
            playRawSFX(20);
            playerSprite = (playerSprite == playerSpriteRight)
                           ? playerCarryingDamselSpriteRight
                           : playerCarryingDamselSpriteLeft;
        } else {
            playRawSFX(15);
            playerSprite = (playerSprite == playerCarryingDamselSpriteRight)
                           ? playerSpriteRight
                           : playerSpriteLeft;
        }
        carryingDelay = 0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// handleHungerAndEffects
// ─────────────────────────────────────────────────────────────────────────────
static int hungerTick       = 0;
static int damselHealDelay  = 0;

void handleHungerAndEffects() {
    // ── Hunger tick ───────────────────────────────────────────────────────
    hungerTick += (playerMoving || damsel[0].beingCarried) ? 2 : 1;

    int tickThreshold = starving ? HUNGER_TICK_STARVING : HUNGER_TICK_NORMAL;
    if (hungerTick >= tickThreshold) {
        if (starving) {
            playerHP -= HUNGER_STARVE_DAMAGE;
        } else {
            int drain = HUNGER_BASE_DRAIN
                      + (hungerRingsNumber    * 7)
                      + (regenRingsNumber     * 3)
                      - (indigestionRingsNumber * 2);
            if (drain < 1) drain = 1;
            playerFood -= drain;
        }
        hungerTick = 0;
        starving   = (playerFood <= 0);
    }
    checkIfDeadFrom("hunger");

    // ── Confusion ─────────────────────────────────────────────────────────
    if (confused) {
        if (--confusionTimer <= 0) {
            confusionTimer = CONFUSION_TIMER_DEFAULT;
            confused       = false;
        }
    }

    // ── Speed ─────────────────────────────────────────────────────────────
    if (speeding) {
        if (--speedTimer <= 0) {
            speedTimer              = SPEED_TIMER_DEFAULT;
            speeding                = false;
            currentSpeedMultiplier -= lastPotionSpeedModifier;
            lastPotionSpeedModifier = 0.0f;
        }
    }

    // ── See-all ───────────────────────────────────────────────────────────
    if (seeAll) {
        if (--seeAllTimer <= 0) {
            seeAllTimer = SEE_ALL_TIMER_DEFAULT;
            seeAll      = false;
        }
    }

    // ── Damsel carry heal ─────────────────────────────────────────────────
    if (damsel[0].beingCarried) {
        if (++damselHealDelay >= DAMSEL_HEAL_DELAY) {
            damselHealDelay = 0;
            playerHP += damsel[0].levelOfLove;
            if (playerHP > playerMaxHP) playerHP = playerMaxHP;
        }
    }

    handleRingEffects();

    // ── Ridicule ──────────────────────────────────────────────────────────
    if (ridiculed) {
        if (--ridiculeTimer <= 0) {
            ridiculeTimer = RIDICULE_DURATION;
            ridiculed     = false;
        }
    }

    // ── Glamour ───────────────────────────────────────────────────────────
    if (glamoured) {
        if (--glamourTimer <= 0) {
            glamourTimer = GLAMOUR_TIMER_DEFAULT;
            glamoured    = false;
        }
    }

    // ── Blindness ─────────────────────────────────────────────────────────
    if (blinded) {
        if (--blindnessTimer <= 0) {
            blindnessTimer = BLINDNESS_TIMER_DEFAULT;
            blinded        = false;
        }
    }

    // ── Paralysis ─────────────────────────────────────────────────────────
    if (paralyzed) {
        if (--paralysisTimer <= 0) {
            paralysisTimer = PARALYSIS_TIMER_DEFAULT;
            paralyzed      = false;
        }
    }

    // ── Chaos armour ──────────────────────────────────────────────────────
    if (equippedArmor.item == ChaosArmor) {
        equippedArmorValue = (float)random(0, 20 + (int)equippedArmor.armorValue);
    }

    // ── Succubus pull ─────────────────────────────────────────────────────
    if (!damsel[0].beingCarried && !succubusIsFriend) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].hp <= 0) continue;
            if (strcmp(enemies[i].name, "succubus") != 0) continue;
            if (!enemies[i].chasingPlayer) continue;

            float sdx  = enemies[i].x - playerX;
            float sdy  = enemies[i].y - playerY;
            float sdSq = sdx * sdx + sdy * sdy;

            bool damselProtects = damsel[0].levelOfLove >= 6 &&
                                  damsel[0].followingPlayer &&
                                  !damsel[0].dead &&
                                  damsel[0].active;

            if (!damselProtects && sdSq < SUCCUBUS_PULL_RANGE_SQ) {
                nearSuccubus = true;

                snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                         "%s", "Hey there, handsome...");
                damsel[0].followingPlayer = false;
                currentDamselPortrait = succubusPortrait;
                showDialogue          = true;

                if (dialogueTimeLength != SUCCUBUS_DIALOGUE_TIMER) {
                    playRawSFX3D(24, enemies[i].x, enemies[i].y);
                }
                dialogueTimeLength = SUCCUBUS_DIALOGUE_TIMER;

                float sd = sqrtf(sdSq);
                float nx = sdx / sd;
                float ny = sdy / sd;
                float newPX = playerX + nx * SUCCUBUS_PULL_STRENGTH;
                float newPY = playerY + ny * SUCCUBUS_PULL_STRENGTH;

                // Wall check before applying pull
                int rPX = round(newPX), rPY = round(newPY);
                TileTypes t = dungeonMap[rPY][rPX];
                bool walkable = (t == Floor || t == Exit || t == StartStairs ||
                                 t == DoorOpen || t == KeyItem || t == KeyTile);
                if (walkable) {
                    playerX = newPX;
                    playerY = newPY;
                }
            } else if (damselProtects) {
                nearSuccubus              = false;
                damsel[0].beingCarried    = true;
                currentDamselPortrait     = damselPortraitCarrying;
                dialogueTimeLength        = 400;
                snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                         "%s", "Stay away from her- you're mine!");
                showDialogue = true;
                playRawSFX(21);
                playerSprite = (playerSprite == playerSpriteRight)
                               ? playerCarryingDamselSpriteRight
                               : playerCarryingDamselSpriteLeft;
            } else {
                nearSuccubus = false;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// handleRingEffects
// ─────────────────────────────────────────────────────────────────────────────
void handleRingEffects() {
    static int regenCounter    = 0;
    static int teleportCounter = 0;

    if (regenRingsNumber > 0) {
        regenCounter += regenRingsNumber;
        if (regenCounter >= REGEN_TICK_THRESHOLD) {
            if (playerHP < playerMaxHP) playerHP++;
            regenCounter = 0;
        }
    } else {
        regenCounter = 0;
    }

    if (teleportRingsNumber > 0) {
        teleportCounter += teleportRingsNumber;
        if (teleportCounter >= TELEPORT_TICK_THRESHOLD) {
            playRawSFX(14);
            int newX, newY;
            do {
                newX = random(0, mapWidth);
                newY = random(0, mapHeight);
            } while (dungeonMap[newY][newX] != Floor);
            playerX = (float)newX;
            playerY = (float)newY;
            teleportCounter = 0;
        }
    } else {
        teleportCounter = 0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// handlePauseScreen
// ─────────────────────────────────────────────────────────────────────────────
void handlePauseScreen() {
    static int pauseSelection = 0;   // 0=Volume 1=Restart 2=Save 3=Load
    const ButtonStates& b = g_state.buttons;

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(27, 16);
    display.setTextColor(15, 0);
    display.print("PAUSED");
    display.setTextSize(1);

    // Helper: draw one menu row with highlight
    auto drawRow = [&](int baseY, int idx, const char* label) {
        if (pauseSelection == idx) {
            display.fillRect(10, baseY - 2, 108, 12, 15);
            display.setTextColor(0, 15);
        } else {
            display.setTextColor(15, 0);
        }
        display.setCursor(18, baseY);
        display.print(label);
    };

    // Volume row (with value)
    int baseY = 46;
    if (pauseSelection == 0) {
        display.fillRect(10, baseY - 2, 108, 12, 15);
        display.setTextColor(0, 15);
    } else {
        display.setTextColor(15, 0);
    }
    display.setCursor(18, baseY);
    display.print("Volume:");
    char volStr[10];
    snprintf(volStr, sizeof(volStr), "<%d>", masterVolume);
    display.setCursor(80, baseY);
    display.print(volStr);

    drawRow(baseY + 14, 1, "Restart Game");
    drawRow(baseY + 28, 2, "Save game");
    drawRow(baseY + 42, 3, "Load game");

    display.setTextColor(15, 0);
    display.setCursor(24, 110);
    display.print("Press [ENTER]");
    display.display();

    // ── Navigation ────────────────────────────────────────────────────────
    if (b.upPressed   && !b.upPressedPrev)   { pauseSelection = (pauseSelection + 3) % 4; playRawSFX(8); }
    if (b.downPressed && !b.downPressedPrev) { pauseSelection = (pauseSelection + 1) % 4; playRawSFX(8); }

    // Volume adjustment
    if (pauseSelection == 0) {
        if (b.leftPressed && !b.leftPressedPrev) {
            masterVolume = max(MASTER_VOLUME_MIN, masterVolume - 1);
            float volf = masterVolume / 10.0f;
            sgtl5000_1.volume(volf);
            mixer1.gain(0, 0.5f * volf);
            mixer1.gain(1, 0.5f * volf);
            mixer1.gain(2, 0.5f * volf);
            mixer1.gain(3, 0.5f * volf);
            musicMixer.gain(0, volf);
            musicMixer.gain(1, 0.2f * volf);
            playRawSFX(7);
        }
        if (b.rightPressed && !b.rightPressedPrev) {
            masterVolume = min(MASTER_VOLUME_MAX, masterVolume + 1);
            float volf = masterVolume / 10.0f;
            sgtl5000_1.volume(volf);
            mixer1.gain(0, 0.5f * volf);
            mixer1.gain(1, 0.5f * volf);
            mixer1.gain(2, 0.5f * volf);
            mixer1.gain(3, 0.5f * volf);
            musicMixer.gain(0, volf);
            musicMixer.gain(1, 0.2f * volf);
            playRawSFX(7);
        }
    }

    // Actions
    if (b.bPressed && !b.bPressedPrev) {
        switch (pauseSelection) {
            case 1:
                playRawSFX(9);
                currentUIState = UI_SPLASH;
                break;
            case 2:
                trySaveGame();
                break;
            case 3: {
                tryLoadGame();
                int rPx = round(playerX);
                int rPy = round(playerY);
                if (rPy >= 0 && rPy < mapHeight && rPx >= 0 && rPx < mapWidth) {
                    dungeonMap[rPy][rPx] = Floor;
                }
                break;
            }
            default: break;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// OpenChest  (starts the puzzle gate)
// ─────────────────────────────────────────────────────────────────────────────
void OpenChest(int cy, int cx, int /*dx_unused*/) {
    if (pendingChestActive) return;
    pendingChestActive = true;
    pendingChestX      = cx;
    pendingChestY      = cy;

    if (random(0, 2) == 0) {
        startPicrossPuzzle();
    } else {
        startLightsOutPuzzle();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// finishPendingChest  (called when puzzle resolves)
// ─────────────────────────────────────────────────────────────────────────────
void finishPendingChest(bool success) {
    if (!pendingChestActive) return;
    int cx = pendingChestX;
    int cy = pendingChestY;
    pendingChestActive = false;
    pendingChestX      = -1;
    pendingChestY      = -1;

    if (!success) {
        playRawSFX(13);
        return;
    }

    playRawSFX(3);
    if (cy < 0 || cy >= mapHeight || cx < 0 || cx >= mapWidth) return;

    dungeonMap[cy][cx] = Floor;

    for (int ldx = -1; ldx <= 1; ldx++) {
        for (int ldy = -1; ldy <= 1; ldy++) {
            int lx = cx + ldx;
            int ly = cy + ldy;
            if (lx < 0 || lx >= mapWidth || ly < 0 || ly >= mapHeight) continue;
            if (dungeonMap[ly][lx] != Floor) continue;
            if (random(0, 100) < 80) continue;   // ~20 % chance per tile

            TileTypes loot = getRandomLootTile(5);
            // Safety: only place item tiles
            bool isItemTile = (loot == Potion     || loot == Map        ||
                               loot == MushroomTile || loot == RiddleStoneTile ||
                               loot == ArmorTile  || loot == ScrollTile  ||
                               loot == RingTile   || loot == WeaponTile  ||
                               loot == GoldTile);
            dungeonMap[ly][lx] = isItemTile ? loot : Potion;
        }
    }
}