#include "SaveLogic.h"
#include "GameAudio.h"
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
    
    // STOP AUDIO COMPLETELY before any SD operations
    stopAllAudio();
    
    // Generate unique temp filename
    static uint32_t tempCounter = 0;
    char tempPath[20];
    sprintf(tempPath, "/save_temp%d.dat", tempCounter++);
    
    Serial.print("Using temp file: ");
    Serial.println(tempPath);
    
    // Remove temp file if it exists
    if (SD.exists(tempPath)) {
        SD.remove(tempPath);
        delay(20);
    }
    
    File f = SD.open(tempPath, FILE_WRITE);
    if (!f) {
        Serial.println("❌ Cannot open temp file for writing");
        resumeAudio();
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
    deleteSave();
    delay(50);
    
    // Rename temp to final
    if (SD.rename(tempPath, SAVE_FILE_PATH)) {
        Serial.println("✅ Rename successful");
    } else {
        Serial.println("❌ Rename failed - trying copy fallback");
        
        // Manual copy fallback (audio is still stopped)
        File src = SD.open(tempPath, FILE_READ);
        File dst = SD.open(SAVE_FILE_PATH, FILE_WRITE);
        
        if (src && dst) {
            uint8_t buffer[256];
            size_t copied = 0;
            
            while (src.available()) {
                size_t read = src.read(buffer, sizeof(buffer));
                dst.write(buffer, read);
                copied += read;
            }
            
            dst.close();
            src.close();
            
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
    
    // RESUME AUDIO only after ALL SD operations are complete
    resumeAudio();
    
    Serial.println(success ? "✅ Save successful" : "❌ Save failed");
    return success;
}

bool loadGame(SaveData& outData) {
    Serial.println("=== CHUNKED LOAD ===");
    
    // STOP AUDIO COMPLETELY before any SD operations
    stopAllAudio();
    
    // Try both main and temp paths
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
            // RESUME AUDIO only after successful load
            resumeAudio();
            return true;
        } else {
            Serial.println("❌ Checksum mismatch - file may be corrupted");
            success = false;
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
    bool tempDeleted = true;
    
    // Delete main file
    if (SD.exists(SAVE_FILE_PATH)) {
        mainDeleted = SD.remove(SAVE_FILE_PATH);
        if (mainDeleted) {
            Serial.println("✅ Main save file deleted");
        } else {
            Serial.println("❌ Could not delete main save file");
        }
        delay(20);
    } else {
        Serial.println("ℹ️ Main save file doesn't exist");
    }
    
    // Delete all possible temp files
    const char* tempFiles[] = {"/save_temp.dat", "/save_temp0.dat", "/save_temp1.dat", "/save_temp2.dat", "/save_temp3.dat"};
    
    for (int i = 0; i < 5; i++) {
        if (SD.exists(tempFiles[i])) {
            if (SD.remove(tempFiles[i])) {
                Serial.print("✅ Deleted temp file: ");
                Serial.println(tempFiles[i]);
            } else {
                Serial.print("❌ Could not delete temp file: ");
                Serial.println(tempFiles[i]);
                tempDeleted = false;
            }
            delay(10);
        }
    }
    
    // RESUME AUDIO after all deletion attempts
    resumeAudio();
    
    return mainDeleted && tempDeleted;
}

bool saveExists() {
    return SD.exists(SAVE_FILE_PATH) || SD.exists("/save_temp.dat");
}

void stopAllAudio() {
    Serial.println("Stopping all audio playback...");
    
    // Stop all audio objects that play from SD card
    // Replace these with your actual audio object names
    if (playWav1.isPlaying()) {
        playWav1.stop();
        Serial.println("Stopped playWav1");
    }
    if (playWav2.isPlaying()) {
        playWav2.stop();
        Serial.println("Stopped playWav2");
    }
    
    delay(100); // Let audio fully stop
    
    // Then disable audio interrupts
    AudioNoInterrupts();
    delay(50);
}

void resumeAudio() {
    AudioInterrupts();
    delay(50);
    Serial.println("Audio resumed");
}