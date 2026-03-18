#include "GameState.h"
#include "Item.h"       // for GameItem (equippedArmor / equippedWeapon live here)

// ─────────────────────────────────────────────────────────────────────────────
// The one global instance of all game state
// ─────────────────────────────────────────────────────────────────────────────
GameState g_state;

// ─────────────────────────────────────────────────────────────────────────────
// Legacy reference aliases
//
// Each alias is a reference that is bound to the corresponding g_state field.
// This means existing code that uses bare names (playerX, dungeon, etc.)
// continues to compile and work without any changes.
// ─────────────────────────────────────────────────────────────────────────────

UIState&     currentUIState         = g_state.currentUIState;
bool&        statusScreen           = g_state.statusScreen;
bool&        finalStatusScreen      = g_state.finalStatusScreen;
bool&        showDeathScreen        = g_state.showDeathScreen;
bool&        credits                = g_state.credits;
bool&        itemResultScreenActive = g_state.itemResultScreenActive;
int&         selectedActionIndex    = g_state.selectedActionIndex;
int&         selectedInventoryIndex = g_state.selectedInventoryIndex;
int&         currentInventoryPageIndex = g_state.currentInventoryPageIndex;
int&         reloadBarWidth         = g_state.reloadBarWidth;
ButtonStates& buttons               = g_state.buttons;

float&       playerX                = g_state.playerX;
float&       playerY                = g_state.playerY;
int&         playerDX               = g_state.playerDX;
int&         playerDY               = g_state.playerDY;
int&         playerHP               = g_state.playerHP;
int&         playerMaxHP            = g_state.playerMaxHP;
int&         playerFood             = g_state.playerFood;
int&         playerAttackDamage     = g_state.playerAttackDamage;
int&         attackDelayFrames      = g_state.attackDelayFrames;
bool&        playerActed            = g_state.playerActed;
bool&        playerMoving           = g_state.playerMoving;

bool&        speeding               = g_state.speeding;
float&       currentSpeedMultiplier = g_state.currentSpeedMultiplier;
float&       lastPotionSpeedModifier= g_state.lastPotionSpeedModifier;
int&         speedTimer             = g_state.speedTimer;
bool&        seeAll                 = g_state.seeAll;
int&         seeAllTimer            = g_state.seeAllTimer;
bool&        confused               = g_state.confused;
int&         confusionTimer         = g_state.confusionTimer;
bool&        glamoured              = g_state.glamoured;
int&         glamourTimer           = g_state.glamourTimer;
bool&        ridiculed              = g_state.ridiculed;
int&         ridiculeTimer          = g_state.ridiculeTimer;
bool&        isRidiculeDialogue     = g_state.isRidiculeDialogue;
bool&        blinded                = g_state.blinded;
int&         blindnessTimer         = g_state.blindnessTimer;
bool&        paralyzed              = g_state.paralyzed;
int&         paralysisTimer         = g_state.paralysisTimer;
bool&        starving               = g_state.starving;

int&         swiftnessRingsNumber   = g_state.swiftnessRingsNumber;
int&         strengthRingsNumber    = g_state.strengthRingsNumber;
int&         weaknessRingsNumber    = g_state.weaknessRingsNumber;
int&         hungerRingsNumber      = g_state.hungerRingsNumber;
int&         regenRingsNumber       = g_state.regenRingsNumber;
int&         sicknessRingsNumber    = g_state.sicknessRingsNumber;
int&         aggravateRingsNumber   = g_state.aggravateRingsNumber;
int&         armorRingsNumber       = g_state.armorRingsNumber;
int&         indigestionRingsNumber = g_state.indigestionRingsNumber;
int&         teleportRingsNumber    = g_state.teleportRingsNumber;
int&         invisibleRingsNumber   = g_state.invisibleRingsNumber;

