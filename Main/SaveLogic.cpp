#include "SaveLogic.h"
#include <Audio.h>

// Calculate checksum of entire struct except checksum field
static uint32_t calculateChecksum(const SaveData& data) {
    const uint8_t* bytes = (const uint8_t*)&data;
    uint32_t sum = 0;
    for (size_t i = 0; i < sizeof(SaveData) - sizeof(uint32_t); i++) {
        sum = (sum << 3) ^ bytes[i];
    }
    return sum;
}

bool saveGame(const SaveData& data) {
    Serial.println("=== CHUNKED SAVE ===");
    
    AudioNoInterrupts();
    delay(100);
    
    const char* tempPath = "/save_temp.dat";
    
    // Remove temp file if it exists
    if (SD.exists(tempPath)) {
        SD.remove(tempPath);
        delay(10);
    }
    
    // Create temp file first
    File f = SD.open(tempPath, FILE_WRITE);
    if (!f) {
        Serial.println("❌ Cannot open TEMP file for writing");
        AudioInterrupts();
        return false;
    }
    
    Serial.println("✅ Temp file opened successfully");
    
    // Calculate and add checksum
    SaveData tempData = data;
    tempData.checksum = calculateChecksum(data);
    
    // Save in chunks
    const uint8_t* dataPtr = (const uint8_t*)&tempData;
    size_t totalSize = sizeof(SaveData);
    size_t chunks = (totalSize + SAVE_CHUNK_SIZE - 1) / SAVE_CHUNK_SIZE;
    
    Serial.print("Saving ");
    Serial.print(totalSize);
    Serial.print(" bytes in ");
    Serial.print(chunks);
    Serial.println(" chunks");
    
    bool success = true;
    for (size_t i = 0; i < chunks; i++) {
        size_t chunkSize = (i == chunks - 1) ? totalSize % SAVE_CHUNK_SIZE : SAVE_CHUNK_SIZE;
        if (chunkSize == 0) chunkSize = SAVE_CHUNK_SIZE;
        
        size_t offset = i * SAVE_CHUNK_SIZE;
        size_t written = f.write(dataPtr + offset, chunkSize);
        
        if (written != chunkSize) {
            Serial.print("❌ Chunk ");
            Serial.print(i);
            Serial.print(" write failed: ");
            Serial.print(written);
            Serial.print("/");
            Serial.println(chunkSize);
            success = false;
            break;
        }
        
        f.flush();
        delay(10);
        
        Serial.print("✓ Chunk ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(chunkSize);
        Serial.println(" bytes");
    }
    
    f.close();
    delay(50);
    
    if (!success) {
        Serial.println("❌ Save failed during chunk writing");
        SD.remove(tempPath);
        AudioInterrupts();
        return false;
    }
    
    // Verify temp file
    File verifyTemp = SD.open(tempPath, FILE_READ);
    if (!verifyTemp) {
        Serial.println("❌ Cannot verify temp file");
        SD.remove(tempPath);
        AudioInterrupts();
        return false;
    }
    
    size_t tempSize = verifyTemp.size();
    verifyTemp.close();
    
    Serial.print("Temp file size: ");
    Serial.print(tempSize);
    Serial.print("/");
    Serial.println(totalSize);
    
    if (tempSize != totalSize) {
        Serial.println("❌ Temp file size mismatch");
        SD.remove(tempPath);
        AudioInterrupts();
        return false;
    }
    
    // Remove old save file and rename temp to final
    deleteSave(); // This will remove the old file if it exists
    
    if (SD.rename(tempPath, SAVE_FILE_PATH)) {
        Serial.println("✅ Temp file renamed to final save file");
    } else {
        Serial.println("❌ Rename failed");
        SD.remove(tempPath);
        AudioInterrupts();
        return false;
    }
    
    // Final verification
    File finalCheck = SD.open(SAVE_FILE_PATH, FILE_READ);
    size_t finalSize = finalCheck.size();
    finalCheck.close();
    
    Serial.print("Final file size: ");
    Serial.print(finalSize);
    Serial.print("/");
    Serial.println(totalSize);
    
    success = (finalSize == totalSize);
    
    AudioInterrupts();
    Serial.println(success ? "✅ Save successful" : "❌ Save failed");
    return success;
}

bool loadGame(SaveData& outData) {
    Serial.println("=== CHUNKED LOAD ===");
    
    AudioNoInterrupts();
    delay(50);
    
    // Try both main and temp paths
    const char* paths[] = {SAVE_FILE_PATH, "/save_temp.dat"};
    bool success = false;
    
    for (int pathIndex = 0; pathIndex < 2; pathIndex++) {
        const char* currentPath = paths[pathIndex];
        
        if (!SD.exists(currentPath)) {
            Serial.print("File doesn't exist: ");
            Serial.println(currentPath);
            continue;
        }
        
        File f = SD.open(currentPath, FILE_READ);
        if (!f) {
            Serial.print("❌ Cannot open file for reading: ");
            Serial.println(currentPath);
            continue;
        }
        
        size_t fileSize = f.size();
        size_t expectedSize = sizeof(SaveData);
        
        Serial.print("Trying ");
        Serial.print(currentPath);
        Serial.print(" - Size: ");
        Serial.print(fileSize);
        Serial.print("/");
        Serial.println(expectedSize);
        
        if (fileSize != expectedSize) {
            Serial.println("❌ File size mismatch");
            f.close();
            continue;
        }
        
        // Load in chunks
        uint8_t* dataPtr = (uint8_t*)&outData;
        size_t chunks = (fileSize + SAVE_CHUNK_SIZE - 1) / SAVE_CHUNK_SIZE;
        
        Serial.print("Loading ");
        Serial.print(fileSize);
        Serial.print(" bytes in ");
        Serial.print(chunks);
        Serial.println(" chunks");
        
        success = true;
        for (size_t i = 0; i < chunks; i++) {
            size_t chunkSize = (i == chunks - 1) ? fileSize % SAVE_CHUNK_SIZE : SAVE_CHUNK_SIZE;
            if (chunkSize == 0) chunkSize = SAVE_CHUNK_SIZE;
            
            size_t offset = i * SAVE_CHUNK_SIZE;
            size_t read = f.read(dataPtr + offset, chunkSize);
            
            if (read != chunkSize) {
                Serial.print("❌ Chunk ");
                Serial.print(i);
                Serial.print(" read failed: ");
                Serial.print(read);
                Serial.print("/");
                Serial.println(chunkSize);
                success = false;
                break;
            }
            
            Serial.print("✓ Chunk ");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(chunkSize);
            Serial.println(" bytes");
        }
        
        f.close();
        
        if (!success) {
            Serial.println("❌ Chunk reading failed");
            continue;
        }
        
        // Verify checksum
        uint32_t savedChecksum = outData.checksum;
        uint32_t calculatedChecksum = calculateChecksum(outData);
        
        Serial.print("Checksum: ");
        Serial.print(savedChecksum);
        Serial.print(" == ");
        Serial.println(calculatedChecksum);
        
        if (savedChecksum == calculatedChecksum) {
            Serial.println("✅ Load successful");
            AudioInterrupts();
            return true;
        } else {
            Serial.println("❌ Checksum mismatch - file may be corrupted");
            success = false;
        }
    }
    
    AudioInterrupts();
    Serial.println("❌ Load failed from all paths");
    return false;
}

bool deleteSave() {
    Serial.println("=== DELETING SAVE FILE ===");
    
    AudioNoInterrupts();
    delay(50);
    
    // SIMPLE: Just call remove() directly
    bool success = SD.remove(SAVE_FILE_PATH);
    
    if (success) {
        Serial.println("✅ Save file deleted successfully");
    } else {
        // Check if the failure is because file doesn't exist
        if (!SD.exists(SAVE_FILE_PATH)) {
            Serial.println("ℹ️ No save file to delete (file doesn't exist)");
            success = true; // Consider this success
        } else {
            Serial.println("❌ Could not delete save file (file exists but remove failed)");
        }
    }
    
    // Also clean up temp file if it exists
    if (SD.exists("/save_temp.dat")) {
        if (SD.remove("/save_temp.dat")) {
            Serial.println("✅ Temp file also deleted");
        }
    }
    
    delay(20);
    AudioInterrupts();
    return success;
}

bool saveExists() {
    return SD.exists(SAVE_FILE_PATH) || SD.exists("/save_temp.dat");
}