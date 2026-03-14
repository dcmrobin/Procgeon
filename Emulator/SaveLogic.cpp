#include "SaveLogic.h"
#include "GameAudio.h"
#include <cstring>
#include <cstdio>

// SAVE_FILE_PATH, FILE_WRITE, FILE_READ, SAVE_CHUNK_SIZE, SAVE_VERSION,
// and SAVE_MAGIC are all defined in SaveLogic.h – do NOT redefine them here.

// ── Checksum ─────────────────────────────────────────────────────────────────
// Hashes every byte of SaveData up to (but NOT including) the checksum field.
// Because checksum is declared just before the large data arrays, this covers
// all the scalar / struct fields without touching dungeonMap / itemList.
static uint32_t calculateChecksum(const SaveData& data) {
    const uint8_t* bytes          = reinterpret_cast<const uint8_t*>(&data);
    const size_t   checksumOffset = offsetof(SaveData, checksum);
    uint32_t sum = 0;
    for (size_t i = 0; i < checksumOffset; i++) {
        sum = (sum << 3) ^ bytes[i];
    }
    return sum;
}

// Returns the size of the final (possibly partial) chunk.
// Prevents the old bug where totalSize % SAVE_CHUNK_SIZE == 0 returned 0,
// causing an extra full chunk of uninitialised data to be written.
static size_t lastChunkSize(size_t totalSize) {
    size_t rem = totalSize % SAVE_CHUNK_SIZE;
    return (rem == 0) ? SAVE_CHUNK_SIZE : rem;
}

// ── saveGame ──────────────────────────────────────────────────────────────────
bool saveGame(const SaveData& data) {
    Serial.println("=== SAVE ===");

    stopAllAudio();

    // Remove any stale save file before writing a fresh one so a failed
    // partial write cannot leave a corrupt file that passes the size check.
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

    // Work on a local copy so we can stamp magic/version/checksum without
    // mutating the caller's struct.
    SaveData temp = data;
    temp.magic    = SAVE_MAGIC;
    temp.version  = SAVE_VERSION;
    temp.checksum = calculateChecksum(temp);

    const uint8_t* ptr       = reinterpret_cast<const uint8_t*>(&temp);
    const size_t   totalSize = sizeof(SaveData);
    const size_t   chunks    = (totalSize + SAVE_CHUNK_SIZE - 1) / SAVE_CHUNK_SIZE;

    Serial.print("Saving ");   Serial.print(totalSize);
    Serial.print(" bytes in "); Serial.print(chunks); Serial.println(" chunks");

    bool success = true;
    for (size_t i = 0; i < chunks && success; i++) {
        const size_t chunkSize = (i == chunks - 1) ? lastChunkSize(totalSize)
                                                    : SAVE_CHUNK_SIZE;
        const size_t offset    = i * SAVE_CHUNK_SIZE;
        const size_t written   = f.write(ptr + offset, chunkSize);

        if (written != chunkSize) {
            Serial.print("Chunk "); Serial.print(i);
            Serial.print(" write failed: "); Serial.print(written);
            Serial.print("/"); Serial.println(chunkSize);
            success = false;
        } else {
            delay(10);
            Serial.print("Chunk "); Serial.print(i);
            Serial.print(": "); Serial.print(chunkSize); Serial.println(" bytes OK");
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

    // Verify the file on disk matches the expected size.
    SDClass::File verify = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!verify) {
        Serial.println("Cannot open save file for verification");
        resumeAudio();
        return false;
    }
    const size_t fileSize = verify.size();
    verify.close();

    Serial.print("Final file size: "); Serial.print(fileSize);
    Serial.print("/"); Serial.println(totalSize);

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

// ── loadGame ──────────────────────────────────────────────────────────────────
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

    Serial.print("File size: "); Serial.print(fileSize);
    Serial.print("/"); Serial.println(expectedSize);

    if (fileSize != expectedSize) {
        Serial.println("File size mismatch — save format may have changed, rejecting");
        f.close();
        resumeAudio();
        return false;
    }

    uint8_t*     ptr      = reinterpret_cast<uint8_t*>(&outData);
    const size_t dataSize = sizeof(SaveData);
    const size_t chunks   = (dataSize + SAVE_CHUNK_SIZE - 1) / SAVE_CHUNK_SIZE;

    Serial.print("Loading "); Serial.print(dataSize);
    Serial.print(" bytes in "); Serial.print(chunks); Serial.println(" chunks");

    bool success = true;
    for (size_t i = 0; i < chunks && success; i++) {
        const size_t chunkSize = (i == chunks - 1) ? lastChunkSize(dataSize)
                                                    : SAVE_CHUNK_SIZE;
        const size_t offset    = i * SAVE_CHUNK_SIZE;
        const size_t bytesRead = f.read(ptr + offset, chunkSize);

        if (bytesRead != chunkSize) {
            Serial.print("Chunk "); Serial.print(i);
            Serial.print(" read failed: "); Serial.print(bytesRead);
            Serial.print("/"); Serial.println(chunkSize);
            success = false;
        } else {
            Serial.print("Chunk "); Serial.print(i);
            Serial.print(": "); Serial.print(chunkSize); Serial.println(" bytes OK");
        }
    }

    f.close();

    if (!success) {
        Serial.println("Chunk reading failed");
        resumeAudio();
        return false;
    }

    // ── Validate magic ────────────────────────────────────────────────────
    if (outData.magic != SAVE_MAGIC) {
        Serial.println("Bad magic number — not a valid save file");
        resumeAudio();
        return false;
    }

    // ── Validate version ──────────────────────────────────────────────────
    if (outData.version != SAVE_VERSION) {
        Serial.print("Save version mismatch: got ");
        Serial.print(outData.version);
        Serial.print(", expected ");
        Serial.println(SAVE_VERSION);
        resumeAudio();
        return false;
    }

    // ── Validate checksum ─────────────────────────────────────────────────
    const uint32_t stored     = outData.checksum;
    const uint32_t calculated = calculateChecksum(outData);

    Serial.print("Checksum stored="); Serial.print(stored);
    Serial.print(" calculated=");     Serial.println(calculated);

    if (stored != calculated) {
        Serial.println("Checksum mismatch — file may be corrupted");
        resumeAudio();
        return false;
    }

    resumeAudio();
    Serial.println("Load successful");
    return true;
}

// ── deleteSave / saveExists ───────────────────────────────────────────────────
bool deleteSave() {
    Serial.println("=== DELETE SAVE ===");
    stopAllAudio();

    bool success = true;
    if (SD.exists(SAVE_FILE_PATH)) {
        success = (std::remove(SAVE_FILE_PATH) == 0);
        Serial.println(success ? "Save file deleted" : "Could not delete save file");
        delay(20);
    } else {
        Serial.println("Save file does not exist");
    }

    resumeAudio();
    return success;
}

bool saveExists() {
    return SD.exists(SAVE_FILE_PATH);
}

// ── Audio helpers ─────────────────────────────────────────────────────────────
// SD card I/O and simultaneous audio playback can cause instability on Teensy.
// Silence everything before touching the card, then let the game loop resume it.
void stopAllAudio() {
    Serial.println("Stopping audio for SD I/O...");
    if (playWav1.isPlaying()) { playWav1.stop(); }
    if (playWav2.isPlaying()) { playWav2.stop(); }
    delay(100);
}

void resumeAudio() {
    delay(50);
    Serial.println("Audio I/O complete");
}