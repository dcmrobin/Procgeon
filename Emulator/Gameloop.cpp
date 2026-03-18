#include "Common.h"
#include "GameState.h"
#include "game.h"
#include "Sprites.h"
#include "Dungeon.h"
#include "HelperFunctions.h"
#include "Entities.h"
#include "Item.h"
#include "Inventory.h"
#include "Player.h"
#include "GameAudio.h"
#include "Puzzles.h"
#include "SaveLogic.h"

#include <fstream>
#include <sstream>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// Highscore file helpers  (private to this translation unit)
// ─────────────────────────────────────────────────────────────────────────────
static void readHighscoresFromFile(int& dngnHighscore, int& kllHighscore) {
    dngnHighscore = 0;
    kllHighscore  = 0;
    std::ifstream in("highscore");
    if (!in) return;
    if (!(in >> dngnHighscore)) dngnHighscore = 0;
    if (!(in >> kllHighscore))  kllHighscore  = 0;
}

static void writeHighscoresToFile(int dngnHighscore, int kllHighscore) {
    std::ofstream out("highscore", std::ios::trunc);
    if (!out) return;
    out << dngnHighscore << " " << kllHighscore << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// resetGame
// ─────────────────────────────────────────────────────────────────────────────
void resetGame() {
    setJukeboxVolume(0.0f);
    setShopVolume(0.0f);

    g_state.deleteSV        = false;
    g_state.introNum        = 0;

    snprintf(damselDeathMsg, sizeof(g_state.damselDeathMsg), "%s", "You killed ");
    g_state.DIDNOTRESCUEDAMSEL  = false;
    g_state.shouldRestartGame   = false;
    g_state.keysCount           = 0;
    g_state.goldCount           = 0;

    stopAllAudio();

    // Player stats
    g_state.playerHP        = PLAYER_START_HP;
    g_state.playerFood      = PLAYER_START_FOOD;
    g_state.dungeon         = 1;
    g_state.levelOfDamselDeath = -4;
    g_state.kills           = 0;
    g_state.finalStatusScreen  = false;
    g_state.credits         = false;
    g_state.bossState       = Idle;
    g_state.bossStateTimer  = 0;
    g_state.creditsBrightness  = CREDITS_BRIGHTNESS_START;
    g_state.nearSuccubus    = false;
    g_state.succubusIsFriend   = false;
    g_state.endlessMode     = false;

    // Splash timing reset
    g_state.splashTimingActive = false;
    g_state.splashStartTime    = 0;
    currentSplash = splashScreen;

    // Damsel
    generateFemaleName(damsel[0].name, sizeof(damsel[0].name));
    damsel[0].levelOfLove       = 0;
    g_state.knowsDamselName     = false;
    damsel[0].beingCarried      = false;
    damsel[0].completelyRescued = false;
    g_state.damselGotTaken      = false;

    // Inventory
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        for (int j = 0; j < NUM_INVENTORY_PAGES; j++) {
            inventoryPages[j].items[i] = { Null, PotionCategory, "Empty" };
            inventoryPages[j].itemCount = 0;
        }
    }

    // Starting cloak
    GameItem cloak = getItem(Cloak);
    cloak.isEquipped = true;
    addToInventory(cloak, false);
    g_state.equippedArmorValue = cloak.armorValue;
    equippedArmor = cloak;

    // Starting staff
    GameItem staff = getItem(Weapon);
    staff.isEquipped = true;
    staff.weapon = weaponList[Staff];
    snprintf(staff.description,   sizeof(staff.description),   "%s", weaponList[Staff].description);
    snprintf(staff.name,          sizeof(staff.name),           "%s", weaponList[Staff].name);
    snprintf(staff.originalName,  sizeof(staff.originalName),   "%s", weaponList[Staff].name);
    staff.canRust = weaponList[Staff].canRust;
    addToInventory(staff, false);
    equippedWeapon = staff;

    // Projectiles
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        projectiles[i].active = false;
    }

    // UI / status flags
    g_state.currentUIState          = UI_NORMAL;
    g_state.speeding                = false;
    g_state.currentSpeedMultiplier  = 1.0f;
    g_state.speedTimer              = SPEED_TIMER_DEFAULT;
    g_state.hasMap                  = false;
    g_state.equippedRiddleStone     = false;
    g_state.starving                = false;
    g_state.seeAll                  = false;
    g_state.confused                = false;
    g_state.ridiculed               = false;
    g_state.glamoured               = false;
    g_state.blinded                 = false;
    g_state.showDialogue            = false;
    g_state.paralyzed               = false;

    // Damsel follow state
    g_state.damselWasFollowing  = false;
    g_state.damselWaitUpTimer   = 0;
    g_state.damselSaidWaitUp    = false;

    // Potion / scroll / ring randomisation
    resetPotionNames();
    randomizePotionEffects();
    randomizeScrollEffects();
    randomizeRingEffects();

    // Ring counters
    g_state.swiftnessRingsNumber    = 0;
    g_state.strengthRingsNumber     = 0;
    g_state.weaknessRingsNumber     = 0;
    g_state.hungerRingsNumber       = 0;
    g_state.regenRingsNumber        = 0;
    g_state.lastPotionSpeedModifier = 0.0f;
    g_state.playerAttackDamage      = PLAYER_BASE_ATTACK;
    g_state.sicknessRingsNumber     = 0;
    g_state.aggravateRingsNumber    = 0;
    g_state.armorRingsNumber        = 0;
    g_state.indigestionRingsNumber  = 0;
    g_state.teleportRingsNumber     = 0;
    g_state.invisibleRingsNumber    = 0;

    generateDungeon(false);
    spawnEnemies(false);
}

