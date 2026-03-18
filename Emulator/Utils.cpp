#include "Common.h"
#include "GameState.h"
#include "Sprites.h"
#include "Dungeon.h"
#include "Entities.h"
#include "HelperFunctions.h"
#include "Item.h"
#include "Inventory.h"
#include "SaveLogic.h"
#include "GameAudio.h"
#include "Puzzles.h"

#include <cstring>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Utils.cpp
//
// Pure utility functions — no rendering, no game-loop logic.
// Covers:
//   • Collision detection (tile and sprite)
//   • Visibility (Bresenham line-of-sight)
//   • Walkability + enemy unstuck
//   • Dungeon geometry (corridor carving, room edge, swap, wall count)
//   • Screen shake
//   • Tile proximity helper
//   • Death check
//   • Name / seed generation (Markov chain)
//   • Save / load wrappers (trySaveGame / tryLoadGame)
// ─────────────────────────────────────────────────────────────────────────────

// ── Viewport constants (extern in HelperFunctions.h) ────────────────────────
const int   viewportWidth  = VIEWPORT_TILE_W;
const int   viewportHeight = VIEWPORT_TILE_H;
const float scrollSpeed    = 0.25f;

// ── Konami code (extern in HelperFunctions.h) ────────────────────────────────
const KonamiInput konamiCode[] = {
    K_UP, K_UP, K_DOWN, K_DOWN,
    K_LEFT, K_RIGHT, K_LEFT, K_RIGHT,
    K_B, K_A
};
const int konamiLength = sizeof(konamiCode) / sizeof(konamiCode[0]);

// ── Riddle data (extern in HelperFunctions.h) ────────────────────────────────
RiddleAnswer possibleAnswers[] = {
    {"sea",      {"boundless",  "whispering", "enigmatic",  "murmuring"}},
    {"fire",     {"burning",    "flickering", "transient",  "intense"  }},
    {"shadow",   {"elusive",    "fleeting",   "whispering", "ephemeral"}},
    {"cloud",    {"drifting",   "evanescent", "vague",      "mysterious"}},
    {"wind",     {"invisible",  "restless",   "unseen",     "quiet"    }},
    {"time",     {"slipping",   "silent",     "elusive",    "everflowing"}},
    {"mountain", {"stolid",     "ancient",    "unmoving",   "enduring" }},
    {"river",    {"wandering",  "serene",     "meandering", "subtle"   }},
    {"star",     {"distant",    "silent",     "celestial",  "untold"   }},
    {"tree",     {"rooted",     "whispering", "quiet",      "timeless" }}
};
const int numAnswers = sizeof(possibleAnswers) / sizeof(possibleAnswers[0]);

static const char* riddleTemplates[] = {
    "I am always %s, yet sometimes %s. What am I?",
    "I can be %s and also %s. What could I be?",
    "Often %s, occasionally %s. Who am I?",
    "I embody %s and reveal %s. Can you name me?",
    "Known for being %s, and sometimes %s. Guess what I am?",
    "I am described as %s, but also %s. What am I?",
    "Many say I am %s, yet I can be %s too. Who am I?",
    "I move with %s and hide in %s. What might I be?",
    "I am both %s and %s. Who might I be?",
    "I appear %s, and sometimes I feel %s. What is my name?",
    "I am often seen as %s, but I can turn %s. Who am I?",
    "Some say I'm %s, while others call me %s. What am I?"
};
static const int numRiddleTemplates =
    sizeof(riddleTemplates) / sizeof(riddleTemplates[0]);

// ─────────────────────────────────────────────────────────────────────────────
// Tile prediction
// ─────────────────────────────────────────────────────────────────────────────
int predictXtile(float x) { return (int)(x + 0.5f); }
int predictYtile(float y) { return (int)(y + 0.5f); }

