#include "game.h"
#include "Adafruit_SSD1327_emu.h"
#include "Adafruit_GFX_emu.h"
#include "Sprites.h"
#include "Dungeon.h"
#include "HelperFunctions.h"
#include "Entities.h"
#include "Item.h"
#include "Inventory.h"
#include "Player.h"
#include "GameAudio.h"
#include "Puzzles.h"

// Game state variables
bool itemResultScreenActive = false;
unsigned long lastUpdateTime = 0;
const unsigned long frameDelay = 10; // Update every 100ms
int page = 1;
bool leftDamsel = false;

// High score addresses
unsigned int dngnHighscoreAddress = 0;
unsigned int killHighscoreAddress = 1;

void game_setup() {
    display.setFont(Adafruit_GFX::profont10_font);
    // Initialize game state
    //randomSeed(42); // Use a fixed seed for emulator consistency
    
    // Assign a randomly generated name to the damsel
    damsel[0].name = generateFemaleName();

    // Initialize the game
    resetGame();
    playRawSFX(11);
}

void game_loop() {
    //serviceRawSFX(); // uh this might be important but idk

    unsigned long currentTime = SDL_GetTicks();
    if (currentTime - lastUpdateTime >= frameDelay) {
        lastUpdateTime = currentTime;
        updateButtonStates();
        
        if (playerHP > 0) {
            handleUIStateTransitions();
            if (!statusScreen) {
                switch (currentUIState) {
                    case UI_NORMAL:
                        renderGame();
                        updateGame();
                        break;

                    case UI_INVENTORY:
                        renderInventory();
                        handleInventoryNavigation();
                        handleInventoryItemUsage();
                        break;

                    case UI_MINIMAP:
                        drawMinimap();
                        break;

                    case UI_ITEM_ACTION:
                        handleItemActionMenu();
                        renderInventory();
                        break;

                    case UI_ITEM_INFO:
                        renderInventory();
                        break;

                    case UI_ITEM_RESULT:
                        renderInventory();
                        break;

                    case UI_PAUSE:
                        handlePauseScreen();
                        break;

                    case UI_RIDDLE:
                        handleRiddles();
                        break;
                    
                    case UI_LIGHTSOUT_PUZZLE:
                        updateLightsOutPuzzle();
                        break;
                    
                    case UI_PICROSS_PUZZLE:
                        updatePicrossPuzzle();
                        break;
                }
            } else {
                showStatusScreen();
            }
        } else {
            gameOver();
        }
    }
}

void resetGame() {
    // Reset player stats
    playerHP = 100;
    playerFood = 100;
    dungeon = 1;
    levelOfDamselDeath = -4;
    kills = 0;
    
    // Reset damsel
    damsel[0].name = generateFemaleName();
    damsel[0].levelOfLove = 0;
    knowsDamselName = false;
    
    // Reset inventory
    for (int i = 0; i < inventorySize; i++) {
        for (int j = 0; j < numInventoryPages; j++) {
            inventoryPages[j].items[i] = { Null, PotionCategory, "Empty"};
            inventoryPages[j].itemCount = 0;
        }
    }
    
    // Give player a cloak and equip it
    GameItem cloak = getItem(Cloak);
    cloak.isEquipped = true;
    addToInventory(cloak, false);
    equippedArmorValue = cloak.armorValue;
    equippedArmor = cloak;
    
    // Reset projectiles
    for (int i = 0; i < maxProjectiles; i++) {
        projectiles[i].active = false;
    }
    
    // Reset UI and game state
    currentUIState = UI_NORMAL;
    speeding = false;
    currentSpeedMultiplier = 1;
    speedTimer = 500;
    hasMap = false;
    equippedRiddleStone = false;
    starving = false;
    seeAll = false;
    confused = false;
    ridiculed = false;
    glamoured = false;
    blinded = false;
    showDialogue = false;
    
    // Reset damsel state
    damselWasFollowing = false;
    damselWaitUpTimer = 0;
    damselSaidWaitUp = false;
    
    // Reset potion effects
    resetPotionNames();
    randomizePotionEffects();
    randomizeScrollEffects();
    randomizeRingEffects();

    // --- Explicitly reset all ring and speed effect flags ---
    ringOfSwiftnessActive = false;
    ringOfStrengthActive = false;
    ringOfWeaknessActive = false;
    ringOfHungerActive = false;
    ringOfRegenActive = false;
    lastPotionSpeedModifier = 0;
    playerAttackDamage = 10;

    // Generate new dungeon and spawn enemies
    generateDungeon();
    spawnEnemies();
}

void updateGame() {
    updateScreenShake();
    handleInput();
    
    // Only update game state if the player has taken an action
    if (playerActed) {
        handleHungerAndEffects();
        updateScrolling(viewportWidth, viewportHeight, scrollSpeed, offsetX, offsetY);
        updateDamsel();
        updateEnemies();
        updateProjectiles();
    }
}

void renderGame() {
    display.clearDisplay();
    if (!blinded) {
        renderDungeon();
        renderDamsel();
        renderEnemies();
        renderProjectiles();
    }
    renderPlayer();
    renderUI();
    handleDialogue();
    if (playerActed) {
        updateAnimations();
    }
    display.display();
}

