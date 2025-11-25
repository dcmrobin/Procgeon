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
    Serial.println("=== SIMPLE SAVE ===");
    
    // STOP AUDIO COMPLETELY before any SD operations
    stopAllAudio();
    
    // Delete existing save file if it exists
    if (SD.exists(SAVE_FILE_PATH)) {
        if (!SD.remove(SAVE_FILE_PATH)) {
            Serial.println("❌ Could not delete existing save file");
            resumeAudio();
            return false;
        }
        delay(50);
    }
    
    // Create new save file
    File f = SD.open(SAVE_FILE_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("❌ Cannot open save file for writing");
        resumeAudio();
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
    
    f.close();
    delay(50);
    
    if (!success) {
        Serial.println("❌ Save failed during chunk writing");
        // Clean up partial file
        if (SD.exists(SAVE_FILE_PATH)) {
            SD.remove(SAVE_FILE_PATH);
        }
        resumeAudio();
        return false;
    }
    
    // Verify file was written correctly
    File verifyFile = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!verifyFile) {
        Serial.println("❌ Cannot verify save file");
        resumeAudio();
        return false;
    }
    
    size_t fileSize = verifyFile.size();
    verifyFile.close();
    
    Serial.print("Final file size: ");
    Serial.print(fileSize);
    Serial.print("/");
    Serial.println(totalSize);
    
    if (fileSize != totalSize) {
        Serial.println("❌ File size mismatch");
        if (SD.exists(SAVE_FILE_PATH)) {
            SD.remove(SAVE_FILE_PATH);
        }
        resumeAudio();
        return false;
    }
    
    // RESUME AUDIO only after ALL SD operations are complete
    resumeAudio();
    
    Serial.println("✅ Save successful");
    return true;
}

bool loadGame(SaveData& outData) {
    Serial.println("=== SIMPLE LOAD ===");
    
    // STOP AUDIO COMPLETELY before any SD operations
    stopAllAudio();
    
    // Check if save file exists
    if (!SD.exists(SAVE_FILE_PATH)) {
        Serial.println("❌ Save file doesn't exist");
        resumeAudio();
        return false;
    }
    
    File f = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!f) {
        Serial.println("❌ Cannot open save file for reading");
        resumeAudio();
        return false;
    }
    
    size_t fileSize = f.size();
    size_t expectedSize = sizeof(SaveData);
    
    Serial.print("File size: ");
    Serial.print(fileSize);
    Serial.print("/");
    Serial.println(expectedSize);
    
    if (fileSize != expectedSize) {
        Serial.println("❌ File size mismatch");
        f.close();
        resumeAudio();
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
    
    if (!success) {
        Serial.println("❌ Chunk reading failed");
        resumeAudio();
        return false;
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
        resumeAudio();
        return false;
    }
}

bool deleteSave() {
    Serial.println("=== DELETING SAVE FILE ===");
    
    // STOP AUDIO COMPLETELY before any SD operations
    stopAllAudio();
    
    bool success = true;
    
    // Delete main file only
    if (SD.exists(SAVE_FILE_PATH)) {
        success = SD.remove(SAVE_FILE_PATH);
        if (success) {
            Serial.println("✅ Save file deleted");
        } else {
            Serial.println("❌ Could not delete save file");
        }
        delay(20);
    } else {
        Serial.println("ℹ️ Save file doesn't exist");
    }
    
    // RESUME AUDIO after deletion attempt
    resumeAudio();
    
    return success;
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