// ─────────────────────────────────────────────────────────────────────────────
// Collision helpers
// ─────────────────────────────────────────────────────────────────────────────
bool checkSpriteCollisionWithTileX(float newX, float currentX, float newY) {
    int ptx = predictXtile(newX);
    int cty = round(newY);
    bool valid = (newX >= 0 && newX < mapWidth &&
                  dungeonMap[cty][ptx] != Wall  &&
                  dungeonMap[cty][ptx] != Bars  &&
                  dungeonMap[cty][ptx] != ChestTile);
    return !valid;
}

bool checkSpriteCollisionWithTileY(float newY, float currentY, float newX) {
    int pty = predictYtile(newY);
    int ctx = round(newX);
    bool valid = (newY >= 0 && newY < mapHeight &&
                  dungeonMap[pty][ctx] != Wall  &&
                  dungeonMap[pty][ctx] != Bars  &&
                  dungeonMap[pty][ctx] != ChestTile);
    return !valid;
}

bool checkSpriteCollisionWithSprite(float s1x, float s1y, float s2x, float s2y) {
    return predictXtile(s1x) == predictXtile(s2x) &&
           predictYtile(s1y) == predictYtile(s2y);
}

// ─────────────────────────────────────────────────────────────────────────────
// Visibility  (Bresenham ray)
// ─────────────────────────────────────────────────────────────────────────────
bool isVisible(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        if (x0 < 0 || x0 >= mapWidth || y0 < 0 || y0 >= mapHeight) return false;
        if (x0 == x1 && y0 == y1) break;

        TileTypes tile = dungeonMap[y0][x0];
        if (tile == Wall || tile == DoorClosed || tile == ShopWall) return false;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 <  dx) { err += dx; y0 += sy; }
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Walkability
// ─────────────────────────────────────────────────────────────────────────────
bool isWalkable(int x, int y) {
    if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) return false;
    TileTypes t = dungeonMap[y][x];
    return (t == Floor       || t == StartStairs || t == Exit      ||
            t == Freedom     || t == Potion      || t == Map       ||
            t == MushroomTile|| t == RingTile    || t == ArmorTile ||
            t == ScrollTile  || t == DoorOpen    || t == RiddleStoneTile ||
            t == KeyItem     || t == KeyTile     || t == WeaponTile ||
            t == GoldTile);
}

