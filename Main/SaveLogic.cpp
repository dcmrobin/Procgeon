#include "SaveLogic.h"

// ------------------------------------------------------------
// INTERNAL: Calculate simple checksum for tamper detection
// ------------------------------------------------------------
static uint32_t calculateChecksum(const uint8_t* buf, size_t len)
{
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++)
        sum += buf[i];
    return sum;
}

// ------------------------------------------------------------
// INTERNAL: XOR encrypt or decrypt buffer
// ------------------------------------------------------------
static void xorBuffer(uint8_t* buf, size_t len)
{
    for (size_t i = 0; i < len; i++)
        buf[i] ^= SAVE_XOR_KEY;
}

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
    if (SD.exists(SAVE_FILE_PATH)){
        return SD.remove(SAVE_FILE_PATH);
    }
    return false;
}

// ------------------------------------------------------------
// Copy file helper
// ------------------------------------------------------------
bool copyFile(const char* src, const char* dest) {
    File srcFile = SD.open(src, FILE_READ);
    if (!srcFile) return false;
    
    File destFile = SD.open(dest, FILE_WRITE);
    if (!destFile) {
        srcFile.close();
        return false;
    }
    
    uint8_t buffer[64];
    while (srcFile.available()) {
        size_t read = srcFile.read(buffer, sizeof(buffer));
        destFile.write(buffer, read);
    }
    
    destFile.flush();
    destFile.close();
    srcFile.close();
    
    return true;
}

// ------------------------------------------------------------
// SAVE GAME (with temp file for atomicity)
// ------------------------------------------------------------
bool saveGame(const SaveData& data) {
    Serial.println("=== SAVE GAME ===");
    
    const char* tempPath = "/save.tmp";
    
    // Delete temp file if it exists
    if (SD.exists(tempPath)) {
        SD.remove(tempPath);
    }
    
    File f = SD.open(tempPath, FILE_WRITE);
    if (!f) {
        Serial.println("❌ Cannot open temp file for writing");
        return false;
    }
    
    // Calculate checksum
    uint32_t checksum = calculateChecksum((const uint8_t*)&data, sizeof(SaveData) - sizeof(uint32_t));
    
    // Write in one operation
    SaveData tempData = data;
    tempData.checksum = checksum;
    xorBuffer((uint8_t*)&tempData, sizeof(SaveData));
    
    size_t written = f.write((const uint8_t*)&tempData, sizeof(SaveData));
    
    // CRITICAL: Flush and close properly
    f.flush();
    f.close();
    
    // Wait for write to complete
    delay(50);
    
    // Verify the temp file was written correctly
    File verifyFile = SD.open(tempPath, FILE_READ);
    if (!verifyFile) {
        Serial.println("❌ Cannot verify temp file");
        SD.remove(tempPath);
        return false;
    }
    
    size_t tempSize = verifyFile.size();
    verifyFile.close();
    
    Serial.print("Temp file size: ");
    Serial.print(tempSize);
    Serial.print("/");
    Serial.println(sizeof(SaveData));
    
    if (tempSize != sizeof(SaveData)) {
        Serial.println("❌ Temp file size mismatch");
        SD.remove(tempPath);
        return false;
    }
    
    // Now copy temp to final location
    if (SD.exists(SAVE_FILE_PATH)) {
        SD.remove(SAVE_FILE_PATH);
    }
    
    if (!copyFile(tempPath, SAVE_FILE_PATH)) {
        Serial.println("❌ Failed to copy temp file");
        SD.remove(tempPath);
        return false;
    }
    
    SD.remove(tempPath);
    
    // Final verification
    if (!SD.exists(SAVE_FILE_PATH)) {
        Serial.println("❌ Final save file doesn't exist");
        return false;
    }
    
    File finalCheck = SD.open(SAVE_FILE_PATH, FILE_READ);
    size_t finalSize = finalCheck.size();
    finalCheck.close();
    
    Serial.print("Final file size: ");
    Serial.print(finalSize);
    Serial.print("/");
    Serial.println(sizeof(SaveData));
    
    bool success = (written == sizeof(SaveData)) && (finalSize == sizeof(SaveData));
    Serial.println(success ? "✅ Save successful" : "❌ Save failed");
    return success;
}

// ------------------------------------------------------------
// LOAD GAME (with retry logic)
// ------------------------------------------------------------
bool loadGame(SaveData& outData) {
    Serial.println("=== LOAD GAME ===");
    
    if (!SD.exists(SAVE_FILE_PATH)) {
        Serial.println("❌ Save file doesn't exist");
        return false;
    }
    
    // Try multiple times with delays
    for (int attempt = 0; attempt < 3; attempt++) {
        if (attempt > 0) {
            Serial.print("Load retry ");
            Serial.println(attempt);
            delay(50);
        }
        
        File f = SD.open(SAVE_FILE_PATH, FILE_READ);
        if (!f) {
            Serial.println("❌ Cannot open file for reading");
            continue;
        }
        
        size_t fileSize = f.size();
        if (fileSize != sizeof(SaveData)) {
            Serial.print("❌ File size mismatch: ");
            Serial.print(fileSize);
            Serial.print(" != ");
            Serial.println(sizeof(SaveData));
            f.close();
            continue;
        }
        
        size_t readBytes = f.read((uint8_t*)&outData, sizeof(SaveData));
        f.close();
        
        Serial.print("Read: ");
        Serial.print(readBytes);
        Serial.print("/");
        Serial.println(sizeof(SaveData));
        
        if (readBytes != sizeof(SaveData)) {
            Serial.println("❌ Read incomplete");
            continue;
        }
        
        // Decrypt and verify
        xorBuffer((uint8_t*)&outData, sizeof(SaveData));
        
        uint32_t savedChecksum = outData.checksum;
        outData.checksum = 0;
        uint32_t calculatedChecksum = calculateChecksum((uint8_t*)&outData, sizeof(SaveData));
        outData.checksum = savedChecksum;
        
        Serial.print("Checksum: ");
        Serial.print(savedChecksum);
        Serial.print(" == ");
        Serial.println(calculatedChecksum);
        
        if (savedChecksum == calculatedChecksum) {
            Serial.println("✅ Load successful");
            return true;
        } else {
            Serial.println("❌ Checksum mismatch");
        }
    }
    
    Serial.println("❌ Load failed after 3 attempts");
    return false;
}

// ------------------------------------------------------------
// Simple memory cleanup
// ------------------------------------------------------------
void cleanupMemory() {
    Serial.println("🧹 Cleaning up memory...");
    // Simple delay to allow any pending operations to complete
    delay(10);
}