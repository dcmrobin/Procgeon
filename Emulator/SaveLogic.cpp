#include "SaveLogic.h"
#include "GameAudio.h"
#include "Translation.h"
#include <cstring>
#include <cstdio>

#define SAVE_FILE_PATH "savegame.dat"
#define FILE_WRITE "wb"
#define FILE_READ "rb"

// Add checksum field to SaveData struct (if not already in header)
struct SaveDataWithChecksum {
    SaveData data;
    uint32_t checksum;
};

// Calculate checksum of entire struct except checksum field
static uint32_t calculateChecksum(const SaveData& data) {
    const uint8_t* bytes = (const uint8_t*)&data;
    uint32_t sum = 0;
    for (size_t i = 0; i < sizeof(SaveData); i++) {
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
        // Note: Our SD emulation doesn't have remove, but we'll overwrite
        // For SDL2, we can just try to delete using filesystem
        std::remove(SAVE_FILE_PATH);
        delay(50);
    }
    
    // Create new save file using our SD emulation
    SDClass::File f = SD.open(SAVE_FILE_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("❌ Cannot open save file for writing");
        resumeAudio();
        return false;
    }
    
    Serial.println("✅ Save file opened successfully");
    
    // Calculate and add checksum
    SaveData tempData = data;
    uint32_t checksum = calculateChecksum(tempData);
    
    // Write the data first
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
        
        // f.flush(); // Our SD emulation doesn't have flush
        delay(10);
        
        Serial.print("✓ Chunk ");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(chunkSize);
        Serial.println(" bytes");
    }
    
    // Write checksum after the data
    if (success) {
        size_t checksumWritten = f.write((const uint8_t*)&checksum, sizeof(checksum));
        if (checksumWritten != sizeof(checksum)) {
            Serial.println("❌ Failed to write checksum");
            success = false;
        }
    }
    
    f.close();
    delay(50);
    
    if (!success) {
        Serial.println("❌ Save failed during chunk writing");
        // Clean up partial file
        std::remove(SAVE_FILE_PATH);
        resumeAudio();
        return false;
    }
    
    // Verify file was written correctly
    SDClass::File verifyFile = SD.open(SAVE_FILE_PATH, FILE_READ);
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
    Serial.println(totalSize + sizeof(uint32_t));
    
    if (fileSize != totalSize + sizeof(uint32_t)) {
        Serial.println("❌ File size mismatch");
        std::remove(SAVE_FILE_PATH);
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
    
    SDClass::File f = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!f) {
        Serial.println("❌ Cannot open save file for reading");
        resumeAudio();
        return false;
    }
    
    size_t fileSize = f.size();
    size_t expectedSize = sizeof(SaveData) + sizeof(uint32_t);
    
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
    
    // Load data in chunks
    uint8_t* dataPtr = (uint8_t*)&outData;
    size_t dataSize = sizeof(SaveData);
    size_t chunks = (dataSize + SAVE_CHUNK_SIZE - 1) / SAVE_CHUNK_SIZE;
    
    Serial.print("Loading ");
    Serial.print(dataSize);
    Serial.print(" bytes in ");
    Serial.print(chunks);
    Serial.println(" chunks");
    
    bool success = true;
    for (size_t i = 0; i < chunks; i++) {
        size_t chunkSize = (i == chunks - 1) ? dataSize % SAVE_CHUNK_SIZE : SAVE_CHUNK_SIZE;
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
    
    // Read checksum
    uint32_t savedChecksum = 0;
    if (success) {
        size_t checksumRead = f.read((uint8_t*)&savedChecksum, sizeof(savedChecksum));
        if (checksumRead != sizeof(savedChecksum)) {
            Serial.println("❌ Failed to read checksum");
            success = false;
        }
    }
    
    f.close();
    
    if (!success) {
        Serial.println("❌ Chunk reading failed");
        resumeAudio();
        return false;
    }
    
    // Verify checksum
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
        success = (std::remove(SAVE_FILE_PATH) == 0);
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
    
    // SDL2 doesn't need audio interrupts, so we just stop the channels
    // AudioNoInterrupts(); // Remove this
    delay(50);
}

void resumeAudio() {
    // AudioInterrupts(); // Remove this - SDL2 doesn't need it
    delay(50);
    Serial.println("Audio resumed");
}