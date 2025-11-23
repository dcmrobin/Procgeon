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

    if (SD.exists(SAVE_FILE_PATH)) {
        Serial.println("Removing existing save file...");
        if (!SD.remove(SAVE_FILE_PATH)) {
            Serial.println("⚠️ Could not remove existing save file");
            // Don't return false here - try to continue anyway
        } else {
            Serial.println("✅ Existing save file removed");
        }
        delay(10); // Give SD card time to process deletion
    }
    
    File f = SD.open(SAVE_FILE_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("❌ Cannot open file for writing");
        AudioInterrupts();
        return false;
    }
    
    // Calculate and add checksum
    SaveData tempData = data;
    tempData.checksum = calculateChecksum(data);
    
    // Save in chunks to avoid large continuous writes
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
        
        f.flush(); // Flush after each chunk
        delay(10); // Small delay between chunks
        
        Serial.print("✓ Chunk ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(chunkSize);
        Serial.println(" bytes");
    }
    
    f.close();
    
    // Verify file size
    if (success) {
        File verify = SD.open(SAVE_FILE_PATH, FILE_READ);
        size_t fileSize = verify.size();
        verify.close();
        
        Serial.print("Final file size: ");
        Serial.print(fileSize);
        Serial.print("/");
        Serial.println(totalSize);
        
        success = (fileSize == totalSize);
    }
    
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
    AudioNoInterrupts();
    delay(50);
    
    bool result = false;
    if (SD.exists(SAVE_FILE_PATH)) {
        result = SD.remove(SAVE_FILE_PATH);
        if (result) {
            Serial.println("✅ Save file deleted");
        } else {
            Serial.println("❌ Could not delete save file");
        }
    } else {
        Serial.println("ℹ️ No save file to delete");
        result = true; // No file to delete is considered success
    }
    
    AudioInterrupts();
    return result;
}