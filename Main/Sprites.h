#ifndef SPRITES_H
#define SPRITES_H

#include <avr/pgmspace.h> // For PROGMEM

// Stair sprite
extern const unsigned char stairsSprite[];

// Wall sprite
extern const unsigned char wallSprite[];
extern const unsigned char wallSpriteDim[];

// Bars sprite
extern const unsigned char barsSprite[];

// Player sprites
extern const unsigned char playerSpriteRight[];
extern const unsigned char playerSpriteLeft[];
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
extern const unsigned char* blobSprite;

extern const unsigned char PROGMEM potionSprite[];
extern const unsigned char PROGMEM mapSprite[];

// Screens
extern const unsigned char rescueDamselScreen[];
extern const unsigned char deadDamselScreen[];
extern const unsigned char leftDamselScreen[];
extern const unsigned char aloneWizardScreen[];
extern const unsigned char capturedDamselScreen[];

#endif // SPRITES_H