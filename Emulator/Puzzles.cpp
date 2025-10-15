#include "Puzzles.h"
#include "Sprites.h"
#include "Player.h"
#include "HelperFunctions.h"

bool picrossSolution[PICROSS_SIZE][PICROSS_SIZE];
bool picrossPlayerGrid[PICROSS_SIZE][PICROSS_SIZE];

// Cursor position for navigation
static int picrossCursorX = 0;
static int picrossCursorY = 0;

// --- Lights Out Puzzle State ---
bool lightsOutGrid[LIGHTSOUT_SIZE][LIGHTSOUT_SIZE];
int lightsOutCursorX = 0;
int lightsOutCursorY = 0;

void resetPicrossPuzzle() {
    generatePicrossPuzzle();
    for (int y = 0; y < PICROSS_SIZE; y++)
        for (int x = 0; x < PICROSS_SIZE; x++)
            picrossPlayerGrid[y][x] = false;
    picrossCursorX = 0;
    picrossCursorY = 0;
}

void generatePicrossPuzzle() {
    // Only one contiguous group per row, random length 0-5, random start
    for (int y = 0; y < PICROSS_SIZE; y++) {
        int len = random(0, PICROSS_SIZE+1); // 0 to 5
        int start = (len == 0) ? 0 : random(0, PICROSS_SIZE-len+1);
        for (int x = 0; x < PICROSS_SIZE; x++) {
            picrossSolution[y][x] = (x >= start && x < start+len);
        }
    }
}

// Helper: count filled cells in a row/col for clues
void getPicrossClues(int clues[PICROSS_SIZE][PICROSS_SIZE], bool isRow) {
    for (int i = 0; i < PICROSS_SIZE; i++) {
        int clueIdx = 0, count = 0;
        for (int j = 0; j < PICROSS_SIZE; j++) {
            bool filled = isRow ? picrossSolution[i][j] : picrossSolution[j][i];
            if (filled) count++;
            else if (count > 0) {
                clues[i][clueIdx++] = (count > 5 ? 5 : count); // Clamp to 5
                count = 0;
            }
        }
        if (count > 0) clues[i][clueIdx++] = (count > 5 ? 5 : count); // Clamp to 5
        while (clueIdx < PICROSS_SIZE) clues[i][clueIdx++] = 0;
    }
}

void drawPicrossPuzzle() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(15);
    // Draw grid
    int cellSize = 16;
    int gridX = 32, gridY = 16;
    for (int y = 0; y < PICROSS_SIZE; y++) {
        for (int x = 0; x < PICROSS_SIZE; x++) {
            int px = gridX + x * cellSize;
            int py = gridY + y * cellSize;
            display.drawRect(px, py, cellSize, cellSize, 15);
            if (picrossPlayerGrid[y][x]) {
                display.fillRect(px+2, py+2, cellSize-4, cellSize-4, 15);
            }
            // Make the current square indicator highly visible
            if (x == picrossCursorX && y == picrossCursorY) {
                // Fill the cell with a contrasting color (invert)
                if (picrossPlayerGrid[y][x]) {
                    display.fillRect(px+4, py+4, cellSize-8, cellSize-8, 0); // Black center if filled
                } else {
                    display.fillRect(px+4, py+4, cellSize-8, cellSize-8, 15); // White center if empty
                }
                display.drawRect(px-2, py-2, cellSize+4, cellSize+4, 1); // Extra thick border
            }
        }
    }
    display.setFont(Adafruit_GFX::profont10_font);
    display.setTextColor(15);
    display.setTextSize(1);
    // Draw simple clues: just one number per row/col
    for (int y = 0; y < PICROSS_SIZE; y++) {
        int count = 0;
        for (int x = 0; x < PICROSS_SIZE; x++)
            if (picrossSolution[y][x]) count++;
        display.setCursor(gridX - 18, gridY + y * cellSize + 4);
        display.print(std::to_string(count));
    }
    for (int x = 0; x < PICROSS_SIZE; x++) {
        int count = 0;
        for (int y = 0; y < PICROSS_SIZE; y++)
            if (picrossSolution[y][x]) count++;
        display.setCursor(gridX + x * cellSize + 4, gridY - 10);
        display.print(std::to_string(count));
    }
    display.setCursor(0, 120);
    display.print("Picross puzzle");
    display.display();
    display.setFont(Adafruit_GFX::builtin_font);
}