void gameOver() {
    if (buttons.aPressed && !buttons.aPressedPrev) {
        playRawSFX(8);
        page++;
        if (page == 3) {
            page = 1;
        }
    }

    char Dngn[7];
    snprintf(Dngn, sizeof(Dngn), "%d", dungeon);
    char KLLS[7];
    snprintf(KLLS, sizeof(KLLS), "%d", kills);

    // In emulator, we'll just use variables instead of EEPROM
    static int dngnHighscore = 0;
    static int kllHighscore = 0;
    
    if (dungeon > dngnHighscore) {
        dngnHighscore = dungeon;
    }
    if (kills > kllHighscore) {
        kllHighscore = kills;
    }

    char DHighscore[7];
    snprintf(DHighscore, sizeof(DHighscore), "%d", dngnHighscore);
    char KHighscore[7];
    snprintf(KHighscore, sizeof(KHighscore), "%d", kllHighscore);

    display.clearDisplay();

    display.setCursor(7, 10);
    display.setTextSize(2);
    display.print("Game over!");
    display.setTextSize(1);
    display.setCursor(5, 30);
    display.print("Press [B] to restart");

    display.drawRect(8, 41, 110, 72, 15);

    if (page == 1) {
        display.setCursor(12, 44);
        display.print("Slain by:");
        display.setCursor(66, 44);
        display.print(deathCause.c_str());

        display.setCursor(12, 56);
        display.print("On dungeon:");
        display.setCursor(78, 56);
        display.print(Dngn);

        display.setCursor(12, 68);
        display.print("Dngn highscore:");
        display.setCursor(102, 68);
        display.print(DHighscore);

        display.setCursor(12, 80);
        display.print("Kills:");
        display.setCursor(48, 80);
        display.print(KLLS);

        display.setCursor(12, 92);
        display.print("Kll Highscore:");
        display.setCursor(96, 92);
        display.print(KHighscore);

        display.setCursor(22, 102);
        display.print("[A] next page");
    } else if (page == 2) {
        display.setCursor(12, 42);
        display.print("next page");
        display.setCursor(22, 102);
        display.print("[A] next page");
    }

    display.display();

    if (buttons.bPressed && !buttons.bPressedPrev) {
        resetGame();
    }
}

void showStatusScreen() {
    static bool damselKidnapScreen = false; // Tracks if we are showing the kidnap screen

    display.clearDisplay();

    //display.setFont(Adafruit_GFX::profont10_font);

    if (!damselKidnapScreen) {
        if (dungeon > levelOfDamselDeath + 3) {
            if (!damsel[0].dead && damsel[0].followingPlayer) {
                if (!carryingDamsel) {
                    display.drawBitmap(0, -10, rescueDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
                } else {
                    display.drawBitmap(0, -10, carryDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
                }
                display.setCursor(0, 115);
                display.print("You rescued the Damsel!");
            } else {
                display.setCursor(0, 115);
                display.print("Error.");
            }
        } else if (dungeon == levelOfDamselDeath) {
            if (damsel[0].dead) {
                display.drawBitmap(0, -10, deadDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
                display.setCursor(0, 95);
                if (!knowsDamselName) {
                    display.print("You killed the Damsel!");
                } else {
                    std::string msg = "You killed " + damsel[0].name + "!";
                    display.print(msg.c_str());
                }
                display.setCursor(0, 105);
                display.print(damsel[0].levelOfLove >= 2 ? "She trusted you!" : "How could you!");
                if (damsel[0].levelOfLove >= 5) {
                    display.setCursor(0, 115);
                    display.print("She loved you!");
                }
            } else if (!damsel[0].dead && !damsel[0].followingPlayer) {
                display.drawBitmap(0, 0, leftDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
                display.setCursor(0, 115);
                if (!knowsDamselName) {
                    display.print("You left the Damsel!");
                } else {
                    std::string msg = "You left " + damsel[0].name + "!";
                    display.print(msg.c_str());
                }
                leftDamsel = true;
            }
        } else {
            display.drawBitmap(0, 0, aloneWizardScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.setCursor(0, 115);
            display.print("You progress. Alone.");
        }
    } else {
        display.drawBitmap(0, 0, capturedDamselScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
        display.setCursor(0, 2);
        display.print("The Damsel was captured!");
    }

    display.display();

    // Handle button press logic
    if (buttons.bPressed && !buttons.bPressedPrev) { // Detect new button press
        if (damselKidnapScreen) {
            // Exit the kidnap screen
            damselKidnapScreen = false;
            statusScreen = false;
            damselSayThanksForRescue = true;
        } else if (statusScreen) {
            bool rescued = damsel[0].active && !damsel[0].dead && damsel[0].followingPlayer;

            dungeon += 1;
            playerDX = 0;
            playerDY = 1;
            statusScreen = false;
            generateDungeon(); // Generate a new dungeon
            showDialogue = false;
            for (int i = 0; i < maxProjectiles; i++) {
                projectiles[i].active = false;
            }
            spawnEnemies();

            hasMap = false;

            int randomChance = rand() % 5 + 1;

            damsel[0].levelOfLove += rescued ? 1 : 0;
            damsel[0].levelOfLove += rescued && damselGotTaken ? 1 : 0;
            damsel[0].levelOfLove += rescued && carryingDamsel ? 1 : 0;
            damselGotTaken = rescued ? false : damselGotTaken;
            if (damsel[0].dead) {
                damsel[0].levelOfLove = 0;
                knowsDamselName = false;
                damsel[0].name = generateFemaleName();
            }
            if (leftDamsel) {
                damsel[0].levelOfLove = 0;
                knowsDamselName = false;
                damsel[0].name = generateFemaleName();
                leftDamsel = false;
            }

            if (rescued && randomChance == 3 && !carryingDamsel) {
                damselKidnapScreen = true; // Switch to the kidnap screen
                statusScreen = true;       // Keep status screen active
                damselGotTaken = true;
            } else if (rescued) {
                damsel[0].x = playerX;
                damsel[0].y = playerY - 1;
            }
        }
    }
}