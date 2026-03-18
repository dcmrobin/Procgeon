#ifndef GAMESTATE_H
#define GAMESTATE_H

// ─────────────────────────────────────────────────────────────────────────────
// GameState.h
//
// All mutable global game state lives in one place.
// Rules:
//   • Declare the struct here, define the instance in GameState.cpp.
//   • Every module that needs state includes this header and accesses
//     g_state.<field> — no more bare globals scattered across translation units.
//   • The legacy bare-name externs (playerX, dungeon, etc.) are provided as
//     reference aliases at the bottom of this file so existing .cpp files
//     compile without a mass rename.  They can be removed gradually.
// ─────────────────────────────────────────────────────────────────────────────

#include "Common.h"

// Forward-declare heavy structs so we don't need full headers here
struct GameItem;
struct InventoryPage;
struct Enemy;
struct Damsel;
struct Projectile;
struct Particle;

// ─────────────────────────────────────────────────────────────────────────────
// Button input snapshot  (was ButtonStates in HelperFunctions.h)
// ─────────────────────────────────────────────────────────────────────────────
struct ButtonStates {
    bool upPressed       = false;
    bool upPressedPrev   = false;
    bool downPressed     = false;
    bool downPressedPrev = false;
    bool aPressed        = false;
    bool aPressedPrev    = false;
    bool bPressed        = false;
    bool bPressedPrev    = false;
    bool leftPressed     = false;
    bool leftPressedPrev = false;
    bool rightPressed    = false;
    bool rightPressedPrev= false;
    bool startPressed    = false;
    bool startPressedPrev= false;
};

// ─────────────────────────────────────────────────────────────────────────────
// Riddle data  (was spread across HelperFunctions)
// ─────────────────────────────────────────────────────────────────────────────
struct GeneratedRiddle {
    char riddle[300]    = "";
    char options[4][50] = {};
    int  correctOption  = 0;
};

struct RiddleAnswer {
    const char* word;
    const char* attributes[4];
};

// ─────────────────────────────────────────────────────────────────────────────
// PathNode  (was in Entities.h — needed by SaveLogic and Entities)
// ─────────────────────────────────────────────────────────────────────────────
struct PathNode {
    int x, y;
};

// ─────────────────────────────────────────────────────────────────────────────
// Dialogue  (was in Entities.h)
// ─────────────────────────────────────────────────────────────────────────────
struct Dialogue {
    char message[200] = "";
    int  duration     = 0;
    char tone[20]     = "normal";
    bool alreadyBeenSaid = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// Room  (was in Dungeon.h)
// ─────────────────────────────────────────────────────────────────────────────
struct Room {
    int x, y, width, height;
};

// ─────────────────────────────────────────────────────────────────────────────
// The global state struct
// ─────────────────────────────────────────────────────────────────────────────
struct GameState {

    // ── UI ────────────────────────────────────────────────────────────────
    UIState     currentUIState      = UI_INTRO;
    bool        statusScreen        = false;
    bool        finalStatusScreen   = false;
    bool        showDeathScreen     = false;
    bool        credits             = false;
    bool        itemResultScreenActive = false;
    int         selectedActionIndex = 0;
    int         selectedInventoryIndex = 0;
    int         currentInventoryPageIndex = 0;
    int         reloadBarWidth      = 0;
    bool        showTooltip         = true;

    // ── Input ─────────────────────────────────────────────────────────────
    ButtonStates buttons;

    // ── Player position & stats ───────────────────────────────────────────
    float   playerX             = 0.0f;
    float   playerY             = 0.0f;
    int     playerDX            = 0;
    int     playerDY            = 1;
    int     playerHP            = PLAYER_START_HP;
    int     playerMaxHP         = PLAYER_MAX_HP;
    int     playerFood          = PLAYER_START_FOOD;
    int     playerAttackDamage  = PLAYER_BASE_ATTACK;
    int     attackDelayFrames   = DEFAULT_ATTACK_DELAY;
    bool    playerActed         = false;
    bool    playerMoving        = false;

    // ── Player status effects ─────────────────────────────────────────────
    bool    speeding            = false;
    float   currentSpeedMultiplier = 1.0f;
    float   lastPotionSpeedModifier = 0.0f;
    int     speedTimer          = SPEED_TIMER_DEFAULT;

    bool    seeAll              = false;
    int     seeAllTimer         = SEE_ALL_TIMER_DEFAULT;

    bool    confused            = false;
    int     confusionTimer      = CONFUSION_TIMER_DEFAULT;

    bool    glamoured           = false;
    int     glamourTimer        = GLAMOUR_TIMER_DEFAULT;

    bool    ridiculed           = false;
    int     ridiculeTimer       = RIDICULE_DURATION;
    int     lastRidiculeIndex   = -1;
    bool    isRidiculeDialogue  = false;

    bool    blinded             = false;
    int     blindnessTimer      = BLINDNESS_TIMER_DEFAULT;

