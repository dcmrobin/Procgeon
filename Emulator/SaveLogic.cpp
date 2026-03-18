#include "Common.h"
#include "SaveLogic.h"
#include "GameAudio.h"

#include <cstring>
#include <cstdio>

// ─────────────────────────────────────────────────────────────────────────────
// SaveLogic.cpp  — chunked SD card save / load
// Logic identical to original; only includes have changed.
// ─────────────────────────────────────────────────────────────────────────────

static uint32_t calculateChecksum(const SaveData& data) {
    const uint8_t* bytes          = reinterpret_cast<const uint8_t*>(&data);
    const size_t   checksumOffset = offsetof(SaveData, checksum);
    uint32_t sum = 0;
    for (size_t i = 0; i < checksumOffset; i++)
        sum = (sum << 3) ^ bytes[i];
    return sum;
}

static size_t lastChunkSize(size_t totalSize) {
    size_t rem = totalSize % SAVE_CHUNK_SIZE;
    return (rem == 0) ? SAVE_CHUNK_SIZE : rem;
}

bool saveGame(const SaveData& data) {
    Serial.println("=== SAVE ===");
    stopAllAudio();

    if (SD.exists(SAVE_FILE_PATH)) {
        std::remove(SAVE_FILE_PATH);
        delay(50);
    }

    SDClass::File f = SD.open(SAVE_FILE_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("Cannot open save file for writing");
        resumeAudio();
        return false;
    }

    SaveData temp    = data;
    temp.magic       = SAVE_MAGIC;
    temp.version     = SAVE_VERSION;
    temp.checksum    = calculateChecksum(temp);

    const uint8_t* ptr       = reinterpret_cast<const uint8_t*>(&temp);
    const size_t   totalSize = sizeof(SaveData);
    const size_t   chunks    = (totalSize + SAVE_CHUNK_SIZE - 1) / SAVE_CHUNK_SIZE;

    Serial.print("Saving "); Serial.print(totalSize);
    Serial.print(" bytes in "); Serial.print(chunks); Serial.println(" chunks");

    bool success = true;
    for (size_t i = 0; i < chunks && success; i++) {
        const size_t chunkSize = (i == chunks-1) ? lastChunkSize(totalSize) : SAVE_CHUNK_SIZE;
        const size_t written   = f.write(ptr + i * SAVE_CHUNK_SIZE, chunkSize);
        if (written != chunkSize) {
            Serial.printf("Chunk %zu write failed: %zu/%zu\n", i, written, chunkSize);
            success = false;
        } else {
            delay(10);
            Serial.printf("Chunk %zu: %zu bytes OK\n", i, chunkSize);
        }
    }
    f.close();
    delay(50);

    if (!success) {
        Serial.println("Save failed — removing partial file");
        std::remove(SAVE_FILE_PATH);
        resumeAudio();
        return false;
    }

    SDClass::File verify = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!verify) {
        Serial.println("Cannot open save file for verification");
        resumeAudio();
        return false;
    }
    const size_t fileSize = verify.size();
    verify.close();

    if (fileSize != totalSize) {
        Serial.println("File size mismatch — removing");
        std::remove(SAVE_FILE_PATH);
        resumeAudio();
        return false;
    }

    resumeAudio();
    Serial.println("Save successful");
    return true;
}

bool loadGame(SaveData& outData) {
    Serial.println("=== LOAD ===");
    stopAllAudio();

    if (!SD.exists(SAVE_FILE_PATH)) {
        Serial.println("Save file does not exist");
        resumeAudio();
        return false;
    }

    SDClass::File f = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!f) {
        Serial.println("Cannot open save file for reading");
        resumeAudio();
        return false;
    }

    const size_t fileSize     = f.size();
    const size_t expectedSize = sizeof(SaveData);

    if (fileSize != expectedSize) {
        Serial.println("File size mismatch — rejecting");
        f.close();
        resumeAudio();
        return false;
    }

    uint8_t*     ptr    = reinterpret_cast<uint8_t*>(&outData);
    const size_t chunks = (expectedSize + SAVE_CHUNK_SIZE - 1) / SAVE_CHUNK_SIZE;

    bool success = true;
    for (size_t i = 0; i < chunks && success; i++) {
        const size_t chunkSize = (i == chunks-1) ? lastChunkSize(expectedSize) : SAVE_CHUNK_SIZE;
        const size_t bytesRead = f.read(ptr + i * SAVE_CHUNK_SIZE, chunkSize);
        if (bytesRead != chunkSize) {
            Serial.printf("Chunk %zu read failed: %zu/%zu\n", i, bytesRead, chunkSize);
            success = false;
        } else {
            Serial.printf("Chunk %zu: %zu bytes OK\n", i, chunkSize);
        }
    }
    f.close();

    if (!success) { resumeAudio(); return false; }

    if (outData.magic != SAVE_MAGIC) {
        Serial.println("Bad magic number");
        resumeAudio(); return false;
    }
    if (outData.version != SAVE_VERSION) {
        Serial.printf("Version mismatch: got %u, expected %u\n",
                      outData.version, SAVE_VERSION);
        resumeAudio(); return false;
    }

    const uint32_t stored     = outData.checksum;
    const uint32_t calculated = calculateChecksum(outData);
    if (stored != calculated) {
        Serial.println("Checksum mismatch — file may be corrupted");
        resumeAudio(); return false;
    }

    resumeAudio();
    Serial.println("Load successful");
    return true;
}

bool deleteSave() {
    Serial.println("=== DELETE SAVE ===");
    stopAllAudio();
    bool success = true;
    if (SD.exists(SAVE_FILE_PATH)) {
        success = (std::remove(SAVE_FILE_PATH) == 0);
        delay(20);
    }
    resumeAudio();
    return success;
}

bool saveExists() {
    return SD.exists(SAVE_FILE_PATH);
}

void stopAllAudio() {
    if (playWav1.isPlaying()) playWav1.stop();
    if (playWav2.isPlaying()) playWav2.stop();
    if (playWav3.isPlaying()) playWav3.stop();
    delay(100);
}

void resumeAudio() {
    delay(50);
    Serial.println("Audio I/O complete");
}