#include "SaveLogic.h"
#include <Audio.h>

// Calculate checksum of entire struct except checksum field
static uint32_t calculateChecksum(const SaveData& data) {
    const uint8_t* bytes = (const uint8_t*)&data;
    uint32_t sum = 0;
    for (size_t i = 0; i < sizeof(SaveData) - sizeof(uint32_t); i++) {
        sum = (sum << 3) ^ bytes[i]; // Better checksum mixing
    }
    return sum;
}

bool saveGame(const SaveData& data) {
    Serial.println("=== CHUNKED SAVE ===");
    
    AudioNoInterrupts();
    delay(50);
    
    // Remove old save file FIRST, before we open anything
    if (SD.exists(SAVE_FILE_PATH)) {
        Serial.println("Emptying old save file...");
        if (!deleteSave()) {
            Serial.println("⚠️ Could not empty old save file");
            // Continue anyway - it might not exist or be in a weird state
        } else {
            Serial.println("✅ Old save file emptied");
        }
        delay(20);
    }
    
    // Now create the new file
    File f = SD.open(SAVE_FILE_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("❌ Cannot open file for writing");
        AudioInterrupts();
        return false;
    }
    
    Serial.println("✅ Save file opened successfully");
    
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
    
    // CRITICAL: Flush and close the file
    f.flush();
    f.close();
    delay(50); // Important delay after close
    
    if (!success) {
        Serial.println("❌ Save failed during chunk writing");
        // Remove the partial file
        SD.remove(SAVE_FILE_PATH);
        AudioInterrupts();
        return false;
    }
    
    // Verify the file was written correctly
    File verify = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!verify) {
        Serial.println("❌ Cannot verify save file");
        SD.remove(SAVE_FILE_PATH);
        AudioInterrupts();
        return false;
    }
    
    size_t fileSize = verify.size();
    verify.close();
    
    Serial.print("Final file size: ");
    Serial.print(fileSize);
    Serial.print("/");
    Serial.println(totalSize);
    
    success = (fileSize == totalSize);
    
    AudioInterrupts();
    Serial.println(success ? "✅ Save successful" : "❌ Save failed");
    return success;
}

bool loadGame(SaveData& outData) {
    Serial.println("=== CHUNKED LOAD ===");
    
    AudioNoInterrupts();
    delay(50);
    
    if (!SD.exists(SAVE_FILE_PATH)) {
        Serial.println("❌ Save file doesn't exist");
        AudioInterrupts();
        return false;
    }
    
    File f = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!f) {
        Serial.println("❌ Cannot open file for reading");
        AudioInterrupts();
        return false;
    }
    
    size_t fileSize = f.size();
    size_t expectedSize = sizeof(SaveData);
    
    if (fileSize != expectedSize) {
        Serial.print("❌ File size mismatch: ");
        Serial.print(fileSize);
        Serial.print(" != ");
        Serial.println(expectedSize);
        f.close();
        AudioInterrupts();
        return false;
    }
    
    // Load in chunks
    uint8_t* dataPtr = (uint8_t*)&outData;
    size_t chunks = (fileSize + SAVE_CHUNK_SIZE - 1) / SAVE_CHUNK_SIZE;
    
    Serial.print("Loading ");
    Serial.print(fileSize);
    Serial.print(" bytes in ");
    Serial.print(chunks);
    Serial.println(" chunks");
    
    bool success = true;
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
    
    // Verify checksum
    if (success) {
        uint32_t savedChecksum = outData.checksum;
        uint32_t calculatedChecksum = calculateChecksum(outData);
        
        Serial.print("Checksum: ");
        Serial.print(savedChecksum);
        Serial.print(" == ");
        Serial.println(calculatedChecksum);
        
        success = (savedChecksum == calculatedChecksum);
        
        if (!success) {
            Serial.println("❌ Checksum mismatch - file may be corrupted");
        }
    }
    
    AudioInterrupts();
    Serial.println(success ? "✅ Load successful" : "❌ Load failed");
    return success;
}

bool saveExists() {
    return SD.exists(SAVE_FILE_PATH);
}

bool deleteSave() {
    Serial.println("=== EMPTYING SAVE FILE ===");
    
    AudioNoInterrupts();
    delay(50);
    
    bool success = false;
    
    // Method 1: Open with truncation (preferred)
    File f = SD.open(SAVE_FILE_PATH, FILE_WRITE);
    if (f) {
        // Just opening in write mode should truncate to 0 bytes, but let's be explicit
        f.truncate(0);
        f.flush();
        f.close();
        delay(20);
        
        // Verify the file is now empty or doesn't exist
        if (SD.exists(SAVE_FILE_PATH)) {
            File verify = SD.open(SAVE_FILE_PATH, FILE_READ);
            size_t size = verify.size();
            verify.close();
            
            if (size == 0) {
                Serial.println("✅ Save file emptied successfully (0 bytes)");
                success = true;
            } else {
                Serial.print("⚠️ File not empty after truncation: ");
                Serial.print(size);
                Serial.println(" bytes");
            }
        } else {
            Serial.println("✅ Save file no longer exists after truncation");
            success = true;
        }
    } else {
        Serial.println("❌ Cannot open save file for emptying");
    }
    
    AudioInterrupts();
    return success;
}