    bool    paralyzed           = false;
    int     paralysisTimer      = PARALYSIS_TIMER_DEFAULT;

    bool    starving            = false;

    // ── Ring counters ─────────────────────────────────────────────────────
    int     swiftnessRingsNumber    = 0;
    int     strengthRingsNumber     = 0;
    int     weaknessRingsNumber     = 0;
    int     hungerRingsNumber       = 0;
    int     regenRingsNumber        = 0;
    int     sicknessRingsNumber     = 0;
    int     aggravateRingsNumber    = 0;
    int     armorRingsNumber        = 0;
    int     indigestionRingsNumber  = 0;
    int     teleportRingsNumber     = 0;
    int     invisibleRingsNumber    = 0;

    // ── Equipment ─────────────────────────────────────────────────────────
    float   equippedArmorValue  = 0.0f;
    bool    equippedRiddleStone = false;
    // GameItem equippedArmor / equippedWeapon defined in GameState.cpp
    // (forward-declared here to avoid including Item.h)

    // ── Melee FX ──────────────────────────────────────────────────────────
    int     meleeFrames         = 0;
    int     meleeFX             = -1;
    int     meleeFY             = -1;
    int     meleeArcTilesX[MAX_MELEE_TILES] = {};
    int     meleeArcTilesY[MAX_MELEE_TILES] = {};
    int     meleeArcCount       = 0;

    // ── Shooting ──────────────────────────────────────────────────────────
    bool    reloading           = false;
    int     shootDelay          = 0;

    // ── World / progression ───────────────────────────────────────────────
    int         dungeon             = 1;
    int         kills               = 0;
    int         keysCount           = 0;
    int         goldCount           = 0;
    uint32_t    worldSeed           = 0;
    bool        endlessMode         = false;
    bool        shouldRestartGame   = false;
    bool        hasMap              = false;
    bool        playerNearClockEnemy = false;
    bool        shopOnThisFloor     = false;
    Room        rooms[MAX_ROOMS_MAX] = {};
    int         ambientNoiseLevel   = 0;

    // ── Camera / viewport ─────────────────────────────────────────────────
    float   offsetX             = 0.0f;
    float   offsetY             = 0.0f;
    int     shakeDuration       = 0;
    int     shakeIntensity      = 1;

    // ── Dialogue ──────────────────────────────────────────────────────────
    bool    showDialogue        = false;
    char    currentDialogue[200]= "";
    int     dialogueTimeLength  = 1000;
    int     timeTillNextDialogue= 1000;

    // ── Damsel ────────────────────────────────────────────────────────────
    bool    damselWasFollowing      = false;
    int     damselWaitUpTimer       = 0;
    bool    damselSaidWaitUp        = false;
    bool    damselGotTaken          = false;
    bool    damselSayThanksForRescue= false;
    bool    knowsDamselName         = false;
    char    damselDeathMsg[100]     = "You killed ";
    bool    DIDNOTRESCUEDAMSEL      = false;
    int     levelOfDamselDeath      = -4;

    // ── Succubus / special ────────────────────────────────────────────────
    bool    nearSuccubus        = false;
    bool    succubusIsFriend    = false;

    // ── Boss ──────────────────────────────────────────────────────────────
    BossStates  bossState       = Idle;
    int         bossStateTimer  = 0;

    // ── Inventory / combining ─────────────────────────────────────────────
    bool    combiningTwoItems   = false;
    int     ingredient1index    = 0;
    bool    identifyingItem     = false;
    int     identifyScrollPage  = -1;
    int     identifyScrollIndex = -1;
    char    itemResultMessage[150] = "";

    // Shop
    int shopSelectedIndex = 0;
    int shopItemCount = 0;

    // ── Pending chest (puzzle gate) ───────────────────────────────────────
    bool    pendingChestActive  = false;
    int     pendingChestX       = -1;
    int     pendingChestY       = -1;

    // ── Riddle ────────────────────────────────────────────────────────────
    GeneratedRiddle currentRiddle;
    int     selectedRiddleOption = 0;
    bool    riddleGenerated      = false;

    // ── Splash / intro ────────────────────────────────────────────────────
    unsigned long   splashStartTime     = 0;
    bool            splashTimingActive  = false;
    int             splashShakeFrames   = 0;
    int             prevSplashIndex     = -1;
    int             introNum            = 0;

    // ── Credits ───────────────────────────────────────────────────────────
    int     creditsBrightness   = CREDITS_BRIGHTNESS_START;

    // ── Audio ─────────────────────────────────────────────────────────────
    int     masterVolume        = MASTER_VOLUME_DEFAULT;
    float   jukeboxVolume       = 0.0f;

    // ── Death / game-over ─────────────────────────────────────────────────
    char    deathCause[50]      = "";
    int     gameOverPage        = 1;
    bool    leftDamsel          = false;
    bool    deleteSV            = false;

    // ── Timing ────────────────────────────────────────────────────────────
    unsigned long   lastUpdateTime  = 0;

