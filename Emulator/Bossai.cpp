#include "Common.h"
#include "GameState.h"
#include "game.h"
#include "Sprites.h"
#include "Dungeon.h"
#include "Entities.h"
#include "HelperFunctions.h"
#include "Player.h"
#include "GameAudio.h"

#include <cmath>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// BossAI.cpp
//
// Contains only the boss fight state machine.
// No rendering, no UI, no game-over logic.
// All magic numbers come from Constants.h.
// ─────────────────────────────────────────────────────────────────────────────

// ── Module-local boss movement state ────────────────────────────────────────
namespace {
    float bossTargetX    = 0.0f;
    float bossTargetY    = 0.0f;
    float bossVelocityX  = 0.0f;
    float bossVelocityY  = 0.0f;
    float bossChargeSpeed= 0.0f;
    bool  bossIsCharging = false;
}

// ─────────────────────────────────────────────────────────────────────────────
void updateBossfight() {
    if (g_state.credits) return;

    // ── Phase tick ───────────────────────────────────────────────────────
    if (g_state.bossState != Beaten) {
        g_state.bossStateTimer++;
    }

    // ── HP-based state transitions ───────────────────────────────────────
    if (enemies[0].hp <= BOSS_ENRAGE_HP && enemies[0].hp > 0) {
        if (g_state.bossState != Enraged) {
            playWav1.stop();
            float volf = masterVolume / 10.0f;
            sgtl5000_1.volume(volf);
            mixer1.gain(0, 0.5f * volf);
            mixer1.gain(1, 0.5f * volf);
            mixer1.gain(2, 0.5f * volf);
            mixer1.gain(3, 0.5f * volf);
            musicMixer.gain(0, volf);
            musicMixer.gain(1, 0.2f * volf);
            playWav1.play("./Audio/alternateBossfight.wav");
        }
        g_state.bossState = Enraged;
    } else if (enemies[0].hp <= 0) {
        playWav1.stop();
        g_state.bossState = Beaten;
    }

    // ── Contact damage ───────────────────────────────────────────────────
    if (enemies[0].hp > 0) {
        bool touching =
            checkSpriteCollisionWithSprite(playerX, playerY, enemies[0].x,     enemies[0].y    ) ||
            checkSpriteCollisionWithSprite(playerX, playerY, enemies[0].x + 1, enemies[0].y    ) ||
            checkSpriteCollisionWithSprite(playerX, playerY, enemies[0].x,     enemies[0].y + 1) ||
            checkSpriteCollisionWithSprite(playerX, playerY, enemies[0].x + 1, enemies[0].y + 1);

        if (touching && g_state.bossStateTimer % BOSS_CONTACT_TICK == 0) {
            playerHP -= enemies[0].damage;
            triggerScreenShake(2, 2);
            checkIfDeadFrom(enemies[0].name);
        }
    }

    // ── Timer-based phase transitions (non-enraged) ──────────────────────
    if (g_state.bossState != Beaten && g_state.bossState != Enraged) {
        int t = g_state.bossStateTimer;
        if      (t == BOSS_PHASE_FLOATING)  { g_state.bossState = Floating;  enemies[0].moveAmount = BOSS_FLOAT_MOVE; }
        else if (t == BOSS_PHASE_SHOOTING)  { g_state.bossState = Shooting;  enemies[0].moveAmount = 0.0f; }
        else if (t == BOSS_PHASE_SUMMONING) { g_state.bossState = Summoning; enemies[0].moveAmount = 0.0f; }
        else if (t == BOSS_PHASE_SHOOT2)    { g_state.bossState = Shooting;  enemies[0].moveAmount = 0.0f; }
        else if (t == BOSS_PHASE_FLOAT2)    {
            g_state.bossState      = Floating;
            enemies[0].moveAmount  = BOSS_FLOAT_MOVE;
            g_state.bossStateTimer = 0;
        }
    } else if (g_state.bossState == Enraged) {
        enemies[0].moveAmount = BOSS_ENRAGE_MOVE;
    } else if (g_state.bossState == Beaten) {
        enemies[0].moveAmount = 0.0f;
        dungeonMap[(mapHeight / 2) + 4][(mapWidth / 2) - 3] = Exit;
        dungeonMap[(mapHeight / 2) + 4][(mapWidth / 2) + 3] = Freedom;
    }

    // ── Per-state AI ─────────────────────────────────────────────────────
    switch (g_state.bossState) {

    // ── Idle ─────────────────────────────────────────────────────────────
    case Idle:
        enemies[0].damage = BOSS_CONTACT_DAMAGE_NORMAL;
        if (strcmp(currentDialogue,
                   "You've amused me, little wizard. Time to die!") != 0) {
            currentDamselPortrait = bossPortraitIdle;
            dialogueTimeLength    = 3000;
            snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                     "%s", "You've amused me, little wizard. Time to die!");
            showDialogue = true;
        }
        break;

    // ── Floating ─────────────────────────────────────────────────────────
    case Floating: {
        enemies[0].damage = BOSS_CONTACT_DAMAGE_NORMAL;
        float volf = masterVolume / 10.0f;
            sgtl5000_1.volume(volf);
            mixer1.gain(0, 0.5f * volf);
            mixer1.gain(1, 0.5f * volf);
            mixer1.gain(2, 0.5f * volf);
            mixer1.gain(3, 0.5f * volf);
            musicMixer.gain(0, volf);
            musicMixer.gain(1, 0.2f * volf);
        if (!playWav1.isPlaying()) playWav1.play("./Audio/bossfight.wav");
        showDialogue       = false;
        dialogueTimeLength = 0;

        if (g_state.bossStateTimer % BOSS_SHOOT_INTERVAL_FLOAT == 0) {
            bossTargetX = (float)random(BOSS_WANDER_RANGE_MIN, BOSS_WANDER_RANGE_MAX_X);
            bossTargetY = (float)random(BOSS_WANDER_RANGE_MIN, BOSS_WANDER_RANGE_MAX_Y);
        }

        float dirX = bossTargetX - enemies[0].x;
        float dirY = bossTargetY - enemies[0].y;
        float dist = sqrtf(dirX * dirX + dirY * dirY);
        if (dist > 0.1f) {
            enemies[0].x += (dirX / dist) * enemies[0].moveAmount;
            enemies[0].y += (dirY / dist) * enemies[0].moveAmount;
        }

        if (g_state.bossStateTimer % BOSS_SHOOT_INTERVAL_FLOAT == 0) {
            float sdx = playerX - enemies[0].x;
            float sdy = playerY - enemies[0].y;
            float sd  = sqrtf(sdx * sdx + sdy * sdy);
            if (sd > 0.0f) {
                shootProjectile(enemies[0].x, enemies[0].y, sdx / sd, sdy / sd, false, 0);
                playRawSFX(1);
            }
        }
        break;
    }

    // ── Shooting ─────────────────────────────────────────────────────────
    case Shooting: {
        enemies[0].damage = 0;
        float volf = masterVolume / 10.0f;
            sgtl5000_1.volume(volf);
            mixer1.gain(0, 0.5f * volf);
            mixer1.gain(1, 0.5f * volf);
            mixer1.gain(2, 0.5f * volf);
            mixer1.gain(3, 0.5f * volf);
            musicMixer.gain(0, volf);
            musicMixer.gain(1, 0.2f * volf);
        if (!playWav1.isPlaying()) playWav1.play("./Audio/bossfight.wav");

        if (g_state.bossStateTimer % BOSS_SHOOT_INTERVAL_SHOOT == 0) {
            float sdx = playerX - enemies[0].x;
            float sdy = playerY - enemies[0].y;
            float sd  = sqrtf(sdx * sdx + sdy * sdy);
            if (sd > 0.0f) {
                shootProjectile(enemies[0].x, enemies[0].y, sdx / sd, sdy / sd, false, 0);
                playRawSFX(1);
            }
        }
        break;
    }

    // ── Enraged ──────────────────────────────────────────────────────────
    case Enraged: {
        enemies[0].damage = BOSS_CONTACT_DAMAGE_ENRAGED;
        float volf = masterVolume / 10.0f;
            sgtl5000_1.volume(volf);
            mixer1.gain(0, 0.5f * volf);
            mixer1.gain(1, 0.5f * volf);
            mixer1.gain(2, 0.5f * volf);
            mixer1.gain(3, 0.5f * volf);
            musicMixer.gain(0, volf);
            musicMixer.gain(1, 0.2f * volf);
        if (!playWav1.isPlaying()) playWav1.play("./Audio/alternateBossfight.wav");

        if (strcmp(currentDialogue, "AAGH! DIE, PEST!") != 0) {
            currentDamselPortrait = bossPortraitEnraged;
            dialogueTimeLength    = 300;
            snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                     "%s", "AAGH! DIE, PEST!");
            showDialogue = true;
        }

        float tdx   = playerX - enemies[0].x;
        float tdy   = playerY - enemies[0].y;
        float tdist = sqrtf(tdx * tdx + tdy * tdy);

        if (!bossIsCharging) {
            if (tdist > 0.0f) {
                float nx = tdx / tdist;
                float ny = tdy / tdist;
                bossVelocityX += (nx - bossVelocityX) * 0.03f;
                bossVelocityY += (ny - bossVelocityY) * 0.03f;
                float dot = bossVelocityX * nx + bossVelocityY * ny;
                if (dot > BOSS_CHARGE_ALIGN_DOT) {
                    bossIsCharging  = true;
                    bossChargeSpeed = enemies[0].moveAmount * BOSS_CHARGE_SPEED_MULT;
                }
            }
        } else {
            float velMagForDot = sqrtf(bossVelocityX * bossVelocityX +
                                       bossVelocityY * bossVelocityY);
            float dot = (velMagForDot > 0.0f && tdist > 0.0f)
                ? (tdx * bossVelocityX + tdy * bossVelocityY) / velMagForDot
                : 0.0f;

            bool missedTarget = (dot < 0.0f);
            bool hitWall =
                checkSpriteCollisionWithTileX(enemies[0].x + bossVelocityX,
                                              enemies[0].x, enemies[0].y) ||
                checkSpriteCollisionWithTileY(enemies[0].y + bossVelocityY,
                                              enemies[0].y, enemies[0].x);

            if (tdist < 1.0f || hitWall || missedTarget) {
                bossChargeSpeed *= (hitWall ? BOSS_CHARGE_SLOWDOWN_WALL
                                            : BOSS_CHARGE_SLOWDOWN_MISS);
                if (bossChargeSpeed < BOSS_CHARGE_STOP_SPEED) {
                    bossIsCharging  = false;
                    bossChargeSpeed = 0.0f;
                }
            }
        }

        float velMag = sqrtf(bossVelocityX * bossVelocityX +
                             bossVelocityY * bossVelocityY);
        if (velMag > 0.0f) {
            float spd = bossIsCharging ? bossChargeSpeed : enemies[0].moveAmount;
            enemies[0].x += (bossVelocityX / velMag) * spd;
            enemies[0].y += (bossVelocityY / velMag) * spd;
        }

        if (g_state.bossStateTimer % BOSS_SHOOT_INTERVAL_ENRAGE == 0 &&
            velMag > 0.0f) {
            shootProjectile(enemies[0].x, enemies[0].y,
                            bossVelocityX / velMag,
                            bossVelocityY / velMag, false, 0);
            playRawSFX(1);
        }
        break;
    }

    // ── Summoning ────────────────────────────────────────────────────────
    case Summoning: {
        enemies[0].damage = 0;
        float volf = masterVolume / 10.0f;
            sgtl5000_1.volume(volf);
            mixer1.gain(0, 0.5f * volf);
            mixer1.gain(1, 0.5f * volf);
            mixer1.gain(2, 0.5f * volf);
            mixer1.gain(3, 0.5f * volf);
            musicMixer.gain(0, volf);
            musicMixer.gain(1, 0.2f * volf);
        if (!playWav1.isPlaying()) playWav1.play("./Audio/bossfight.wav");

        if (g_state.bossStateTimer % BOSS_SUMMON_INTERVAL == 0) {
            int startSlot = succubusIsFriend ? 2 : 1;
            for (int j = startSlot; j < MAX_ENEMIES; j++) {
                if (enemies[j].hp > 0) continue;

                float angle  = random(0, 628) / 100.0f;
                float radius = (float)random(BOSS_SUMMON_RADIUS_MIN,
                                             BOSS_SUMMON_RADIUS_MAX + 1);
                int sx = (int)(enemies[0].x + cosf(angle) * radius);
                int sy = (int)(enemies[0].y + sinf(angle) * radius);

                if (sx < 0 || sx >= mapWidth || sy < 0 || sy >= mapHeight) break;
                if (dungeonMap[sy][sx] != Floor) break;

                int t = random(0, 9);
                if (t <= 3) {
                    enemies[j] = { (float)sx, (float)sy, 20, false, 0.05f, "blob",
                                   20, 2, false, 0, 0, {}, nullptr, 20, false, false };
                    enemies[j].sprite = blobAnimation[random(0, blobAnimationLength)].frame;
                } else if (t <= 5) {
                    enemies[j] = { (float)sx, (float)sy, 10, false, 0.08f, "batguy",
                                   20, 1, false, 0, 0, {}, nullptr, 20, false, false };
                    enemies[j].sprite = batguyAnimation[random(0, batguyAnimationLength)].frame;
                } else {
                    enemies[j] = { (float)sx, (float)sy, 15, false, 0.06f, "shooter",
                                   20, 0, false, 0, 0, {}, nullptr, 20, false, false };
                    enemies[j].sprite = shooterAnimation[random(0, shooterAnimationLength)].frame;
                }
                playRawSFX3D(14, enemies[j].x, enemies[j].y);
                break;
            }
        }
        break;
    }

    // ── Beaten ───────────────────────────────────────────────────────────
    case Beaten:
        if (succubusIsFriend) {
            dialogueTimeLength    = 300;
            currentDamselPortrait = succubusPortrait;
            snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                     "%s", "Heh... that was quite exhilarating.");
            showDialogue = true;
        } else if (damsel[0].active && !damsel[0].dead) {
            if (strcmp(currentDialogue, "You did it! You killed him!")      != 0 &&
                strcmp(currentDialogue,
                       "Please don't go- come be free, free with me!") != 0) {
                currentDamselPortrait = damselPortraitNormal;
                dialogueTimeLength    = 300;
                playRawSFX(18);
                snprintf(currentDialogue, sizeof(g_state.currentDialogue),
                         "%s", "You did it! You killed him!");
                showDialogue = true;
            }
        }
        break;

    default:
        break;
    }
}