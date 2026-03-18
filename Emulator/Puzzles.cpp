#include "Common.h"
#include "GameState.h"
#include "Puzzles.h"
#include "Sprites.h"
#include "HelperFunctions.h"

// ─────────────────────────────────────────────────────────────────────────────
// Puzzles.cpp  — Picross and Lights-Out puzzles
// Logic identical to original; only includes have changed.
// buttons → g_state.buttons throughout.
// ─────────────────────────────────────────────────────────────────────────────

bool picrossSolution[PICROSS_SIZE][PICROSS_SIZE];
bool picrossPlayerGrid[PICROSS_SIZE][PICROSS_SIZE];
static int picrossCursorX = 0;
static int picrossCursorY = 0;

bool lightsOutGrid[LIGHTSOUT_SIZE][LIGHTSOUT_SIZE];
int  lightsOutCursorX = 0;
int  lightsOutCursorY = 0;

bool puzzleFinished = false;
bool puzzleSuccess  = false;

static bool showingPicrossInstructions   = false;
static bool showingLightsOutInstructions = false;

// ─────────────────────────────────────────────────────────────────────────────
// Picross
// ─────────────────────────────────────────────────────────────────────────────
void generatePicrossPuzzle() {
    for (int y = 0; y < PICROSS_SIZE; y++) {
        int len   = random(0, PICROSS_SIZE + 1);
        int start = (len == 0) ? 0 : random(0, PICROSS_SIZE - len + 1);
        for (int x = 0; x < PICROSS_SIZE; x++)
            picrossSolution[y][x] = (x >= start && x < start + len);
    }
}

void resetPicrossPuzzle() {
    generatePicrossPuzzle();
    for (int y = 0; y < PICROSS_SIZE; y++)
        for (int x = 0; x < PICROSS_SIZE; x++)
            picrossPlayerGrid[y][x] = false;
    picrossCursorX = 0;
    picrossCursorY = 0;
}

static void drawPicrossPuzzle() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(15, 0);

    const int cellSize = 16, gridX = 32, gridY = 16;
    for (int y = 0; y < PICROSS_SIZE; y++) {
        for (int x = 0; x < PICROSS_SIZE; x++) {
            int px = gridX + x * cellSize;
            int py = gridY + y * cellSize;
            display.drawRect(px, py, cellSize, cellSize, 15);
            if (picrossPlayerGrid[y][x])
                display.fillRect(px + 2, py + 2, cellSize - 4, cellSize - 4, 15);
            if (x == picrossCursorX && y == picrossCursorY) {
                if (picrossPlayerGrid[y][x])
                    display.fillRect(px + 4, py + 4, cellSize - 8, cellSize - 8, 0);
                else
                    display.fillRect(px + 4, py + 4, cellSize - 8, cellSize - 8, 15);
                display.drawRect(px - 2, py - 2, cellSize + 4, cellSize + 4, 1);
            }
        }
    }
    for (int y = 0; y < PICROSS_SIZE; y++) {
        int count = 0;
        for (int x = 0; x < PICROSS_SIZE; x++) if (picrossSolution[y][x]) count++;
        display.setCursor(gridX - 18, gridY + y * cellSize + 4);
        display.print(std::to_string(count).c_str());
    }
    for (int x = 0; x < PICROSS_SIZE; x++) {
        int count = 0;
        for (int y = 0; y < PICROSS_SIZE; y++) if (picrossSolution[y][x]) count++;
        display.setCursor(gridX + x * cellSize + 4, gridY - 10);
        display.print(std::to_string(count).c_str());
    }
    display.setCursor(0, 120); display.print("Picross puzzle");
    display.setCursor(0, 100); display.print("Press [ENTER] for help.");
    display.display();
}

static void handlePicrossInput() {
    const ButtonStates& b = g_state.buttons;
    if      (b.upPressed    && !b.upPressedPrev    && picrossCursorY > 0)              picrossCursorY--;
    else if (b.downPressed  && !b.downPressedPrev  && picrossCursorY < PICROSS_SIZE-1) picrossCursorY++;
    else if (b.leftPressed  && !b.leftPressedPrev  && picrossCursorX > 0)              picrossCursorX--;
    else if (b.rightPressed && !b.rightPressedPrev && picrossCursorX < PICROSS_SIZE-1) picrossCursorX++;
    else if (b.bPressed     && !b.bPressedPrev)
        picrossPlayerGrid[picrossCursorY][picrossCursorX] =
            !picrossPlayerGrid[picrossCursorY][picrossCursorX];
}

static bool isPicrossSolved() {
    for (int y = 0; y < PICROSS_SIZE; y++) {
        int clue = 0, filled = 0;
        for (int x = 0; x < PICROSS_SIZE; x++) {
            if (picrossSolution[y][x])   clue++;
            if (picrossPlayerGrid[y][x]) filled++;
        }
        if (clue != filled) return false;
    }
    for (int x = 0; x < PICROSS_SIZE; x++) {
        int clue = 0, filled = 0;
        for (int y = 0; y < PICROSS_SIZE; y++) {
            if (picrossSolution[y][x])   clue++;
            if (picrossPlayerGrid[y][x]) filled++;
        }
        if (clue != filled) return false;
    }
    return true;
}

void startPicrossPuzzle() {
    resetPicrossPuzzle();
    puzzleFinished = false;
    puzzleSuccess  = false;
    currentUIState = UI_PICROSS;
}