    // ── Konami ────────────────────────────────────────────────────────────
    int     konamiIndex         = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// The one global instance — defined in GameState.cpp
// ─────────────────────────────────────────────────────────────────────────────
extern GameState g_state;

// ─────────────────────────────────────────────────────────────────────────────
// Legacy compatibility aliases
//
// These reference aliases let existing .cpp files keep using bare names like
// playerX, dungeon, etc. without any changes.  They resolve to g_state fields.
// Over time, direct uses can be migrated to g_state.<field> and these removed.
//
// NOTE: Reference aliases to struct members are valid C++ — they bind at
// program startup and thereafter behave exactly like the original extern.
// ─────────────────────────────────────────────────────────────────────────────
// (Defined in GameState.cpp — declared here as extern references)

extern UIState&     currentUIState;
extern bool&        statusScreen;
extern bool&        finalStatusScreen;
extern bool&        showDeathScreen;
extern bool&        credits;
extern bool&        itemResultScreenActive;
extern int&         selectedActionIndex;
extern int&         selectedInventoryIndex;
extern int&         currentInventoryPageIndex;
extern int&         reloadBarWidth;
extern ButtonStates& buttons;

extern float&       playerX;
extern float&       playerY;
extern int&         playerDX;
extern int&         playerDY;
extern int&         playerHP;
extern int&         playerMaxHP;
extern int&         playerFood;
extern int&         playerAttackDamage;
extern int&         attackDelayFrames;
extern bool&        playerActed;
extern bool&        playerMoving;

extern bool&        speeding;
extern float&       currentSpeedMultiplier;
extern float&       lastPotionSpeedModifier;
extern int&         speedTimer;
extern bool&        seeAll;
extern int&         seeAllTimer;
extern bool&        confused;
extern int&         confusionTimer;
extern bool&        glamoured;
extern int&         glamourTimer;
extern bool&        ridiculed;
extern int&         ridiculeTimer;
extern bool&        isRidiculeDialogue;
extern bool&        blinded;
extern int&         blindnessTimer;
extern bool&        paralyzed;
extern int&         paralysisTimer;
extern bool&        starving;

extern int&         swiftnessRingsNumber;
extern int&         strengthRingsNumber;
extern int&         weaknessRingsNumber;
extern int&         hungerRingsNumber;
extern int&         regenRingsNumber;
extern int&         sicknessRingsNumber;
extern int&         aggravateRingsNumber;
extern int&         armorRingsNumber;
extern int&         indigestionRingsNumber;
extern int&         teleportRingsNumber;
extern int&         invisibleRingsNumber;

extern float&       equippedArmorValue;
extern bool&        equippedRiddleStone;
extern int&         meleeFrames;
extern int&         meleeArcCount;
extern bool&        reloading;
extern int&         shootDelay;

extern int&         dungeon;
extern int&         kills;
extern int&         keysCount;
extern int&         goldCount;
extern uint32_t&    worldSeed;
extern bool&        endlessMode;
extern bool&        shouldRestartGame;
extern bool&        hasMap;
extern bool&        playerNearClockEnemy;
// ambientNoiseLevel, masterVolume, jukeboxVolume are declared in Translation.h
// as plain extern int/float (owned by GameAudio.cpp) — do NOT redeclare here.

extern float&       offsetX;
extern float&       offsetY;
extern int&         shakeDuration;
extern int&         shakeIntensity;

extern bool&        showDialogue;
extern char*        currentDialogue;    // char array — use pointer alias
extern int&         dialogueTimeLength;
extern int&         timeTillNextDialogue;

extern bool&        damselWasFollowing;
extern int&         damselWaitUpTimer;
extern bool&        damselSaidWaitUp;
extern bool&        damselGotTaken;
extern bool&        damselSayThanksForRescue;
extern bool&        knowsDamselName;
extern char*        damselDeathMsg;
extern bool&        DIDNOTRESCUEDAMSEL;
extern int&         levelOfDamselDeath;

extern bool&        nearSuccubus;
extern bool&        succubusIsFriend;

extern BossStates&  bossState;
extern int&         bossStateTimer;

extern bool&        combiningTwoItems;
extern int&         ingredient1index;
extern bool&        identifyingItem;
extern int&         identifyScrollPage;
extern int&         identifyScrollIndex;
extern char*        itemResultMessage;

extern bool&        pendingChestActive;
extern int&         pendingChestX;
extern int&         pendingChestY;

extern int&         selectedRiddleOption;
extern bool&        riddleGenerated;

extern unsigned long& splashStartTime;
extern bool&          splashTimingActive;
extern int&           splashShakeFrames;
extern int&           prevSplashIndex;
extern int&           introNum;
extern int&           creditsBrightness;

// masterVolume and jukeboxVolume: see note above — owned by GameAudio.cpp/Translation.h

extern char*          deathCause;
extern int&           gameOverPage;
extern bool&          leftDamsel;
extern bool&          deleteSV;
extern unsigned long& lastUpdateTime;
extern int&           konamiIndex;

#endif // GAMESTATE_H