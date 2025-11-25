#include "SaveLogic.h"
#include "GameAudio.h"
#include <Audio.h>

// Improved checksum calculation
static uint32_t calculateChecksum(const SaveData& data) {
    const uint8_t* bytes = (const uint8_t*)&data;
    uint32_t sum = 0;
    size_t dataSize = sizeof(SaveData) - sizeof(uint32_t);
    
    for (size_t i = 0; i < dataSize; i++) {
        sum = (sum * 31) + bytes[i];  // Better distribution
    }
    return sum;
}

// New function to clean up temp files
void cleanupTempFiles() {
    for (int i = 0; i < MAX_TEMP_FILES; i++) {
        char tempPath[20];
        sprintf(tempPath, "/save_temp%d.dat", i);
        if (SD.exists(tempPath)) {
            SD.remove(tempPath);
            delay(10);
        }
    }
}

bool saveGame(const SaveData& data) {
    Serial.println("=== CHUNKED SAVE ===");
    
    // STOP AUDIO COMPLETELY before any SD operations
    stopAllAudio();
    
    // Clean up old temp files first
    cleanupTempFiles();
    
    // Use a rotating temp counter (0-9)
    static uint8_t tempCounter = 0;
    char tempPath[20];
    sprintf(tempPath, "/save_temp%d.dat", tempCounter);
    tempCounter = (tempCounter + 1) % MAX_TEMP_FILES;  // Wrap around
    
    Serial.print("Using temp file: ");
    Serial.println(tempPath);
    
    // Remove temp file if it exists
    if (SD.exists(tempPath)) {
        SD.remove(tempPath);
        delay(50);  // Increased delay
    }
    
    File f = SD.open(tempPath, FILE_WRITE);
    if (!f) {
        Serial.println("❌ Cannot open temp file for writing");
        resumeAudio();
        return false;
    }
    
    Serial.println("✅ Temp file opened successfully");
    
    // Create temporary data with checksum
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
        delay(15);  // Increased delay
        
        Serial.print("✓ Chunk ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(chunkSize);
        Serial.println(" bytes");
    }
    
    f.close();
    delay(100);  // Increased delay for file system
    
    if (!success) {
        Serial.println("❌ Save failed during chunk writing");
        SD.remove(tempPath);
        resumeAudio();
        return false;
    }
    
    // Verify temp file
    File verifyTemp = SD.open(tempPath, FILE_READ);
    if (!verifyTemp) {
        Serial.println("❌ Cannot verify temp file");
        SD.remove(tempPath);
        resumeAudio();
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
        resumeAudio();
        return false;
    }
    
    // Remove old save file and rename
    Serial.println("=== DELETING SAVE FILES ===");
    stopAllAudio();  // Stop audio again for deletion
    
    bool mainDeleted = true;
    if (SD.exists(SAVE_FILE_PATH)) {
        mainDeleted = SD.remove(SAVE_FILE_PATH);
        if (mainDeleted) {
            Serial.println("✅ Main save file deleted");
        } else {
            Serial.println("❌ Could not delete main save file");
        }
        delay(50);
    } else {
        Serial.println("ℹ️ Main save file doesn't exist");
    }
    
    // Rename temp to final
    if (SD.rename(tempPath, SAVE_FILE_PATH)) {
        Serial.println("✅ Rename successful");
    } else {
        Serial.println("❌ Rename failed - trying copy fallback");
        
        // Manual copy fallback
        File src = SD.open(tempPath, FILE_READ);
        File dst = SD.open(SAVE_FILE_PATH, FILE_WRITE);
        
        if (src && dst) {
            uint8_t buffer[256];
            size_t copied = 0;
            
            while (src.available()) {
                size_t read = src.read(buffer, sizeof(buffer));
                dst.write(buffer, read);
                copied += read;
                delay(5);
            }
            
            dst.flush();
            dst.close();
            src.close();
            delay(50);
            
            Serial.print("Copied: ");
            Serial.print(copied);
            Serial.print("/");
            Serial.println(totalSize);
            
            if (copied == totalSize) {
                Serial.println("✅ Copy fallback successful");
                SD.remove(tempPath);
            } else {
                Serial.println("❌ Copy fallback failed - incomplete copy");
                success = false;
            }
        } else {
            Serial.println("❌ Cannot open files for copy fallback");
            success = false;
        }
    }
    
    // Clean up any remaining temp files
    cleanupTempFiles();
    
    // RESUME AUDIO only after ALL SD operations are complete
    resumeAudio();
    
    Serial.println(success ? "✅ Save successful" : "❌ Save failed");
    return success;
}

bool loadGame(SaveData& outData) {
    Serial.println("=== CHUNKED LOAD ===");
    
    // STOP AUDIO COMPLETELY before any SD operations
    stopAllAudio();
    
    // Try main save file first, then limited temp files
    const char* paths[] = {SAVE_FILE_PATH, "/save_temp0.dat", "/save_temp1.dat", "/save_temp2.dat"};
    bool success = false;
    
    for (int pathIndex = 0; pathIndex < 4; pathIndex++) {
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
        
        // Initialize outData to zeros first
        memset(&outData, 0, sizeof(SaveData));
        
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
            // Clean up temp files after successful load
            cleanupTempFiles();
            resumeAudio();
            return true;
        } else {
            Serial.println("❌ Checksum mismatch - file may be corrupted");
            success = false;
            
            // Debug: print first few bytes to see what's wrong
            Serial.print("First 4 bytes: ");
            for (int i = 0; i < 4; i++) {
                Serial.print(((uint8_t*)&outData)[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }
    }
    
    // RESUME AUDIO even if load failed
    resumeAudio();
    Serial.println("❌ Load failed from all paths");
    return false;
}

bool deleteSave() {
    Serial.println("=== DELETING SAVE FILES ===");
    
    // STOP AUDIO COMPLETELY before any SD operations
    stopAllAudio();
    
    bool mainDeleted = true;
    
    // Delete main file
    if (SD.exists(SAVE_FILE_PATH)) {
        mainDeleted = SD.remove(SAVE_FILE_PATH);
        if (mainDeleted) {
            Serial.println("✅ Main save file deleted");
        } else {
            Serial.println("❌ Could not delete main save file");
        }
        delay(50);
    } else {
        Serial.println("ℹ️ Main save file doesn't exist");
    }
    
    // Clean up all temp files
    cleanupTempFiles();
    
    // RESUME AUDIO after all deletion attempts
    resumeAudio();
    
    return mainDeleted;
}

bool saveExists() {
    return SD.exists(SAVE_FILE_PATH);
}

void stopAllAudio() {
    Serial.println("Stopping all audio playback...");
    
    // Stop all audio objects that play from SD card
    if (playWav1.isPlaying()) {
        playWav1.stop();
        Serial.println("Stopped playWav1");
    }
    if (playWav2.isPlaying()) {
        playWav2.stop();
        Serial.println("Stopped playWav2");
    }
    
    delay(150); // Let audio fully stop
    
    // Then disable audio interrupts
    AudioNoInterrupts();
    delay(50);
}

void resumeAudio() {
    AudioInterrupts();
    delay(50);
    Serial.println("Audio resumed");
}