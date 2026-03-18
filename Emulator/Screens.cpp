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
#include "SaveLogic.h"

#include <fstream>
#include <sstream>
#include <cstring>
#include <cmath>

// ─────────────────────────────────────────────────────────────────────────────
// Screens.cpp
//
// Every screen that is NOT the main gameplay view:
//   • Intro / splash / secret
//   • Credits
//   • Game over
//   • Status screen (between levels)
//
// Highscore helpers are file-local.
// ─────────────────────────────────────────────────────────────────────────────

// ── Highscore helpers ────────────────────────────────────────────────────────
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
// renderIntroScreen
// ─────────────────────────────────────────────────────────────────────────────
void renderIntroScreen() {
    display.setFont(Adafruit_GFX::builtin_font);
    g_state.introNum++;
    if (g_state.introNum > INTRO_MAX_FRAME) {
        g_state.introNum = INTRO_MAX_FRAME;
    }

    display.clearDisplay();
    display.setTextColor(15, 0);
    display.setTextSize(1);
    display.setCursor(45, 50);
    display.print("Paladin");
    display.setCursor(42, 60);
    display.print("Presents");
    display.display();

    if (!playWav1.isPlaying() && g_state.introNum <= INTRO_MUSIC_CUTOFF) {
        playWav1.play("./Audio/intro.wav");
    }
    if (!playWav1.isPlaying() && g_state.introNum > INTRO_MUSIC_CUTOFF) {
        playWav1.stop();
        g_state.currentUIState   = UI_SPLASH;
        playRawSFX(10);
        g_state.splashShakeFrames = SPLASH_SHAKE_FRAMES;
        g_state.introNum          = 0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// renderSecretScreen
// ─────────────────────────────────────────────────────────────────────────────
void renderSecretScreen() {
    display.clearDisplay();
    display.setTextColor(15, 0);
    display.setCursor(1, 7);
    display.print(
        "Hah! You bet I had to add the Konami sequence. Minus the start button. "
        "Anyway, yeah here's some hints. Equip the Riddle Stone. Read some of the "
        "scrolls right after drinking a See-All potion. Try leading a succubus "
        "through an exit. Lastly, don't try to see if the washer fits on your "
        "finger. Just don't.");
    display.display();
}

// ─────────────────────────────────────────────────────────────────────────────
// renderSplashScreen
// ─────────────────────────────────────────────────────────────────────────────
void renderSplashScreen() {
    // ── Konami code check ────────────────────────────────────────────────
    KonamiInput expected = konamiCode[konamiIndex];
    bool matched = false;
    const ButtonStates& b = g_state.buttons;

    switch (expected) {
        case K_UP:    matched = b.upPressed    && !b.upPressedPrev;    break;
        case K_DOWN:  matched = b.downPressed  && !b.downPressedPrev;  break;
        case K_LEFT:  matched = b.leftPressed  && !b.leftPressedPrev;  break;
        case K_RIGHT: matched = b.rightPressed && !b.rightPressedPrev; break;
        case K_B:     matched = b.bPressed     && !b.bPressedPrev;     break;
        case K_A:     matched = b.aPressed     && !b.aPressedPrev;     break;
        case K_START: matched = b.startPressed && !b.startPressedPrev; break;
    }

    if (matched) {
        g_state.konamiIndex++;
        if (g_state.konamiIndex >= konamiLength) {
            g_state.currentUIState = UI_SECRET;
            g_state.konamiIndex    = 0;
        }
    } else {
        bool anyNew = (b.upPressed    && !b.upPressedPrev)    ||
                      (b.downPressed  && !b.downPressedPrev)  ||
                      (b.leftPressed  && !b.leftPressedPrev)  ||
                      (b.rightPressed && !b.rightPressedPrev) ||
                      (b.aPressed     && !b.aPressedPrev)     ||
                      (b.bPressed     && !b.bPressedPrev)     ||
                      (b.startPressed && !b.startPressedPrev);
        if (anyNew) g_state.konamiIndex = 0;
    }

    if (!playWav1.isPlaying()) {
        playWav1.play("./Audio/title_screen.wav");
    }

    // ── Timing ───────────────────────────────────────────────────────────
    if (!g_state.splashTimingActive) {
        g_state.splashStartTime    = millis();
        g_state.splashTimingActive = true;
    }

    unsigned long elapsed = millis() - g_state.splashStartTime;

    // Frame sequence — each entry is the millisecond at which that splash becomes active
    static const unsigned long splashStartTimes[] = {
        0UL, 6500UL, 9800UL, 11500UL, 14700UL,
        18000UL, 21300UL, 24550UL, 27850UL, 31000UL
    };
    static const unsigned char* splashFrames[] = {
        splashScreen, batguy_splash, blob_splash, teleporter_splash, shooter_splash,
        jukebox_splash, succubus_splash, wizard_splash, damsel_splash, master_splash
    };
    static constexpr int splashCount =
        sizeof(splashFrames) / sizeof(splashFrames[0]);

    const unsigned long loopDuration = SPLASH_LOOP_DURATION_MS;
    const unsigned long now          = elapsed % loopDuration;

    // Determine which frame is currently active
    int currentIndex = 0;
    for (int i = 0; i < splashCount; ++i) {
        if (now >= splashStartTimes[i]) currentIndex = i;
    }
    int nextIndex    = (currentIndex + 1) % splashCount;
    unsigned long nextStart = (nextIndex == 0) ? loopDuration
                                               : splashStartTimes[nextIndex];

    // ── Slide-in transition ───────────────────────────────────────────────
    unsigned long transStart = (nextStart > SPLASH_TRANSITION_MS)
                               ? nextStart - SPLASH_TRANSITION_MS : 0UL;
    if (now >= transStart && now < nextStart) {
        float p     = (float)(now - transStart) / (float)SPLASH_TRANSITION_MS;
        if (p < 0.0f) p = 0.0f;
        if (p > 1.0f) p = 1.0f;
        float eased = p * p * p;   // cubic ease-in
        int   inX   = (int)((1.0f - eased) * (float)SCREEN_WIDTH);
        //int   outX  = inX - SCREEN_WIDTH;

        display.clearDisplay();
        display.drawBitmap(0, 0, splashFrames[currentIndex], SCREEN_WIDTH, SCREEN_HEIGHT, 15);
        display.drawBitmap(inX,  0, splashFrames[nextIndex],    SCREEN_WIDTH, SCREEN_HEIGHT, 15);
        display.display();
        return;
    }

    // ── Static frame ──────────────────────────────────────────────────────
    currentSplash = splashFrames[currentIndex];

    if (g_state.prevSplashIndex != -1 &&
        currentIndex != g_state.prevSplashIndex) {
        g_state.splashShakeFrames = SPLASH_SHAKE_FRAMES;
    }
    g_state.prevSplashIndex = currentIndex;

    display.clearDisplay();
    int shakeX = 0, shakeY = 0;
    if (g_state.splashShakeFrames > 0) {
        shakeX = random(-SPLASH_SHAKE_MAGNITUDE, SPLASH_SHAKE_MAGNITUDE + 1);
        shakeY = random(-SPLASH_SHAKE_MAGNITUDE, SPLASH_SHAKE_MAGNITUDE + 1);
        g_state.splashShakeFrames--;
    }
    display.drawBitmap(shakeX, shakeY, currentSplash, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
    if (currentSplash == splashScreen) {
        display.setCursor(61 + shakeX, 110 + shakeY);
        display.print("[ENTER]");
    }
    display.display();
}

// ─────────────────────────────────────────────────────────────────────────────
// renderCredits
// ─────────────────────────────────────────────────────────────────────────────
void renderCredits() {
    if (!playWav1.isPlaying()) {
        if (g_state.creditsBrightness > 0) {
            g_state.creditsBrightness--;
        }
    }

    display.clearDisplay();
    if (g_state.creditsBrightness > 0) {
        if (!damsel[0].dead && damsel[0].active && !succubusIsFriend && !DIDNOTRESCUEDAMSEL) {
            display.drawBitmap(0, 0, creditsDamselSaved, SCREEN_WIDTH, SCREEN_HEIGHT,
                               g_state.creditsBrightness);
        } else if (DIDNOTRESCUEDAMSEL && !succubusIsFriend) {
            display.drawBitmap(0, 0, creditsDamselNotSaved, SCREEN_WIDTH, SCREEN_HEIGHT,
                               g_state.creditsBrightness);
        } else if (succubusIsFriend) {
            display.drawBitmap(0, 0, creditsSuccubus, SCREEN_WIDTH, SCREEN_HEIGHT,
                               g_state.creditsBrightness);
        }
    }

    if (g_state.creditsBrightness == 0) {
        if (g_state.bossStateTimer < 15) {
            g_state.bossStateTimer++;
        }
        display.setTextSize(2);
        display.setCursor(22, 50);
        display.setTextColor(g_state.bossStateTimer, 0);
        display.print("The End");
    }
    display.display();
}

// ─────────────────────────────────────────────────────────────────────────────
// gameOver
// ─────────────────────────────────────────────────────────────────────────────
void gameOver() {
    static const char* chosenMessage = nullptr;
    const ButtonStates& b = g_state.buttons;

    // Reset chosen message when dismissed
    if (g_state.showDeathScreen &&
        ((b.bPressed && !b.bPressedPrev) || (b.aPressed && !b.aPressedPrev))) {
        chosenMessage = nullptr;
    }

    // ── Death screen ─────────────────────────────────────────────────────
    if (g_state.showDeathScreen) {
        display.clearDisplay();
        display.setCursor(0, 117);

        if      (strcmp(deathCause, "blob")      == 0) {
            display.drawBitmap(0, 0, wizardDeath_blob, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.print("Slain by a blob!");
        } else if (strcmp(deathCause, "batguy")    == 0) {
            display.drawBitmap(0, 0, wizardDeath_batguy, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.print("Slain by a batguy!");
        } else if (strcmp(deathCause, "succubus")  == 0) {
            display.drawBitmap(0, 0, wizardDeath_succubus, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.print("Slain by a succubus!");
        } else if (strcmp(deathCause, "shooter")   == 0) {
            display.drawBitmap(-10, 0, wizardDeath_shooter, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.print("Slain by a shooter!");
        } else if (strcmp(deathCause, "hunger")    == 0 ||
                   strcmp(deathCause, "poison")    == 0) {
            display.drawBitmap(0, 0, wizardDeath_hunger, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.print(strcmp(deathCause, "poison") == 0
                          ? "You died from poison!" : "You starved!");
        } else if (strcmp(deathCause, "stupidity") == 0) {
            display.drawBitmap(0, 0, wizardDeath_stupidity, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.setCursor(0, 107);
            display.print("You died of pure stupidity.");
        } else if (strcmp(deathCause, "boss")      == 0) {
            display.drawBitmap(0, 0, wizardDeath_boss, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.print("You failed.");
        } else {
            display.print("Yeah, idk what killed you.");
        }

        display.display();
        if ((b.bPressed && !b.bPressedPrev) || (b.aPressed && !b.aPressedPrev)) {
            g_state.showDeathScreen = false;
        }
        return;
    }

    // ── Stats screen ─────────────────────────────────────────────────────
    char Dngn[7],  KLLS[7],  DHighscore[7],  KHighscore[7];
    snprintf(Dngn, sizeof(Dngn), "%d", dungeon);
    snprintf(KLLS, sizeof(KLLS), "%d", kills);

    int dngnHighscore = 0, kllHighscore = 0;
    readHighscoresFromFile(dngnHighscore, kllHighscore);
    bool wrote = false;
    if (dungeon > dngnHighscore) { dngnHighscore = dungeon; wrote = true; }
    if (kills   > kllHighscore)  { kllHighscore  = kills;   wrote = true; }
    if (wrote) writeHighscoresToFile(dngnHighscore, kllHighscore);

    snprintf(DHighscore, sizeof(DHighscore), "%d", dngnHighscore);
    snprintf(KHighscore, sizeof(KHighscore), "%d", kllHighscore);

    display.clearDisplay();
    display.setCursor(7, 10);
    display.setTextSize(2);
    display.print("Gameover!");
    display.setTextSize(1);
    display.setCursor(5, 30);
    display.print("Press [X]");
    display.drawRect(8, 41, 110, 72, 15);

    // ── Taunt message ─────────────────────────────────────────────────────
    display.setCursor(12, 44);
    if (chosenMessage == nullptr) {
        struct TauntSet { int dungeon; const char* options[3]; };
        static const TauntSet taunts[] = {
            { 1,  {"get out lil bro",    "r u even trying",     "bruh"}            },
            { 2,  {"lol noob xD",        "nah -_-",             "L bozo"}          },
            { 3,  {"lame :/",            "get good bro",        "not even close"}  },
            { 4,  {"meh :(",             "cringe",              "dumb death tbh"}  },
            { 5,  {"not bad.",           "oof",                 "man"}             },
            { 6,  {"pretty good.",       "take a break.",       "long way to go"}  },
            { 7,  {"unlucky bro",        "skill issue",         "crazy"}           },
            { 8,  {"aw man.",            "aww, did u die?",     "splat"}           },
            { 9,  {"good run",           "but why...",          "aw."}             },
            { 10, {"noooo",              "*sigh*",              "ur pretty good"}  },
        };
        static const char* defaultOptions[3] = { "very nice.", "good job.", "*salute*" };

        const char** opts = defaultOptions;
        for (auto& t : taunts) {
            if (dungeon == t.dungeon) { opts = const_cast<const char**>(t.options); break; }
        }
        chosenMessage = opts[random(0, 3)];
    }
    display.print(chosenMessage);

    display.setCursor(12, 56);  display.print("On dungeon:");
    display.setCursor(78, 56);  display.print(Dngn);
    display.setCursor(12, 68);  display.print("Dngn highscore:");
    display.setCursor(102, 68); display.print(DHighscore);
    display.setCursor(12, 80);  display.print("Kills:");
    display.setCursor(48, 80);  display.print(KLLS);
    display.setCursor(12, 92);  display.print("Kll Highscore:");
    display.setCursor(96, 92);  display.print(KHighscore);

    display.display();

    if (b.bPressed && !b.bPressedPrev) {
        resetGame();
        g_state.deleteSV = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// showStatusScreen
// ─────────────────────────────────────────────────────────────────────────────
void showStatusScreen() {
    static bool damselKidnapScreen = false;
    const ButtonStates& b = g_state.buttons;

    display.clearDisplay();

    if (!nearSuccubus && !endlessMode) {
        if (!g_state.finalStatusScreen) {
            if (!succubusIsFriend && !nearSuccubus) {
                if (!damselKidnapScreen) {
                    if (dungeon > levelOfDamselDeath + 3) {
                        if (!damsel[0].dead && damsel[0].followingPlayer) {
                            if (!damsel[0].beingCarried) {
                                display.drawBitmap(0, -15, rescueDamselScreen,
                                                   SCREEN_WIDTH, SCREEN_HEIGHT, 15);
                            } else {
                                display.drawBitmap(0, -15, carryDamselScreen,
                                                   SCREEN_WIDTH, SCREEN_HEIGHT, 15);
                            }
                            display.setCursor(0, 107);
                            display.print("You rescued the Damsel!");
                        } else {
                            display.setCursor(0, 117);
                            display.print("Error.");
                        }
                    } else if (dungeon == levelOfDamselDeath) {
                        if (damsel[0].dead) {
                            display.drawBitmap(0, -15, deadDamselScreen,
                                               SCREEN_WIDTH, SCREEN_HEIGHT, 15);
                            display.setCursor(0, 77);
                            if (!knowsDamselName) {
                                display.print("The Damsel died!");
                            } else {
                                char msg[130];
                                snprintf(msg, sizeof(msg), "%s%s!",
                                         damselDeathMsg, damsel[0].name);
                                display.print(msg);
                            }
                            display.setCursor(0, 107);
                            display.print(damsel[0].levelOfLove >= 2
                                          ? "She trusted you!" : "How could you!");
                            if (damsel[0].levelOfLove >= 5) {
                                display.setCursor(0, 117);
                                display.print("She loved you!");
                            }
                        } else if (!damsel[0].dead && !damsel[0].followingPlayer &&
                                   !damsel[0].beingCarried) {
                            display.drawBitmap(0, 0, leftDamselScreen,
                                               SCREEN_WIDTH, SCREEN_HEIGHT, 15);
                            display.setCursor(0, 117);
                            if (!knowsDamselName) {
                                display.print("You left the Damsel!");
                            } else {
                                char msg[100];
                                snprintf(msg, sizeof(msg), "You left %s!",
                                         damsel[0].name);
                                display.print(msg);
                            }
                            g_state.leftDamsel = true;
                        }
                    } else {
                        display.drawBitmap(0, 0, aloneWizardScreen,
                                           SCREEN_WIDTH, SCREEN_HEIGHT, 15);
                        display.setCursor(0, 117);
                        display.print("You progress. Alone.");
                    }
                } else {
                    display.drawBitmap(0, 0, capturedDamselScreen,
                                       SCREEN_WIDTH, SCREEN_HEIGHT, 15);
                    display.setCursor(0, 2);
                    display.print("The Damsel was captured!");
                }
            }
        }
    } else if (endlessMode) {
        display.drawBitmap(0, 0, aloneWizardScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
        display.setCursor(0, 117);
        display.print("You progress. Alone.");
    }

    if (nearSuccubus && !g_state.finalStatusScreen) {
        if (!succubusIsFriend) {
            display.drawBitmap(0, 0, succubusFollowScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.setCursor(0, 117);
            display.print("She tried to kill you...");
        } else {
            display.drawBitmap(0, 0, succubusFollowScreen2, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.setCursor(0, 118);
            display.print("The succubus follows.");
        }
    }

    if (g_state.finalStatusScreen) {
        if (succubusIsFriend) {
            display.drawBitmap(0, 0, endScreenSuccubus, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.setCursor(0, 97);
            display.print("You defeated the master!");
            display.setCursor(0, 117);
            display.print("Have fun... ;)");
        } else if (!damsel[0].dead && damsel[0].active) {
            display.drawBitmap(0, 0, endScreenDamsel, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.setCursor(0, 97);
            display.print("You defeated the master!");
            display.setCursor(0, 117);
            display.print("And rescued the damsel!");
        } else {
            display.drawBitmap(0, 0, aloneWizardScreen, SCREEN_WIDTH, SCREEN_HEIGHT, 15);
            display.setCursor(0, 97);
            display.print("You defeated the master!");
            display.setCursor(0, 117);
            display.print("But are still alone.");
        }
    }

    display.display();

    // ── Button handling ───────────────────────────────────────────────────
    if (b.bPressed && !b.bPressedPrev) {
        if (damselKidnapScreen) {
            damselKidnapScreen      = false;
            g_state.statusScreen    = false;
            damselSayThanksForRescue = true;
            return;
        }

        if (g_state.statusScreen) {
            bool rescued       = damsel[0].active && !damsel[0].dead &&
                                 damsel[0].followingPlayer;
            bool wasBeingCarried = damsel[0].beingCarried;

            if (nearSuccubus) succubusIsFriend = true;

            dungeon++;
            playerDX = 0;
            playerDY = 1;
            g_state.statusScreen = false;

            generateDungeon(dungeon == bossfightLevel);
            showDialogue = false;
            for (int i = 0; i < MAX_PROJECTILES; i++) projectiles[i].active = false;
            spawnEnemies(dungeon == bossfightLevel);
            hasMap = false;

            // Love level adjustments
            damsel[0].levelOfLove += rescued ? 1 : 0;
            damsel[0].levelOfLove += (rescued && damselGotTaken)    ? 1 : 0;
            damsel[0].levelOfLove += (rescued && wasBeingCarried)   ? 1 : 0;

            // Carry state preservation
            if (dungeon == bossfightLevel) {
                damsel[0].beingCarried = false;
            } else {
                damsel[0].beingCarried = wasBeingCarried;
                if (wasBeingCarried) {
                    damsel[0].followingPlayer = true;
                    damsel[0].active          = true;
                }
            }

            damselGotTaken = rescued ? false : damselGotTaken;

            if (damsel[0].dead) {
                damsel[0].levelOfLove = 0;
                knowsDamselName = false;
                generateFemaleName(damsel[0].name, sizeof(damsel[0].name));
            }
            if (g_state.leftDamsel) {
                damsel[0].levelOfLove = 0;
                knowsDamselName = false;
                generateFemaleName(damsel[0].name, sizeof(damsel[0].name));
                g_state.leftDamsel = false;
            }

            // Kidnap roll (1-in-4 chance when rescued)
            if (rescued && random(1, 5) == 3 && !wasBeingCarried &&
                dungeon < bossfightLevel) {
                damselKidnapScreen   = true;
                g_state.statusScreen = true;
                damselGotTaken       = true;
            } else if (rescued && dungeon < bossfightLevel) {
                damsel[0].x = playerX;
                damsel[0].y = playerY - 1;
            }
        }

        if (g_state.finalStatusScreen && !g_state.credits) {
            g_state.credits      = true;
            g_state.bossStateTimer = 0;
            playWav1.stop();
            playWav2.stop();
            float volf = masterVolume / 10.0f;
            sgtl5000_1.volume(volf);
            mixer1.gain(0, 0.5f * volf);
            mixer1.gain(1, 0.5f * volf);
            mixer1.gain(2, 0.5f * volf);
            mixer1.gain(3, 0.5f * volf);
            musicMixer.gain(0, volf);
            musicMixer.gain(1, 0.2f * volf);
            playWav1.play("./Audio/endCredits.wav");
        }
    }
}