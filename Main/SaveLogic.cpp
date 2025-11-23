#include "SaveLogic.h"

bool saveGame(uint32_t worldSeed, uint8_t currentDungeon, uint16_t playerX, uint16_t playerY,
              uint8_t hp, uint8_t food, bool succubusFriend, int attackDamage, bool endlessMode,
              int kills, float armorValue, bool equippedRiddleStone, int swiftnessRingsNum,
              int strengthRingsNum, int weaknessRingsNum) {
    
    Serial.println("=== TEXT SAVE ===");
    
    if (SD.exists(SAVE_FILE_PATH)) {
        SD.remove(SAVE_FILE_PATH);
    }
    
    File f = SD.open(SAVE_FILE_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("❌ Cannot open file for writing");
        return false;
    }
    
    // Write each value on its own line
    f.println(worldSeed);
    f.println(currentDungeon);
    f.println(playerX);
    f.println(playerY);
    f.println(hp);
    f.println(food);
    f.println(succubusFriend ? 1 : 0);
    f.println(attackDamage);
    f.println(endlessMode ? 1 : 0);
    f.println(kills);
    f.println(armorValue, 6); // 6 decimal places for float
    f.println(equippedRiddleStone ? 1 : 0);
    f.println(swiftnessRingsNum);
    f.println(strengthRingsNum);
    f.println(weaknessRingsNum);
    
    f.close();
    Serial.println("✅ Text save successful");
    return true;
}

bool loadGame(uint32_t& worldSeed, uint8_t& currentDungeon, uint16_t& playerX, uint16_t& playerY,
              uint8_t& hp, uint8_t& food, bool& succubusFriend, int& attackDamage, bool& endlessMode,
              int& kills, float& armorValue, bool& equippedRiddleStone, int& swiftnessRingsNum,
              int& strengthRingsNum, int& weaknessRingsNum) {
    
    Serial.println("=== TEXT LOAD ===");
    
    if (!SD.exists(SAVE_FILE_PATH)) {
        Serial.println("❌ Save file doesn't exist");
        return false;
    }
    
    File f = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!f) {
        Serial.println("❌ Cannot open file for reading");
        return false;
    }
    
    // Read each value line by line
    worldSeed = f.parseInt();
    currentDungeon = f.parseInt();
    playerX = f.parseInt();
    playerY = f.parseInt();
    hp = f.parseInt();
    food = f.parseInt();
    succubusFriend = (f.parseInt() != 0);
    attackDamage = f.parseInt();
    endlessMode = (f.parseInt() != 0);
    kills = f.parseInt();
    armorValue = f.parseFloat();
    equippedRiddleStone = (f.parseInt() != 0);
    swiftnessRingsNum = f.parseInt();
    strengthRingsNum = f.parseInt();
    weaknessRingsNum = f.parseInt();
    
    f.close();
    
    Serial.println("✅ Text load successful");
    return true;
}

bool saveExists() {
    return SD.exists(SAVE_FILE_PATH);
}

bool deleteSave() {
    if (SD.exists(SAVE_FILE_PATH)) {
        return SD.remove(SAVE_FILE_PATH);
    }
    return false;
}