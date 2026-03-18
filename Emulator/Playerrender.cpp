#include "Common.h"
#include "GameState.h"
#include "Sprites.h"
#include "Dungeon.h"
#include "Entities.h"
#include "HelperFunctions.h"
#include "Item.h"
#include "GameAudio.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ─────────────────────────────────────────────────────────────────────────────
// PlayerRender.cpp
//
// Responsible for: drawing the player sprite, the melee swipe arc,
// contextual interaction prompts ("Open Chest [X]", "Descend [X]", etc.),
// and nudging the viewport offset when the player approaches an edge.
//
// No input, no physics, no game-logic side-effects.
// ─────────────────────────────────────────────────────────────────────────────

// ── Local helper: draw the animated melee swipe arc ─────────────────────────
static void drawSwipe(float px, float py, float dx, float dy, float fade) {
    if (dx == 0.0f && dy == 0.0f) return;

    int weaponRange = (equippedWeapon.item != Null) ? equippedWeapon.weapon.range : 1;

    // Perpendicular vector
    float perpX = -dy;
    float perpY =  dx;
    float perpLen = sqrtf(perpX * perpX + perpY * perpY);
    if (perpLen > 0.0f) { perpX /= perpLen; perpY /= perpLen; }

    const int numLines = 3;
    const int segments = 20;

    for (int line = 0; line < numLines; line++) {
        float lineDist    = weaponRange * tileSize * (1.0f - line * 0.3f);
        float baseX       = px + dx * lineDist;
        float baseY       = py + dy * lineDist;
        float curveHeight = weaponRange * tileSize * (1.0f - line * 0.2f);
        float lineFade    = fade * (1.0f - line * 0.3f);

        for (int i = 0; i < segments; i++) {
            float t   = i / float(segments - 1);
            float pos = -1.0f + 2.0f * t;
            float curveOffset = sinf(pos * (float)M_PI / 2.0f) * (2.0f - line * 0.5f);
            float offset      = pos * curveHeight;
            float pointX = baseX + perpX * offset + dx * curveOffset;
            float pointY = baseY + perpY * offset + dy * curveOffset;

            float brightnessFactor = lineFade * (0.3f + 0.7f * (1.0f - fabsf(pos)));
            uint8_t brightness = (uint8_t)(15.0f * brightnessFactor);

            if (brightness > 1) {
                display.drawPixel((int)pointX, (int)pointY, brightness);
                if (line == 0 && i % 3 == 0 && brightness > 8) {
                    display.drawPixel((int)(pointX + perpY * 0.5f),
                                      (int)(pointY - perpX * 0.5f), brightness / 3);
                    display.drawPixel((int)(pointX - perpY * 0.5f),
                                      (int)(pointY + perpX * 0.5f), brightness / 3);
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// renderPlayer
// ─────────────────────────────────────────────────────────────────────────────
void renderPlayer() {
    float screenX = (playerX - offsetX) * tileSize;
    float screenY = (playerY - offsetY) * tileSize;

    if (screenX >= 0 && screenX < SCREEN_WIDTH &&
        screenY >= 0 && screenY < SCREEN_HEIGHT) {

        // ── Draw sprite ───────────────────────────────────────────────────
        if (invisibleRingsNumber == 0 || seeAll) {
            display.drawBitmap(
                (int)(screenX + tileSize / 2) - tileSize / 2,
                (int)(screenY + tileSize / 2) - tileSize / 2,
                playerSprite, tileSize, tileSize, 15);
        }

        // ── "Carry [X]" prompt ────────────────────────────────────────────
        {
            float dx = playerX - damsel[0].x;
            float dy = playerY - damsel[0].y;
            if (!damsel[0].beingCarried && !damsel[0].dead &&
                damsel[0].levelOfLove >= 6 &&
                (dx * dx + dy * dy) <= 0.4f) {
                display.setTextSize(1);
                display.setTextColor(15, 0);
                int textWidth = 9 * 6;
                display.setCursor((int)(screenX + tileSize / 2) - textWidth / 2,
                                  (int)(screenY + tileSize) + 2);
                display.print("Carry [X]");
            }
        }

        // ── "Shop [X]" prompt ────────────────────────────────────────────
        if (strcmp(enemies[7].name, "shopkeeper") == 0) {
            float dx = playerX - enemies[7].x;
            float dy = playerY - enemies[7].y;
            if ((dx * dx + dy * dy) <= 0.4f) {
                display.setTextSize(1);
                display.setTextColor(15, 0);
                int textWidth = 9 * 6;
                display.setCursor((int)(screenX + tileSize / 2) - textWidth / 2,
                                  (int)(screenY + tileSize) + 2);
                display.print("Shop [X]");
                if (g_state.buttons.bPressed && !g_state.buttons.bPressedPrev) {
                    playRawSFX(12);
                    g_state.currentUIState = UI_SHOP;
                }
            }
        }

        // ── Facing-tile interaction prompts ───────────────────────────────
        int facingX = round(playerX) + playerDX;
        int facingY = round(playerY) + playerDY;

        if (!(playerDX == 0 && playerDY == 0) &&
            facingX >= 0 && facingX < mapWidth &&
            facingY >= 0 && facingY < mapHeight) {

            TileTypes facing = dungeonMap[facingY][facingX];
            bool showPrompt  = false;
            const char* promptText = "";

            switch (facing) {
                case ChestTile:   showPrompt = true; promptText = "Open Chest [X]";  break;
                case DoorClosed:  showPrompt = true; promptText = "Open Door [X]";   break;
                case DoorOpen:    showPrompt = true; promptText = "Close Door [X]";  break;
                case Exit:        showPrompt = true; promptText = "Descend [X]";     break;
                case KeyTile:     showPrompt = true; promptText = "Locked Exit [X]"; break;
                case Freedom:     showPrompt = true; promptText = "Escape [X]";      break;
                case Kiosk:       showPrompt = true; promptText = "Shop [X]";   break;
                default: break;
            }

            if (showPrompt) {
                // Prevent shooting while interacting
                reloading  = true;
                shootDelay = 0;

                display.setTextSize(1);
                display.setTextColor(15, 0);
                int textWidth = (int)strlen(promptText) * 6;
                display.setCursor((int)(screenX + tileSize / 2) - textWidth / 2,
                                  (int)(screenY + tileSize) + 12);
                display.print(promptText);
            }
        }
    }

    // ── Melee swipe arc ───────────────────────────────────────────────────
    if (meleeFrames > 0) {
        float pScreenX = (playerX - offsetX) * tileSize + tileSize / 2.0f;
        float pScreenY = (playerY - offsetY) * tileSize + tileSize / 2.0f;
        float fadeFrac = (float)meleeFrames / (float)MELEE_DURATION_FRAMES;
        drawSwipe(pScreenX, pScreenY, (float)playerDX, (float)playerDY, fadeFrac);
    }

    // ── Viewport edge nudging ─────────────────────────────────────────────
    if (playerX - offsetX < 2 && offsetX > 0)
        offsetX -= scrollSpeed;
    if (playerX - offsetX > viewportWidth - 3 && offsetX < mapWidth - viewportWidth)
        offsetX += scrollSpeed;
    if (playerY - offsetY < 2 && offsetY > 0)
        offsetY -= scrollSpeed;
    if (playerY - offsetY > viewportHeight - 3 && offsetY < mapHeight - viewportHeight)
        offsetY += scrollSpeed;
}