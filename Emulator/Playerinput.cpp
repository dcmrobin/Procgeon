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
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// PlayerInput.cpp
//
// Everything driven directly by button presses each frame:
//   • Movement (with confusion inversion, paralysis gate, diagonal normalisation)
//   • Tile pickup (potions, food, scrolls, rings, armour, weapons, keys, gold)
//   • Door open/close, chest open, exit/freedom descent
//   • Melee arc computation and damage application
//   • Projectile firing
//   • Reload bar advancement
// ─────────────────────────────────────────────────────────────────────────────

// ── forward declaration (defined in PlayerEffects.cpp) ───────────────────────
void startCarryingDamsel(bool resetPickupTimer);

// ─────────────────────────────────────────────────────────────────────────────
// handleInput
// ─────────────────────────────────────────────────────────────────────────────
void handleInput() {
    const ButtonStates& b = g_state.buttons;

    float newX = playerX;
    float newY = playerY;

    // Base speed, modified by swiftness rings
    float baseSpeed = PLAYER_BASE_SPEED + (swiftnessRingsNumber * PLAYER_SPEED_PER_RING);
    float speed     = speeding ? baseSpeed * currentSpeedMultiplier : baseSpeed;
    if (speed <= 0.0f) speed = 0.01f;

    playerActed = false;

    float diagSpeed = speed * PLAYER_DIAG_FACTOR;

    // Potentially inverted buttons (confusion)
    bool upPressed    = b.upPressed;
    bool downPressed  = b.downPressed;
    bool leftPressed  = b.leftPressed;
    bool rightPressed = b.rightPressed;
    if (confused) {
        bool tmp   = upPressed;  upPressed    = downPressed; downPressed  = tmp;
        tmp        = leftPressed; leftPressed = rightPressed; rightPressed = tmp;
    }

    // ── Directional movement ──────────────────────────────────────────────
    if      (upPressed    && !leftPressed  && !rightPressed) {
        if (!paralyzed) { playerDY = -1; playerDX = 0; newY -= speed; }
        playerActed = true;
    } else if (downPressed  && !leftPressed  && !rightPressed) {
        if (!paralyzed) { playerDY =  1; playerDX = 0; newY += speed; }
        playerActed = true;
    } else if (leftPressed  && !upPressed    && !downPressed) {
        if (!paralyzed) {
            playerDX = -1; playerDY = 0; newX -= speed;
            playerSprite = damsel[0].beingCarried
                           ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
        }
        playerActed = true;
    } else if (rightPressed && !upPressed    && !downPressed) {
        if (!paralyzed) {
            playerDX =  1; playerDY = 0; newX += speed;
            playerSprite = damsel[0].beingCarried
                           ? playerCarryingDamselSpriteRight : playerSpriteRight;
        }
        playerActed = true;
    } else if (upPressed    && leftPressed) {
        if (!paralyzed) {
            playerDY = -1; playerDX = -1;
            newY -= diagSpeed; newX -= diagSpeed;
            playerSprite = damsel[0].beingCarried
                           ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
        }
        playerActed = true;
    } else if (upPressed    && rightPressed) {
        if (!paralyzed) {
            playerDY = -1; playerDX = 1;
            newY -= diagSpeed; newX += diagSpeed;
            playerSprite = damsel[0].beingCarried
                           ? playerCarryingDamselSpriteRight : playerSpriteRight;
        }
        playerActed = true;
    } else if (downPressed  && leftPressed) {
        if (!paralyzed) {
            playerDX = -1; playerDY = 1;
            newX -= diagSpeed; newY += diagSpeed;
            playerSprite = damsel[0].beingCarried
                           ? playerCarryingDamselSpriteLeft : playerSpriteLeft;
        }
        playerActed = true;
    } else if (downPressed  && rightPressed) {
        if (!paralyzed) {
            playerDX = 1; playerDY = 1;
            newX += diagSpeed; newY += diagSpeed;
            playerSprite = damsel[0].beingCarried
                           ? playerCarryingDamselSpriteRight : playerSpriteRight;
        }
        playerActed = true;
    }

    playerMoving = b.upPressed || b.downPressed ||
                   b.leftPressed || b.rightPressed;

    // ── Carry damsel ──────────────────────────────────────────────────────
    {
        float dx = playerX - damsel[0].x;
        float dy = playerY - damsel[0].y;

        if (!damsel[0].beingCarried) {
            if (b.bPressed &&
                (dx * dx + dy * dy) <= 0.4f &&
                !damsel[0].dead &&
                damsel[0].levelOfLove >= 6) {
                playerActed = true;
                startCarryingDamsel(false);
            } else {
                startCarryingDamsel(true);
            }
        } else {
            if (b.bPressed) {
                playerActed = true;
                startCarryingDamsel(false);
            } else {
                startCarryingDamsel(true);
            }
        }
    }

    // ── Shoot / melee ─────────────────────────────────────────────────────
    if (b.bPressed && equippedWeapon.weapon.type != NoWeapon) {
        playerActed = true;
        float dx = playerX - damsel[0].x;
        float dy = playerY - damsel[0].y;
        bool tooCloseToCarry = damsel[0].beingCarried &&
                               (dx * dx + dy * dy) <= 0.3f;

        bool staffBlocked = (equippedWeapon.weapon.type == MagicStaff) && tooCloseToCarry;

        if (!reloading && !staffBlocked) {
            WeaponType wt = equippedWeapon.weapon.type;

            if (wt == MagicStaff) {
                // Pure ranged
                shootProjectile(playerX, playerY, (float)playerDX, (float)playerDY, true, -1);
                playRawSFX(1);
                reloading         = true;
                shootDelay        = 0;
                attackDelayFrames = equippedWeapon.weapon.attackDelay;

            } else {
                // Melee (optionally also fires a projectile for magic variants)
                if (wt == MagicDagger || wt == MagicSword || wt == MagicLongSword) {
                    shootProjectile(playerX, playerY, (float)playerDX, (float)playerDY, true, -1);
                    playRawSFX(1);
                }

                // ── Melee arc ─────────────────────────────────────────────
                meleeFrames   = MELEE_DURATION_FRAMES;
                int cx = round(playerX);
                int cy = round(playerY);
                meleeArcCount = 0;

                // Always include player tile
                g_state.meleeArcTilesX[meleeArcCount] = cx;
                g_state.meleeArcTilesY[meleeArcCount] = cy;
                meleeArcCount++;

                // Arc offsets in the facing direction
                int ox[3], oy[3];
                int dx = playerDX, dy = playerDY;
                if      (dx ==  0 && dy == -1) { ox[0]=0;oy[0]=-1; ox[1]=-1;oy[1]=-1; ox[2]=1;oy[2]=-1; }
                else if (dx ==  0 && dy ==  1) { ox[0]=0;oy[0]=1;  ox[1]=-1;oy[1]=1;  ox[2]=1;oy[2]=1;  }
                else if (dx == -1 && dy ==  0) { ox[0]=-1;oy[0]=0; ox[1]=-1;oy[1]=-1; ox[2]=-1;oy[2]=1; }
                else if (dx ==  1 && dy ==  0) { ox[0]=1;oy[0]=0;  ox[1]=1;oy[1]=-1;  ox[2]=1;oy[2]=1;  }
                else if (dx ==  1 && dy == -1) { ox[0]=1;oy[0]=-1; ox[1]=0;oy[1]=-1;  ox[2]=1;oy[2]=0;  }
                else if (dx == -1 && dy == -1) { ox[0]=-1;oy[0]=-1;ox[1]=0;oy[1]=-1;  ox[2]=-1;oy[2]=0; }
                else if (dx ==  1 && dy ==  1) { ox[0]=1;oy[0]=1;  ox[1]=1;oy[1]=0;   ox[2]=0;oy[2]=1;  }
                else if (dx == -1 && dy ==  1) { ox[0]=-1;oy[0]=1; ox[1]=-1;oy[1]=0;  ox[2]=0;oy[2]=1;  }
                else                           { ox[0]=0;oy[0]=-1; ox[1]=-1;oy[1]=-1; ox[2]=1;oy[2]=-1; }

                int weaponRange = (equippedWeapon.item != Null)
                                  ? equippedWeapon.weapon.range : 1;

                // Collect unique enemies hit by the arc
                bool hitSeen[MAX_ENEMIES] = {};
                int  hitIndices[MAX_ENEMIES];
                int  hitCount = 0;

                for (int i = 0; i < 3; i++) {
                    for (int d = 1; d <= weaponRange; d++) {
                        int tx = cx + ox[i] * d;
                        int ty = cy + oy[i] * d;
                        if (meleeArcCount < MAX_MELEE_TILES) {
                            g_state.meleeArcTilesX[meleeArcCount] = tx;
                            g_state.meleeArcTilesY[meleeArcCount] = ty;
                            meleeArcCount++;
                        }
                        if (tx < 0 || tx >= mapWidth || ty < 0 || ty >= mapHeight) continue;
                        for (int e = 0; e < MAX_ENEMIES; e++) {
                            if (enemies[e].hp > 0 &&
                                round(enemies[e].x) == tx &&
                                round(enemies[e].y) == ty &&
                                !hitSeen[e]) {
                                hitSeen[e]          = true;
                                hitIndices[hitCount++] = e;
                            }
                        }
                    }
                }

                // Also check player's own tile
                for (int e = 0; e < MAX_ENEMIES; e++) {
                    if (enemies[e].hp > 0 &&
                        round(enemies[e].x) == cx &&
                        round(enemies[e].y) == cy &&
                        !hitSeen[e]) {
                        hitSeen[e]          = true;
                        hitIndices[hitCount++] = e;
                    }
                }

                // Distribute damage evenly across hit enemies
                if (hitCount > 0) {
                    int base = playerAttackDamage / hitCount;
                    int rem  = playerAttackDamage % hitCount;
                    for (int h = 0; h < hitCount; h++) {
                        int idx = hitIndices[h];
                        int dmg = base + (h < rem ? 1 : 0);
                        if (strcmp(enemies[idx].name, "shopkeeper") != 0 && !enemies[idx].isFriend) {
                            enemies[idx].hp -= dmg;
                            spawnParticles(enemies[idx].x, enemies[idx].y, 1, 0.15f, false);
                            playRawSFX3D(23, enemies[idx].x, enemies[idx].y);
                            if (enemies[idx].hp <= 0) {
                                kills++;
                                spawnParticles(enemies[idx].x, enemies[idx].y, 5, 0.25f, true);
                            }
                            if (enemies[idx].hp <= 0 && random(0, 100) < 15 && dungeonMap[(int)round(enemies[idx].y)][(int)round(enemies[idx].x)] == Floor) dungeonMap[(int)round(enemies[idx].y)][(int)round(enemies[idx].x)] = MushroomTile;
                        }
                    }
                }

                playRawSFX(3);
                reloading         = true;
                shootDelay        = 0;
                attackDelayFrames = equippedWeapon.weapon.attackDelay;
            }
            playerActed = true;
        }
    }

    // ── Reload bar advancement ────────────────────────────────────────────
    if (playerActed && reloading) {
        shootDelay++;
        reloadBarWidth = shootDelay * (128 / attackDelayFrames);
        if (shootDelay >= attackDelayFrames) {
            reloading      = false;
            shootDelay     = 0;
            reloadBarWidth = 0;
        }
    }

    // ── Melee animation countdown ─────────────────────────────────────────
    if (playerActed && meleeFrames > 0) {
        meleeFrames--;
    }

    // ── Tile collision & pickup ───────────────────────────────────────────
    int rNewX = round(newX);
    int rNewY = round(newY);

    auto tryMove = [&]() {
        TileTypes t = dungeonMap[rNewY][rNewX];
        if (t == Floor    || t == Exit   || t == StartStairs ||
            t == Freedom  || t == DoorOpen || t == KeyTile) {
            playerX = newX;
            playerY = newY;
            return;
        }
        switch (t) {
            case Potion:
                if (addToInventory(getItem(getRandomPotion(random(6), true)), false)) {
                    playRawSFX(3);
                    dungeonMap[rNewY][rNewX] = Floor;
                }
                break;
            case Map:
                playRawSFX(3);
                hasMap = true;
                dungeonMap[rNewY][rNewX] = Floor;
                break;
            case MushroomTile: {
                GameItems choices[] = { Mushroom, Bread, Cheese, StaleMeat, Berries };
                int weights[]       = { 3, 2, 2, 2, 1 };
                int total = 0;
                for (int w : weights) total += w;
                int roll = random(0, total), cum = 0;
                GameItems chosen = Mushroom;
                for (int i = 0; i < 5; i++) {
                    cum += weights[i];
                    if (roll < cum) { chosen = choices[i]; break; }
                }
                if (addToInventory(getItem(chosen), false)) {
                    playRawSFX(3);
                    dungeonMap[rNewY][rNewX] = Floor;
                }
                break;
            }
            case WeaponTile: {
                GameItem weapon = getItem(Weapon);
                weapon.weapon   = weaponList[random(0, 4)];
                weapon.isCursed = (random(0, 100) < 10);
                if (addToInventory(weapon, false)) {
                    playRawSFX(3);
                    dungeonMap[rNewY][rNewX] = Floor;
                }
                break;
            }
            case RiddleStoneTile:
                if (addToInventory(getItem(RiddleStone), true)) {
                    playRawSFX(3);
                    dungeonMap[rNewY][rNewX] = Floor;
                }
                break;
            case ArmorTile: {
                // Weighted armour selection by rarity
                GameItems armorTypes[] = {
                    LeatherArmor, IronArmor, MagicRobe, Cloak,
                    ChaosArmor, RingMailArmor, DenimJacket, Trenchcoat, SpikyArmor
                };
                const int armorCount = sizeof(armorTypes) / sizeof(armorTypes[0]);
                const int maxRarity  = 5;
                int totalWeight = 0;
                for (int i = 0; i < armorCount; i++)
                    totalWeight += (maxRarity + 1) - getItem(armorTypes[i]).rarity;
                int r = random(0, totalWeight), cum = 0;
                GameItems chosen = LeatherArmor;
                for (int i = 0; i < armorCount; i++) {
                    cum += (maxRarity + 1) - getItem(armorTypes[i]).rarity;
                    if (r < cum) { chosen = armorTypes[i]; break; }
                }
                if (addToInventory(getItem(chosen), true)) {
                    playRawSFX(3);
                    dungeonMap[rNewY][rNewX] = Floor;
                }
                break;
            }
            case ScrollTile:
                if (addToInventory(getItem(Scroll), false)) {
                    playRawSFX(3);
                    dungeonMap[rNewY][rNewX] = Floor;
                }
                break;
            case RingTile:
                if (addToInventory(getItem(Ring), false)) {
                    playRawSFX(3);
                    dungeonMap[rNewY][rNewX] = Floor;
                }
                break;
            case KeyItem:
                keysCount++;
                playRawSFX(3);
                dungeonMap[rNewY][rNewX] = Floor;
                break;
            case GoldTile:
                goldCount++;
                playRawSFX(3);
                dungeonMap[rNewY][rNewX] = Floor;
                break;
            default:
                break;  // wall — don't move
        }
    };
    tryMove();

    // ── Door / chest / exit interactions (B press) ────────────────────────
    if (b.bPressed && !b.bPressedPrev) {
        int rPx = round(playerX);
        int rPy = round(playerY);
        int tx  = rPx + playerDX;
        int ty  = rPy + playerDY;

        // Fallback: no direction set yet — scan adjacent tiles for chests
        if (playerDX == 0 && playerDY == 0) {
            for (int ddx = -1; ddx <= 1; ddx++) {
                for (int ddy = -1; ddy <= 1; ddy++) {
                    int cx = rPx + ddx;
                    int cy = rPy + ddy;
                    if (cx >= 0 && cx < mapWidth &&
                        cy >= 0 && cy < mapHeight &&
                        dungeonMap[cy][cx] == ChestTile) {
                        playRawSFX(12);
                        OpenChest(cy, cx, ddy);
                        return;
                    }
                }
            }
            return;
        }

        if (tx < 0 || tx >= mapWidth || ty < 0 || ty >= mapHeight) return;

        switch (dungeonMap[ty][tx]) {
            case DoorClosed:
                dungeonMap[ty][tx] = DoorOpen;
                playRawSFX(12);
                break;
            case DoorOpen:
                dungeonMap[ty][tx] = DoorClosed;
                playRawSFX(13);
                break;
            case ChestTile:
                playRawSFX(12);
                OpenChest(ty, tx, playerDY);
                break;
            case Exit:
                playRawSFX(11);
                if (dungeon != bossfightLevel) {
                    if (!damsel[0].dead && !damsel[0].followingPlayer && damsel[0].active) {
                        levelOfDamselDeath = dungeon;
                        damsel[0].active   = false;
                    }
                    statusScreen = true;
                } else {
                    if (damsel[0].levelOfLove >= 8 && !damsel[0].dead) {
                        currentDamselPortrait = damselPortraitScared;
                        snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                                 "%s", "Please don't go- come be free, free with me!");
                        showDialogue       = true;
                        dialogueTimeLength = 400;
                    } else {
                        playRawSFX(11);
                        statusScreen              = true;
                        endlessMode               = true;
                        damsel[0].active          = false;
                        damsel[0].levelOfLove     = 0;
                        damsel[0].x               = -3000.0f;
                        damsel[0].y               = -3000.0f;
                        damsel[0].beingCarried    = false;
                        damsel[0].followingPlayer = false;
                        damsel[0].completelyRescued = false;
                    }
                }
                break;
            case KeyTile:
                if (keysCount > 0) {
                    keysCount--;
                    playRawSFX(22);
                    dungeonMap[ty][tx] = Exit;
                } else {
                    playRawSFX(13);
                    currentDamselPortrait = nullptr;
                    snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                             "%s", "The exit is locked! Find a key.");
                    showDialogue       = true;
                    dialogueTimeLength = 70;
                }
                break;
            case Freedom:
                playRawSFX(11);
                statusScreen      = true;
                finalStatusScreen = true;
                bossStateTimer    = 0;
                nearSuccubus      = false;
                break;
            case Kiosk:
                playRawSFX(12);
                g_state.currentUIState = UI_SHOP;
                break;
            default:
                break;
        }
    }
}