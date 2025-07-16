#ifndef SPRITES_H
#define SPRITES_H

#include <string>
#include <cstdint>

// Stair sprite
extern const uint8_t stairsSprite[];

// Wall sprite
extern const uint8_t wallSprite[];

// Bars sprite
extern const uint8_t barsSprite[];

// Player sprites
extern const uint8_t playerSpriteRight[];
extern const uint8_t playerSpriteLeft[];
extern const uint8_t playerCarryingDamselSpriteRight[];
extern const uint8_t playerCarryingDamselSpriteLeft[];
extern const uint8_t* playerSprite;

// Damsel sprites
extern const uint8_t damselSpriteRight[];
extern const uint8_t damselSpriteLeft[];
extern const uint8_t damselHopefullSpriteRight[];
extern const uint8_t damselHopefullSpriteLeft[];
extern const uint8_t damselSpriteDead[];
extern const uint8_t* damselSprite;

// Blob sprites
extern const uint8_t blobSpriteFrame1[];
extern const uint8_t blobSpriteFrame2[];
extern const uint8_t* blobSprite;

// Item sprites
extern const uint8_t potionSprite[];
extern const uint8_t riddleStoneSprite[];
extern const uint8_t eyeSprite[];
extern const uint8_t mapSprite[];
extern const uint8_t mushroomSprite[];
extern const uint8_t armorSprite[];
extern const uint8_t ringSprite[];
extern const uint8_t chestSprite[];
extern const uint8_t scrollSprite[];
extern const uint8_t fastbootSprite[];
extern const uint8_t confusionSprite[];

// Teleporter sprites
extern const uint8_t teleporterSpriteF1[];
extern const uint8_t teleporterSpriteF2[];
extern const uint8_t teleporterSpriteF3[];
extern const uint8_t teleporterSpriteF4[];
extern const uint8_t* teleporterSprite;

// Portraits
extern const uint8_t damselPortraitNormal[];
extern const uint8_t damselPortraitAlone[];
extern const uint8_t damselPortraitScared[];
extern const uint8_t damselPortraitDying [];
extern const uint8_t damselPortraitCarrying[];
extern const uint8_t* currentDamselPortrait;

// Screens
extern const uint8_t rescueDamselScreen[];
extern const uint8_t deadDamselScreen[];
extern const uint8_t leftDamselScreen[];
extern const uint8_t aloneWizardScreen[];
extern const uint8_t capturedDamselScreen[];
extern const uint8_t carryDamselScreen [];

#endif // SPRITES_H