// ─────────────────────────────────────────────────────────────────────────────
// game_setup
// ─────────────────────────────────────────────────────────────────────────────
void game_setup() {
    initAudio();

    if (!SD.begin(SD_CS)) {
        while (1);  // halt — SD required
    }

    if (!loadSFXtoRAM()) {
        // non-fatal: game runs without SFX
    }

    worldSeed = generateRandomSeed();
    randomSeed(worldSeed);

    trainFemaleMarkov();
    generateFemaleName(damsel[0].name, sizeof(damsel[0].name));

    display.begin();
    display.setContrast(100);

    pinMode(BUTTON_UP_PIN,    INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN,  INPUT_PULLUP);
    pinMode(BUTTON_LEFT_PIN,  INPUT_PULLUP);
    pinMode(BUTTON_RIGHT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_B_PIN,     INPUT_PULLUP);
    pinMode(BUTTON_A_PIN,     INPUT_PULLUP);
    pinMode(BUTTON_START_PIN, INPUT_PULLUP);
    display.clearDisplay();

    resetGame();
    g_state.currentUIState = UI_INTRO;
}

// ─────────────────────────────────────────────────────────────────────────────
// game_loop
// ─────────────────────────────────────────────────────────────────────────────
void game_loop() {
    serviceRawSFX();

    if (g_state.shouldRestartGame) {
        resetGame();
        g_state.shouldRestartGame = false;
    }

    unsigned long currentTime = millis();
    if (currentTime - g_state.lastUpdateTime >= FRAME_DELAY_MS) {
        g_state.lastUpdateTime = currentTime;
        updateButtonStates();

        if (!g_state.credits) {
            // Allow riddle UI even at 0 HP
            if (g_state.playerHP > 0 || g_state.currentUIState == UI_RIDDLE) {
                handleUIStateTransitions();

                if (!g_state.statusScreen) {
                    switch (g_state.currentUIState) {
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

                        case UI_SPLASH:
                            renderSplashScreen();
                            break;

                        case UI_INTRO:
                            renderIntroScreen();
                            break;

                        case UI_SECRET:
                            renderSecretScreen();
                            break;

                        case UI_PICROSS:
                            updatePicrossPuzzle();
                            if (g_state.buttons.aPressed && !g_state.buttons.aPressedPrev) {
                                puzzleFinished    = true;
                                puzzleSuccess     = false;
                                pendingChestActive = false;
                                g_state.currentUIState = UI_NORMAL;
                            }
                            break;

                        case UI_LIGHTSOUT:
                            updateLightsOutPuzzle();
                            if (g_state.buttons.aPressed && !g_state.buttons.aPressedPrev) {
                                puzzleFinished    = true;
                                puzzleSuccess     = false;
                                pendingChestActive = false;
                                g_state.currentUIState = UI_NORMAL;
                            }
                            break;
                    }
                } else {
                    showStatusScreen();
                }
            } else {
                // Player is dead
                if (!g_state.deleteSV) {
                    deleteSave();
                    g_state.deleteSV = true;
                }
                gameOver();
            }
        } else {
            renderCredits();
        }
    }

    // Jukebox ambient track — loops continuously
    if (!playWav2.isPlaying()) {
        playWav2.play("./Audio/12_8.wav");
    }

    if (!playWav3.isPlaying()) {
        playWav3.play("./Audio/calm.wav");
    }

    // Silence music on pause
    if (g_state.currentUIState == UI_PAUSE) {
        setJukeboxVolume(0.0f);
        setShopVolume(0.0f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// updateGame  (per-frame game state update, called from UI_NORMAL only)
// ─────────────────────────────────────────────────────────────────────────────
void updateGame() {
    updateScreenShake();
    handleInput();
    updateScrolling(viewportWidth, viewportHeight, scrollSpeed, offsetX, offsetY);
    updateParticles();

    if (playerActed || playerNearClockEnemy) {
        handleAmbientNoiseLevel();
        handleHungerAndEffects();
        updateDamsel();
        updateProjectiles();
        if (g_state.dungeon == bossfightLevel) {
            updateBossfight();
        }
    }
    updateEnemies();
}

// ─────────────────────────────────────────────────────────────────────────────
// handleAmbientNoiseLevel
// ─────────────────────────────────────────────────────────────────────────────
void handleAmbientNoiseLevel() {
    static int noiseLevelDiffuseTimer = 0;
    if (ambientNoiseLevel > 0) {
        noiseLevelDiffuseTimer++;
    }
    if (noiseLevelDiffuseTimer >= NOISE_DIFFUSE_TICKS) {
        ambientNoiseLevel--;
        noiseLevelDiffuseTimer = 0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// renderGame  (called from UI_NORMAL each frame)
// ─────────────────────────────────────────────────────────────────────────────
void renderGame() {
    display.clearDisplay();
    if (!g_state.blinded) {
        renderDungeon();
        renderDamsel();
        renderEnemies();
        renderProjectiles();
        renderParticles();
    }
    renderPlayer();
    renderUI();
    handleDialogue();
    if (playerActed || playerNearClockEnemy) {
        updateAnimations();
    }
    display.display();
}