// ─────────────────────────────────────────────────────────────────────────────
// Unstuck enemy
// ─────────────────────────────────────────────────────────────────────────────
void unstuckEnemy(Enemy& enemy) {
    int ex = round(enemy.x);
    int ey = round(enemy.y);
    if (isWalkable(ex, ey)) return;

    for (int radius = 1; radius <= 5; radius++) {
        for (int ddx = -radius; ddx <= radius; ddx++) {
            for (int ddy = -radius; ddy <= radius; ddy++) {
                if (abs(ddx) != radius && abs(ddy) != radius) continue;
                int cx = ex + ddx;
                int cy = ey + ddy;
                if (isWalkable(cx, cy)) {
                    enemy.x = (float)cx;
                    enemy.y = (float)cy;
                    return;
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Tile proximity
// ─────────────────────────────────────────────────────────────────────────────
bool nearTile(TileTypes tile) {
    int rPx = round(playerX);
    int rPy = round(playerY);
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int cx = rPx + dx;
            int cy = rPy + dy;
            if (cx >= 0 && cx < mapWidth &&
                cy >= 0 && cy < mapHeight &&
                dungeonMap[cy][cx] == tile)
                return true;
        }
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Screen shake
// ─────────────────────────────────────────────────────────────────────────────
void updateScreenShake() {
    if (shakeDuration > 0) {
        offsetX += (float)random(-shakeIntensity, shakeIntensity + 1);
        offsetY += (float)random(-shakeIntensity, shakeIntensity + 1);
        shakeDuration--;
    }
}

void triggerScreenShake(int duration, int intensity) {
    shakeDuration  = duration;
    shakeIntensity = intensity;
}

// ─────────────────────────────────────────────────────────────────────────────
// Dungeon geometry helpers
// ─────────────────────────────────────────────────────────────────────────────
void swap(int& a, int& b) { int t = a; a = b; b = t; }

int countWalls(int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
            if ((dx != 0 || dy != 0) && dungeonMap[y + dy][x + dx] == Wall)
                count++;
    return count;
}

void carveHorizontalCorridor(int x1, int x2, int y) {
    if (x1 > x2) swap(x1, x2);
    for (int x = x1; x <= x2; x++) dungeonMap[y][x] = Floor;
}

void carveVerticalCorridor(int y1, int y2, int x) {
    if (y1 > y2) swap(y1, y2);
    for (int y = y1; y <= y2; y++) dungeonMap[y][x] = Floor;
}

void getEdgeTowards(const Room& from, const Room& to, int& outX, int& outY) {
    int dx = (to.x   + to.width  / 2) - (from.x + from.width  / 2);
    int dy = (to.y   + to.height / 2) - (from.y + from.height / 2);

    if (abs(dx) > abs(dy)) {
        outY = from.y + random(1, from.height - 2);
        outX = (dx > 0) ? from.x + from.width - 1 : from.x;
    } else {
        outX = from.x + random(1, from.width - 2);
        outY = (dy > 0) ? from.y + from.height - 1 : from.y;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Death check
// ─────────────────────────────────────────────────────────────────────────────
void checkIfDeadFrom(const char* cause) {
    if (playerHP > 0) return;

    // Riddle stone saves the player once
    if (equippedRiddleStone) {
        currentUIState      = UI_RIDDLE;
        equippedRiddleStone = false;
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            if (strcmp(inventoryPages[2].items[i].itemResult, "Solve this riddle!") == 0) {
                removeItemFromInventory(2, i);
                break;
            }
        }
        return;
    }

    playRawSFX(10);
    snprintf(deathCause, 50, "%s", cause);
    g_state.buttons.bPressedPrev = true;
    g_state.buttons.aPressedPrev = true;
    showDeathScreen = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Markov name generator
// ─────────────────────────────────────────────────────────────────────────────
static int femaleTransition[MARKOV_LETTERS][MARKOV_LETTERS];

static const char* sampleFemaleNames[] = {
    "Liora","Lucy","Ruby","Talitha","Mary","Sarah","Salina","Olivia",
    "Evelyn","Valerie","Jenny","Eva","Luna","Isabella","Maria","Arwen",
    "Sophie","Felicity","Rebecca","Julia","Rebecca"
};
static const int sampleCount =
    sizeof(sampleFemaleNames) / sizeof(sampleFemaleNames[0]);

void trainFemaleMarkov() {
    for (int i = 0; i < MARKOV_LETTERS; i++)
        for (int j = 0; j < MARKOV_LETTERS; j++)
            femaleTransition[i][j] = 0;

    for (int i = 0; i < sampleCount; i++) {
        const char* name = sampleFemaleNames[i];
        int len = strlen(name);
        for (int j = 0; j < len - 1; j++) {
            char cur  = tolower(name[j]);
            char next = tolower(name[j + 1]);
            if (cur  >= 'a' && cur  <= 'z' &&
                next >= 'a' && next <= 'z')
                femaleTransition[cur - 'a'][next - 'a']++;
        }
    }
}

void generateFemaleName(char* name, size_t nameSize) {
    char temp[NAME_BUFFER_SIZE + 1];
    int  startLetter = random(0, MARKOV_LETTERS);
    temp[0] = 'a' + startLetter;
    int length = random(NAME_MIN_LEN, NAME_BUFFER_SIZE);

    for (int i = 1; i < length; i++) {
        int prev  = temp[i - 1] - 'a';
        int total = 0;
        for (int j = 0; j < MARKOV_LETTERS; j++) total += femaleTransition[prev][j];

        int next = random(0, MARKOV_LETTERS);
        if (total > 0) {
            int rnd = random(0, total), cum = 0;
            for (int j = 0; j < MARKOV_LETTERS; j++) {
                cum += femaleTransition[prev][j];
                if (rnd < cum) { next = j; break; }
            }
        }
        temp[i] = 'a' + next;
    }
    temp[length] = '\0';
    temp[0] = (char)toupper(temp[0]);

    // Reject names with uncommon letters
    bool bad = false;
    for (int i = 0; i < length; i++) {
        char c = temp[i];
        if (c=='z'||c=='Z'||c=='x'||c=='X'||c=='q'||c=='Q') { bad = true; break; }
    }
    snprintf(name, nameSize, "%s",
             bad ? sampleFemaleNames[random(0, sampleCount)] : temp);
}

// ─────────────────────────────────────────────────────────────────────────────
// Random seed
// ─────────────────────────────────────────────────────────────────────────────
uint32_t generateRandomSeed() {
    uint8_t  seedBitValue  = 0;
    uint8_t  seedByteValue = 0;
    uint32_t seedWordValue = 0;

    for (uint8_t wordShift = 0; wordShift < 4; wordShift++) {
        for (uint8_t byteShift = 0; byteShift < 8; byteShift++) {
            for (uint8_t bitSum = 0; bitSum <= 8; bitSum++)
                seedBitValue += (analogRead(SEED_PIN) & 0x01);
            delay(1);
            seedByteValue |= ((seedBitValue & 0x01) << byteShift);
            seedBitValue   = 0;
        }
        seedWordValue |= (uint32_t)seedByteValue << (8 * wordShift);
        seedByteValue  = 0;
    }
    return seedWordValue;
}

// ─────────────────────────────────────────────────────────────────────────────
// Riddle generation
// ─────────────────────────────────────────────────────────────────────────────
static void shuffleArray(int arr[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j   = random(i + 1);
        int tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
}

void generateRiddleUI() {
    int answerIndex = random(numAnswers);
    RiddleAnswer& chosen = possibleAnswers[answerIndex];

    int a1 = random(4), a2 = random(4);
    while (a2 == a1) a2 = random(4);

    char buf[128];
    snprintf(buf, sizeof(buf),
             riddleTemplates[random(numRiddleTemplates)],
             chosen.attributes[a1], chosen.attributes[a2]);
    snprintf(g_state.currentRiddle.riddle,
             sizeof(g_state.currentRiddle.riddle), "%s", buf);

    const int totalOptions = 4;
    int optionIndices[totalOptions];
    optionIndices[0] = answerIndex;
    int count = 1;
    while (count < totalOptions) {
        int decoy = random(numAnswers);
        if (decoy == answerIndex) continue;
        bool dup = false;
        for (int i = 0; i < count; i++) if (optionIndices[i] == decoy) { dup = true; break; }
        if (!dup) optionIndices[count++] = decoy;
    }
    shuffleArray(optionIndices, totalOptions);

    for (int i = 0; i < totalOptions; i++) {
        snprintf(g_state.currentRiddle.options[i],
                 sizeof(g_state.currentRiddle.options[i]),
                 "%s", possibleAnswers[optionIndices[i]].word);
        if (optionIndices[i] == answerIndex)
            g_state.currentRiddle.correctOption = i;
    }
    selectedRiddleOption = 0;
    riddleGenerated      = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Save / load wrappers
// ─────────────────────────────────────────────────────────────────────────────
// Forward-declared in HelperFunctions.h; implementation mirrors old
// HelperFunctions.cpp exactly so behaviour is unchanged.

static SaveData saveData;   // module-private; no need to expose it globally

void trySaveGame() {
    saveData.armorValue               = equippedArmorValue;
    saveData.attackDamage             = playerAttackDamage;
    saveData.currentDungeon           = (uint8_t)dungeon;
    saveData.damsel                   = damsel[0];
    saveData.endlessMode              = endlessMode;
    saveData.equippedArmor            = equippedArmor;
    saveData.equippedWeapon           = equippedWeapon;
    saveData.equippedRiddleStone      = equippedRiddleStone;
    saveData.succubusFriend           = succubusIsFriend;
    saveData.kills                    = kills;
    saveData.hasMap                   = hasMap;
    saveData.playerNearClockEnemy     = playerNearClockEnemy;
    saveData.knowsDamselName          = knowsDamselName;
    saveData.damselSayThanksForRescue = damselSayThanksForRescue;
    saveData.damselGotTaken           = damselGotTaken;
    saveData.levelOfDamselDeath       = levelOfDamselDeath;
    saveData.worldSeed                = worldSeed;
    saveData.keysCount                = keysCount;
    saveData.goldCount                = goldCount;

    saveData.playerX = playerX;
    saveData.playerY = playerY;
    saveData.hp      = (int16_t)playerHP;
    saveData.food    = (int16_t)playerFood;

    saveData.swiftnessRingsNum      = swiftnessRingsNumber;
    saveData.strengthRingsNum       = strengthRingsNumber;
    saveData.weaknessRingsNum       = weaknessRingsNumber;
    saveData.hungerRingsNumber      = hungerRingsNumber;
    saveData.regenRingsNumber       = regenRingsNumber;
    saveData.sicknessRingsNumber    = sicknessRingsNumber;
    saveData.aggravateRingsNumber   = aggravateRingsNumber;
    saveData.armorRingsNumber       = armorRingsNumber;
    saveData.indigestionRingsNumber = indigestionRingsNumber;
    saveData.teleportRingsNumber    = teleportRingsNumber;
    saveData.invisibleRingsNumber   = invisibleRingsNumber;

    for (int i = 0; i < NUM_INVENTORY_PAGES; i++)
        saveData.savedInventory[i] = inventoryPages[i];

    for (int i = 0; i < MAX_ENEMIES; i++) {
        const Enemy& e = enemies[i];
        SavedEnemy&  s = saveData.savedEnemies[i];
        s.x                 = e.x;
        s.y                 = e.y;
        s.hp                = e.hp;
        s.chasingPlayer     = e.chasingPlayer;
        s.moveAmount        = e.moveAmount;
        strncpy(s.name, e.name, sizeof(s.name) - 1);
        s.name[sizeof(s.name) - 1] = '\0';
        s.attackDelay       = e.attackDelay;
        s.damage            = e.damage;
        s.hasWanderPath     = e.hasWanderPath;
        s.pathLength        = e.pathLength;
        s.currentPathIndex  = e.currentPathIndex;
        memcpy(s.wanderPath, e.wanderPath, sizeof(s.wanderPath));
        s.attackDelayCounter= e.attackDelayCounter;
        s.nearClock         = e.nearClock;
        s.isFriend          = e.isFriend;
    }

    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            saveData.dungeonMap[y][x] = dungeonMap[y][x];

    for (int y = 0; y < NUM_SCROLLS; y++)
        for (int x = 0; x < 20; x++) {
            saveData.scrollNames[y][x]         = scrollNames[y][x];
            saveData.scrollNamesRevealed[y][x] = scrollNamesRevealed[y][x];
        }

    for (int i = 0; i < NUM_ITEMS; i++)
        saveData.itemList[i] = itemList[i];

    if (!saveGame(saveData))
        Serial.println("saveGame() failed");

    currentUIState = UI_NORMAL;
}

void tryLoadGame() {
    if (!loadGame(saveData)) {
        Serial.println("loadGame() failed");
        return;
    }

    equippedArmorValue        = saveData.armorValue;
    playerAttackDamage        = saveData.attackDamage;
    dungeon                   = saveData.currentDungeon;
    damsel[0]                 = saveData.damsel;
    endlessMode               = saveData.endlessMode;
    equippedArmor             = saveData.equippedArmor;
    equippedWeapon            = saveData.equippedWeapon;
    equippedRiddleStone       = saveData.equippedRiddleStone;
    succubusIsFriend          = saveData.succubusFriend;
    kills                     = saveData.kills;
    hasMap                    = saveData.hasMap;
    playerNearClockEnemy      = saveData.playerNearClockEnemy;
    knowsDamselName           = saveData.knowsDamselName;
    damselSayThanksForRescue  = saveData.damselSayThanksForRescue;
    damselGotTaken            = saveData.damselGotTaken;
    levelOfDamselDeath        = saveData.levelOfDamselDeath;
    keysCount                 = saveData.keysCount;
    goldCount                 = saveData.goldCount;

    playerX    = saveData.playerX;
    playerY    = saveData.playerY;
    playerHP   = saveData.hp;
    playerFood = saveData.food;

    if (equippedWeapon.item != Null && equippedWeapon.weapon.type != NoWeapon) {
        playerAttackDamage = (int)equippedWeapon.weapon.damage;
        attackDelayFrames  = equippedWeapon.weapon.attackDelay;
    } else {
        playerAttackDamage = PLAYER_BASE_ATTACK;
        attackDelayFrames  = DEFAULT_ATTACK_DELAY;
    }

    swiftnessRingsNumber    = saveData.swiftnessRingsNum;
    strengthRingsNumber     = saveData.strengthRingsNum;
    weaknessRingsNumber     = saveData.weaknessRingsNum;
    hungerRingsNumber       = saveData.hungerRingsNumber;
    regenRingsNumber        = saveData.regenRingsNumber;
    sicknessRingsNumber     = saveData.sicknessRingsNumber;
    aggravateRingsNumber    = saveData.aggravateRingsNumber;
    armorRingsNumber        = saveData.armorRingsNumber;
    indigestionRingsNumber  = saveData.indigestionRingsNumber;
    teleportRingsNumber     = saveData.teleportRingsNumber;
    invisibleRingsNumber    = saveData.invisibleRingsNumber;

    for (int i = 0; i < NUM_INVENTORY_PAGES; i++)
        inventoryPages[i] = saveData.savedInventory[i];

    // Restore enemies — fix up sprite pointer (was never serialised)
    auto spriteFor = [](const char* name) -> const unsigned char* {
        if (strcmp(name, "blob")      == 0) return blobAnimation[0].frame;
        if (strcmp(name, "teleporter")== 0) return teleporterAnimation[0].frame;
        if (strcmp(name, "batguy")    == 0) return batguyAnimation[0].frame;
        if (strcmp(name, "shooter")   == 0) return shooterAnimation[0].frame;
        if (strcmp(name, "clock")     == 0) return clockAnimation[0].frame;
        if (strcmp(name, "jukebox")   == 0) return jukeboxAnimation[0].frame;
        if (strcmp(name, "boss")      == 0) return bossIdleAnimation[0].frame;
        if (strcmp(name, "succubus")  == 0) return succubusIdleSprite;
        return nullptr;
    };

    for (int i = 0; i < MAX_ENEMIES; i++) {
        const SavedEnemy& s = saveData.savedEnemies[i];
        Enemy&            e = enemies[i];
        e.x                 = s.x;
        e.y                 = s.y;
        e.hp                = s.hp;
        e.chasingPlayer     = s.chasingPlayer;
        e.moveAmount        = s.moveAmount;
        strncpy(e.name, s.name, sizeof(e.name) - 1);
        e.name[sizeof(e.name) - 1] = '\0';
        e.attackDelay       = s.attackDelay;
        e.damage            = s.damage;
        e.hasWanderPath     = s.hasWanderPath;
        e.pathLength        = s.pathLength;
        e.currentPathIndex  = s.currentPathIndex;
        memcpy(e.wanderPath, s.wanderPath, sizeof(e.wanderPath));
        e.attackDelayCounter= s.attackDelayCounter;
        e.nearClock         = s.nearClock;
        e.isFriend          = s.isFriend;
        e.sprite            = spriteFor(e.name);
    }

    for (int y = 0; y < MAP_HEIGHT; y++)
        for (int x = 0; x < MAP_WIDTH; x++)
            dungeonMap[y][x] = saveData.dungeonMap[y][x];

    for (int y = 0; y < NUM_SCROLLS; y++)
        for (int x = 0; x < 20; x++) {
            scrollNames[y][x]         = saveData.scrollNames[y][x];
            scrollNamesRevealed[y][x] = saveData.scrollNamesRevealed[y][x];
        }

    for (int i = 0; i < NUM_ITEMS; i++)
        itemList[i] = saveData.itemList[i];

    randomSeed(saveData.worldSeed);
    currentUIState = UI_NORMAL;
}