void updatePicrossPuzzle() {
    const ButtonStates& b = g_state.buttons;

    if (b.startPressed && !b.startPressedPrev)
        showingPicrossInstructions = !showingPicrossInstructions;

    if (showingPicrossInstructions) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(15, 0);
        display.setCursor(4, 8);
        display.print("Fill squares to match the clues for each row and column. "
                      "Use the arrow keys to move, press [X] to toggle a cell. "
                      "Press [Z] to cancel the puzzle. "
                      "Press [ENTER] again to close this help.");
        display.display();
        return;
    }

    drawPicrossPuzzle();
    handlePicrossInput();

    if (b.aPressed && !b.aPressedPrev) {
        puzzleFinished = true;
        puzzleSuccess  = false;
        currentUIState = UI_NORMAL;
        return;
    }
    if (isPicrossSolved()) {
        puzzleFinished = true;
        puzzleSuccess  = true;
        currentUIState = UI_NORMAL;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Lights Out
// ─────────────────────────────────────────────────────────────────────────────
void generateLightsOutPuzzle() {
    for (int y = 0; y < LIGHTSOUT_SIZE; y++)
        for (int x = 0; x < LIGHTSOUT_SIZE; x++)
            lightsOutGrid[y][x] = false;

    const int dx[5] = { 0, 1,-1, 0, 0 };
    const int dy[5] = { 0, 0, 0, 1,-1 };

    int numToggles = random(2, 5);
    for (int i = 0; i < numToggles; i++) {
        int rx = random(0, LIGHTSOUT_SIZE);
        int ry = random(0, LIGHTSOUT_SIZE);
        for (int j = 0; j < 5; j++) {
            int nx = rx + dx[j], ny = ry + dy[j];
            if (nx >= 0 && nx < LIGHTSOUT_SIZE && ny >= 0 && ny < LIGHTSOUT_SIZE)
                lightsOutGrid[ny][nx] = !lightsOutGrid[ny][nx];
        }
    }
}

void resetLightsOutPuzzle() {
    generateLightsOutPuzzle();
    lightsOutCursorX = 0;
    lightsOutCursorY = 0;
}

static void drawLightsOutPuzzle() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(15, 0);

    const int cellSize = 16, gridX = 32, gridY = 16;
    for (int y = 0; y < LIGHTSOUT_SIZE; y++) {
        for (int x = 0; x < LIGHTSOUT_SIZE; x++) {
            int px = gridX + x * cellSize;
            int py = gridY + y * cellSize;
            display.drawRect(px, py, cellSize, cellSize, 15);
            if (lightsOutGrid[y][x])
                display.fillRect(px + 2, py + 2, cellSize - 4, cellSize - 4, 15);
            if (x == lightsOutCursorX && y == lightsOutCursorY) {
                if (lightsOutGrid[y][x])
                    display.fillRect(px + 4, py + 4, cellSize - 8, cellSize - 8, 0);
                else
                    display.fillRect(px + 4, py + 4, cellSize - 8, cellSize - 8, 15);
                display.drawRect(px - 2, py - 2, cellSize + 4, cellSize + 4, 1);
            }
        }
    }
    display.setCursor(0, 120); display.print("Lights Out puzzle");
    display.setCursor(0,   2); display.print("Press [ENTER] for help.");
    display.display();
}

static void handleLightsOutInput() {
    const ButtonStates& b = g_state.buttons;
    const int dx[5] = { 0, 1,-1, 0, 0 };
    const int dy[5] = { 0, 0, 0, 1,-1 };

    if      (b.upPressed    && !b.upPressedPrev    && lightsOutCursorY > 0)               lightsOutCursorY--;
    else if (b.downPressed  && !b.downPressedPrev  && lightsOutCursorY < LIGHTSOUT_SIZE-1) lightsOutCursorY++;
    else if (b.leftPressed  && !b.leftPressedPrev  && lightsOutCursorX > 0)               lightsOutCursorX--;
    else if (b.rightPressed && !b.rightPressedPrev && lightsOutCursorX < LIGHTSOUT_SIZE-1) lightsOutCursorX++;
    else if (b.bPressed     && !b.bPressedPrev) {
        for (int i = 0; i < 5; i++) {
            int nx = lightsOutCursorX + dx[i];
            int ny = lightsOutCursorY + dy[i];
            if (nx >= 0 && nx < LIGHTSOUT_SIZE && ny >= 0 && ny < LIGHTSOUT_SIZE)
                lightsOutGrid[ny][nx] = !lightsOutGrid[ny][nx];
        }
    }
}

static bool isLightsOutSolved() {
    for (int y = 0; y < LIGHTSOUT_SIZE; y++)
        for (int x = 0; x < LIGHTSOUT_SIZE; x++)
            if (lightsOutGrid[y][x]) return false;
    return true;
}

void startLightsOutPuzzle() {
    resetLightsOutPuzzle();
    puzzleFinished = false;
    puzzleSuccess  = false;
    currentUIState = UI_LIGHTSOUT;
}

void updateLightsOutPuzzle() {
    const ButtonStates& b = g_state.buttons;

    if (b.startPressed && !b.startPressedPrev)
        showingLightsOutInstructions = !showingLightsOutInstructions;

    if (showingLightsOutInstructions) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(15, 0);
        display.setCursor(4, 8);
        display.print("Turn all squares black. Use the arrow keys to move the cursor "
                      "and press [X] to toggle a tile (this also toggles its neighbors). "
                      "Press [Z] to cancel the puzzle. "
                      "Press [ENTER] again to close this help.");
        display.display();
        return;
    }

    drawLightsOutPuzzle();
    handleLightsOutInput();

    if (b.aPressed && !b.aPressedPrev) {
        puzzleFinished = true;
        puzzleSuccess  = false;
        currentUIState = UI_NORMAL;
        return;
    }
    if (isLightsOutSolved()) {
        puzzleFinished = true;
        puzzleSuccess  = true;
        currentUIState = UI_NORMAL;
    }
}