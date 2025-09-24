#ifndef SPRITES_H
#define SPRITES_H

#include <avr/pgmspace.h> // For PROGMEM

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

// Portraits
extern const unsigned char damselPortraitNormal[];
extern const unsigned char damselPortraitAlone[];
extern const unsigned char damselPortraitScared[];
extern const unsigned char damselPortraitDying [];
extern const unsigned char damselPortraitCarrying[];
extern const unsigned char* currentDamselPortrait;
extern const unsigned char succubusPortrait[];

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

#endif // SPRITES_H