void handlePicrossInput() {
    //updateButtonStates(); // Ensure button states are current
    if (buttons.upPressed && !buttons.upPressedPrev) {
        if (picrossCursorY > 0) picrossCursorY--;
    } else if (buttons.downPressed && !buttons.downPressedPrev) {
        if (picrossCursorY < PICROSS_SIZE-1) picrossCursorY++;
    } else if (buttons.leftPressed && !buttons.leftPressedPrev) {
        if (picrossCursorX > 0) picrossCursorX--;
    } else if (buttons.rightPressed && !buttons.rightPressedPrev) {
        if (picrossCursorX < PICROSS_SIZE-1) picrossCursorX++;
    } else if (buttons.bPressed && !buttons.bPressedPrev) {
        // Toggle cell
        picrossPlayerGrid[picrossCursorY][picrossCursorX] = !picrossPlayerGrid[picrossCursorY][picrossCursorX];
    }
}

bool isPicrossSolved() {
    // Check rows
    for (int y = 0; y < PICROSS_SIZE; y++) {
        int clue = 0;
        for (int x = 0; x < PICROSS_SIZE; x++)
            if (picrossSolution[y][x]) clue++;
        int filled = 0;
        for (int x = 0; x < PICROSS_SIZE; x++)
            if (picrossPlayerGrid[y][x]) filled++;
        if (clue != filled) return false;
    }
    // Check columns
    for (int x = 0; x < PICROSS_SIZE; x++) {
        int clue = 0;
        for (int y = 0; y < PICROSS_SIZE; y++)
            if (picrossSolution[y][x]) clue++;
        int filled = 0;
        for (int y = 0; y < PICROSS_SIZE; y++)
            if (picrossPlayerGrid[y][x]) filled++;
        if (clue != filled) return false;
    }
    return true;
}

bool updatePicrossPuzzle() {
    if (!isPicrossSolved()) {
        drawPicrossPuzzle();
        handlePicrossInput();
        delay(20);
        return false;
    } else {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(15);
        display.setCursor(20, 50);
        display.print("Solved!");
        display.display();
        delay(800);
        currentUIState = UI_NORMAL;
        return true;
    }
}

void resetLightsOutPuzzle() {
    generateLightsOutPuzzle();
    lightsOutCursorX = 0;
    lightsOutCursorY = 0;
}

void generateLightsOutPuzzle() {
    // Start with all off, then do a few random toggles to ensure solvable and easy
    for (int y = 0; y < LIGHTSOUT_SIZE; y++)
        for (int x = 0; x < LIGHTSOUT_SIZE; x++)
            lightsOutGrid[y][x] = false;
    int numToggles = random(2, 5); // 2-4 random toggles
    for (int i = 0; i < numToggles; i++) {
        int rx = random(0, LIGHTSOUT_SIZE);
        int ry = random(0, LIGHTSOUT_SIZE);
        int dx[5] = {0, 1, -1, 0, 0};
        int dy[5] = {0, 0, 0, 1, -1};
        for (int j = 0; j < 5; j++) {
            int nx = rx + dx[j];
            int ny = ry + dy[j];
            if (nx >= 0 && nx < LIGHTSOUT_SIZE && ny >= 0 && ny < LIGHTSOUT_SIZE) {
                lightsOutGrid[ny][nx] = !lightsOutGrid[ny][nx];
            }
        }
    }
}