float&       equippedArmorValue     = g_state.equippedArmorValue;
bool&        equippedRiddleStone    = g_state.equippedRiddleStone;
int&         meleeFrames            = g_state.meleeFrames;
int&         meleeArcCount          = g_state.meleeArcCount;
bool&        reloading              = g_state.reloading;
int&         shootDelay             = g_state.shootDelay;

int&         dungeon                = g_state.dungeon;
int&         kills                  = g_state.kills;
int&         keysCount              = g_state.keysCount;
int&         goldCount              = g_state.goldCount;
uint32_t&    worldSeed              = g_state.worldSeed;
bool&        endlessMode            = g_state.endlessMode;
bool&        shouldRestartGame      = g_state.shouldRestartGame;
bool&        hasMap                 = g_state.hasMap;
bool&        playerNearClockEnemy   = g_state.playerNearClockEnemy;
// ambientNoiseLevel, masterVolume, jukeboxVolume are defined in GameAudio.cpp
// and declared in Translation.h — no alias needed here.

float&       offsetX                = g_state.offsetX;
float&       offsetY                = g_state.offsetY;
int&         shakeDuration          = g_state.shakeDuration;
int&         shakeIntensity         = g_state.shakeIntensity;

bool&        showDialogue           = g_state.showDialogue;
char*        currentDialogue        = g_state.currentDialogue;
int&         dialogueTimeLength     = g_state.dialogueTimeLength;
int&         timeTillNextDialogue   = g_state.timeTillNextDialogue;

bool&        damselWasFollowing     = g_state.damselWasFollowing;
int&         damselWaitUpTimer      = g_state.damselWaitUpTimer;
bool&        damselSaidWaitUp       = g_state.damselSaidWaitUp;
bool&        damselGotTaken         = g_state.damselGotTaken;
bool&        damselSayThanksForRescue = g_state.damselSayThanksForRescue;
bool&        knowsDamselName        = g_state.knowsDamselName;
char*        damselDeathMsg         = g_state.damselDeathMsg;
bool&        DIDNOTRESCUEDAMSEL     = g_state.DIDNOTRESCUEDAMSEL;
int&         levelOfDamselDeath     = g_state.levelOfDamselDeath;

bool&        nearSuccubus           = g_state.nearSuccubus;
bool&        succubusIsFriend       = g_state.succubusIsFriend;

BossStates&  bossState              = g_state.bossState;
int&         bossStateTimer         = g_state.bossStateTimer;

bool&        combiningTwoItems      = g_state.combiningTwoItems;
int&         ingredient1index       = g_state.ingredient1index;
bool&        identifyingItem        = g_state.identifyingItem;
int&         identifyScrollPage     = g_state.identifyScrollPage;
int&         identifyScrollIndex    = g_state.identifyScrollIndex;
char*        itemResultMessage      = g_state.itemResultMessage;

bool&        pendingChestActive     = g_state.pendingChestActive;
int&         pendingChestX          = g_state.pendingChestX;
int&         pendingChestY          = g_state.pendingChestY;

int&         selectedRiddleOption   = g_state.selectedRiddleOption;
bool&        riddleGenerated        = g_state.riddleGenerated;

unsigned long& splashStartTime      = g_state.splashStartTime;
bool&          splashTimingActive   = g_state.splashTimingActive;
int&           splashShakeFrames    = g_state.splashShakeFrames;
int&           prevSplashIndex      = g_state.prevSplashIndex;
int&           introNum             = g_state.introNum;
int&           creditsBrightness    = g_state.creditsBrightness;
// masterVolume and jukeboxVolume: defined in GameAudio.cpp, declared in Translation.h

char*          deathCause           = g_state.deathCause;
int&           gameOverPage         = g_state.gameOverPage;
bool&          leftDamsel           = g_state.leftDamsel;
bool&          deleteSV             = g_state.deleteSV;
unsigned long& lastUpdateTime       = g_state.lastUpdateTime;
int&           konamiIndex          = g_state.konamiIndex;