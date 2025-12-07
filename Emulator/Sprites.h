#ifndef SPRITES_H
#define SPRITES_H

#include <string>
#include <cstdint>
#include "Translation.h"

struct Frame {
  const unsigned char* frame;
  int length;
};

// Stair sprite
extern const unsigned char stairsSprite[];

// Wall sprite
extern const unsigned char wallSprite[];

// Bars sprite
extern const unsigned char barsSprite[];

// Door sprites
extern const unsigned char doorClosedSprite[];
extern const unsigned char doorOpenSprite[];

// Player sprites
extern const unsigned char playerSpriteRight[];
extern const unsigned char playerSpriteLeft[];
extern const unsigned char playerCarryingDamselSpriteRight[];
extern const unsigned char playerCarryingDamselSpriteLeft[];
extern const unsigned char* playerSprite;

// Damsel sprites
extern const unsigned char damselSpriteRight[];
extern const unsigned char damselSpriteLeft[];
extern const unsigned char damselHopefullSpriteRight[];
extern const unsigned char damselHopefullSpriteLeft[];
extern const unsigned char damselSpriteDead[];
extern const unsigned char* damselSprite;

// Blob sprites
extern const unsigned char blobSpriteFrame1[];
extern const unsigned char blobSpriteFrame2[];
extern Frame blobAnimation[];
extern const int blobAnimationLength;

// Jukebox sprites
extern const unsigned char jukeboxSpriteFrame1[];
extern const unsigned char jukeboxSpriteFrame2[];
extern const unsigned char jukeboxSpriteFrame3[];
extern const unsigned char jukeboxSpriteFrame4[];
extern const unsigned char jukeboxSpriteFrame5[];
extern Frame jukeboxAnimation[];
extern const int jukeboxAnimationLength;

// Batguy sprites
extern const unsigned char batguySpriteFrame1[];
extern const unsigned char batguySpriteFrame2[];
extern Frame batguyAnimation[];
extern const int batguyAnimationLength;

// Shooter sprites
extern const unsigned char shooterSpriteFrame1[];
extern const unsigned char shooterSpriteFrame2[];
extern Frame shooterAnimation[];
extern const int shooterAnimationLength;

// Item sprites
extern const unsigned char potionSprite[];
extern const unsigned char riddleStoneSprite[];
extern const unsigned char eyeSprite[];
extern const unsigned char mapSprite[];
extern const unsigned char mushroomSprite[];
extern const unsigned char armorSprite[];
extern const unsigned char ringSprite[];
extern const unsigned char chestSprite[];
extern const unsigned char scrollSprite[];
extern const unsigned char fastbootSprite[];
extern const unsigned char confusionSprite[];

// Teleporter sprites
extern const unsigned char teleporterSpriteF1[];
extern const unsigned char teleporterSpriteF2[];
extern const unsigned char teleporterSpriteF3[];
extern const unsigned char teleporterSpriteF4[];
extern Frame teleporterAnimation[];
extern const int teleporterAnimationLength;

extern const unsigned char succubusIdleSprite[];
extern const unsigned char succubusIdleSpriteFlipped[];

// Clock sprites
extern const unsigned char clockSpriteF1[];
extern const unsigned char clockSpriteF2[];
extern const unsigned char clockSpriteF3[];
extern const unsigned char clockSpriteF4[];
extern const unsigned char clockSpriteF5[];
extern const unsigned char clockSpriteF6[];
extern const unsigned char clockSpriteF7[];
extern const unsigned char clockSpriteF8[];
extern Frame clockAnimation[];
extern const int clockAnimationLength;

extern const unsigned char keySprite[];
extern const unsigned char lockedSprite[];

// Boss sprites
extern const unsigned char bossIdleSpriteF0[];
extern const unsigned char bossIdleSpriteF1[];
extern const unsigned char bossIdleSpriteF2[];
extern const unsigned char bossIdleSpriteF3[];
extern Frame bossIdleAnimation[];
extern const unsigned char bossIdleSpriteFlippedF0[];
extern const unsigned char bossIdleSpriteFlippedF1[];
extern const unsigned char bossIdleSpriteFlippedF2[];
extern const unsigned char bossIdleSpriteFlippedF3[];
extern Frame bossIdleAnimationFlipped[];
extern const int bossIdleAnimationLength;
extern const unsigned char bossFightSpriteFlippedF0[];
extern const unsigned char bossFightSpriteFlippedF1[];
extern const unsigned char bossFightSpriteFlippedF2[];
extern const unsigned char bossFightSpriteFlippedF3[];
extern Frame bossFightAnimation[];
extern const int bossFightAnimationLength;
extern const unsigned char bossBeatenSpriteF0[];
extern const unsigned char bossBeatenSpriteF1[];
extern const unsigned char bossBeatenSpriteF2[];
extern const unsigned char bossBeatenSpriteF3[];
extern const unsigned char bossBeatenSpriteF4[];
extern Frame bossBeatenAnimation[];
extern const int bossBeatenAnimationLength;


// Portraits
extern const unsigned char damselPortraitNormal[];
extern const unsigned char damselPortraitAlone[];
extern const unsigned char damselPortraitScared[];
extern const unsigned char damselPortraitDying [];
extern const unsigned char damselPortraitCarrying[];
extern const unsigned char* currentDamselPortrait;
extern const unsigned char succubusPortrait[];
extern const unsigned char bossPortraitIdle[];
extern const unsigned char bossPortraitEnraged[];

// Screens
extern const unsigned char rescueDamselScreen[];
extern const unsigned char deadDamselScreen[];
extern const unsigned char leftDamselScreen[];
extern const unsigned char aloneWizardScreen[];
extern const unsigned char capturedDamselScreen[];
extern const unsigned char carryDamselScreen [];
extern const unsigned char wizardDeath_blob [];
extern const unsigned char wizardDeath_batguy [];
extern const unsigned char wizardDeath_shooter [];
extern const unsigned char wizardDeath_hunger [];
extern const unsigned char wizardDeath_stupidity [];
extern const unsigned char wizardDeath_boss [];
extern const unsigned char wizardDeath_succubus [];
extern const unsigned char succubusFollowScreen [];
extern const unsigned char succubusFollowScreen2 [];
extern const unsigned char endScreenDamsel [];
extern const unsigned char endScreenSuccubus [];
extern const unsigned char creditsDamselSaved [];
extern const unsigned char creditsDamselNotSaved [];
extern const unsigned char creditsSuccubus [];
extern const unsigned char splashScreen [];
extern const unsigned char succubus_splash [];
extern const unsigned char shooter_splash [];
extern const unsigned char teleporter_splash [];
extern const unsigned char jukebox_splash [];
extern const unsigned char master_splash [];
extern const unsigned char wizard_splash [];
extern const unsigned char damsel_splash [];
extern const unsigned char blob_splash [];
extern const unsigned char batguy_splash [];
extern const unsigned char* currentSplash;

#endif // SPRITES_H