void drawLightsOutPuzzle() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(15);
    int cellSize = 16;
    int gridX = 32, gridY = 16;
    for (int y = 0; y < LIGHTSOUT_SIZE; y++) {
        for (int x = 0; x < LIGHTSOUT_SIZE; x++) {
            int px = gridX + x * cellSize;
            int py = gridY + y * cellSize;
            display.drawRect(px, py, cellSize, cellSize, 15);
            if (lightsOutGrid[y][x]) {
                display.fillRect(px+2, py+2, cellSize-4, cellSize-4, 15);
            }
            // Make the current square indicator highly visible
            if (x == lightsOutCursorX && y == lightsOutCursorY) {
                if (lightsOutGrid[y][x]) {
                    display.fillRect(px+4, py+4, cellSize-8, cellSize-8, 0); // Black center if lit
                } else {
                    display.fillRect(px+4, py+4, cellSize-8, cellSize-8, 15); // White center if unlit
                }
                display.drawRect(px-2, py-2, cellSize+4, cellSize+4, 1); // Extra thick border
            }
        }
    }
    display.setCursor(0, 120);
    display.print("Lights Out puzzle");
    display.display();
}

void handleLightsOutInput() {
    //updateButtonStates();
    if (buttons.upPressed && !buttons.upPressedPrev) {
        if (lightsOutCursorY > 0) lightsOutCursorY--;
    } else if (buttons.downPressed && !buttons.downPressedPrev) {
        if (lightsOutCursorY < LIGHTSOUT_SIZE-1) lightsOutCursorY++;
    } else if (buttons.leftPressed && !buttons.leftPressedPrev) {
        if (lightsOutCursorX > 0) lightsOutCursorX--;
    } else if (buttons.rightPressed && !buttons.rightPressedPrev) {
        if (lightsOutCursorX < LIGHTSOUT_SIZE-1) lightsOutCursorX++;
    } else if (buttons.bPressed && !buttons.bPressedPrev) {
        // Toggle this cell and its neighbors
        int dx[5] = {0, 1, -1, 0, 0};
        int dy[5] = {0, 0, 0, 1, -1};
        for (int i = 0; i < 5; i++) {
            int nx = lightsOutCursorX + dx[i];
            int ny = lightsOutCursorY + dy[i];
            if (nx >= 0 && nx < LIGHTSOUT_SIZE && ny >= 0 && ny < LIGHTSOUT_SIZE) {
                lightsOutGrid[ny][nx] = !lightsOutGrid[ny][nx];
            }
        }
    }
}

bool isLightsOutSolved() {
    for (int y = 0; y < LIGHTSOUT_SIZE; y++)
        for (int x = 0; x < LIGHTSOUT_SIZE; x++)
            if (lightsOutGrid[y][x]) return false;
    return true;
}

bool updateLightsOutPuzzle() {
    if (!isLightsOutSolved()) {
        drawLightsOutPuzzle();
        handleLightsOutInput();
        delay(20);
        return false;
    } else {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(15);
        display.setCursor(20, 50);
        display.print("Solved!");
        display.display();
        delay(800);
        currentUIState = UI_NORMAL;
        return true;
    }
}

bool updateRandomPuzzle() {
    static int puzzleType = -1;
    static bool initialized = false;
    
    if (!initialized) {
        puzzleType = random(0, 2);
        if (puzzleType == 0) {
            resetPicrossPuzzle();
        } else {
            resetLightsOutPuzzle();
        }
        initialized = true;
    }
    
    if (puzzleType == 0) {
        return updatePicrossPuzzle();
    } else {
        return updateLightsOutPuzzle();
    }
}

void launchRandomPuzzle(int cy, int cx, int dx) {
    // Just set the state - the actual puzzle will be handled in game loop
    currentUIState = UI_PUZZLE;
    // Store chest coordinates for later
    puzzleChestY = cy;
    puzzleChestX = cx;
    puzzleChestDx = dx;
}