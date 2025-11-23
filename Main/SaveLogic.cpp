#include "SaveLogic.h"

// ------------------------------------------------------------
// Check if save file exists
// ------------------------------------------------------------
bool saveExists()
{
    return SD.exists(SAVE_FILE_PATH);
}

// ------------------------------------------------------------
// Delete save file
// ------------------------------------------------------------
bool deleteSave()
{
    if (SD.exists(SAVE_FILE_PATH)) {
        return SD.remove(SAVE_FILE_PATH);
    }
    return false;
}

// ------------------------------------------------------------
// SAVE GAME (ultra simple)
// ------------------------------------------------------------
bool saveGame(const SaveData& data) {
    Serial.println("=== SAVE GAME ===");
    
    // Remove existing file first
    if (SD.exists(SAVE_FILE_PATH)) {
        SD.remove(SAVE_FILE_PATH);
    }
    
    // Open file for writing
    File f = SD.open(SAVE_FILE_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("❌ Cannot open file for writing");
        return false;
    }
    
    // Write data directly (without checksum for now)
    size_t written = f.write((const uint8_t*)&data, sizeof(SaveData));
    f.close();
    
    Serial.print("Written: ");
    Serial.print(written);
    Serial.print("/");
    Serial.println(sizeof(SaveData));
    
    return (written == sizeof(SaveData));
}

// ------------------------------------------------------------
// LOAD GAME (ultra simple)
// ------------------------------------------------------------
bool loadGame(SaveData& outData) {
    Serial.println("=== LOAD GAME ===");
    
    if (!SD.exists(SAVE_FILE_PATH)) {
        Serial.println("❌ Save file doesn't exist");
        return false;
    }
    
    File f = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!f) {
        Serial.println("❌ Cannot open file for reading");
        return false;
    }
    
    size_t fileSize = f.size();
    if (fileSize != sizeof(SaveData)) {
        Serial.print("❌ File size mismatch: ");
        Serial.print(fileSize);
        Serial.print(" != ");
        Serial.println(sizeof(SaveData));
        f.close();
        return false;
    }
    
    // Read data directly
    size_t readBytes = f.read((uint8_t*)&outData, sizeof(SaveData));
    f.close();
    
    Serial.print("Read: ");
    Serial.print(readBytes);
    Serial.print("/");
    Serial.println(sizeof(SaveData));
    
    return (readBytes == sizeof(SaveData));
}

void checkMemory() {
    extern unsigned long _heap_start;
    extern unsigned long _heap_end;
    extern char *__brkval;
    
    int free_memory;
    if (__brkval == 0) {
        free_memory = ((int)&free_memory) - ((int)&_heap_end);
    } else {
        free_memory = ((int)&free_memory) - ((int)__brkval);
    }
    
    Serial.print("Free memory: ");
    Serial.println(free_memory);
}