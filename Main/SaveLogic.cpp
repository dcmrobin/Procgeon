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

bool saveGame(const SaveData& data) {
    Serial.println("=== SAVE GAME ===");
    //checkStack();
    
    // Delete old save first (clean start)
    if (SD.exists(SAVE_FILE_PATH)) {
        SD.remove(SAVE_FILE_PATH);
    }
    
    // Calculate checksum
    uint32_t checksum = calculateChecksum((const uint8_t*)&data, sizeof(SaveData) - sizeof(uint32_t));
    
    // Create encrypted copy
    SaveData encryptedData = data;
    encryptedData.checksum = checksum;
    xorBuffer((uint8_t*)&encryptedData, sizeof(SaveData));
    
    // Write using simple approach (proven to work)
    File f = SD.open(SAVE_FILE_PATH, FILE_WRITE);
    if (!f) {
        Serial.println("❌ Cannot open save file for writing");
        return false;
    }
    
    size_t written = f.write((const uint8_t*)&encryptedData, sizeof(SaveData));
    f.close();
    
    Serial.print("Written: ");
    Serial.print(written);
    Serial.print("/");
    Serial.println(sizeof(SaveData));
    
    bool success = (written == sizeof(SaveData));
    if (success) {
        Serial.println("✅ Save successful");
    } else {
        Serial.println("❌ Save failed");
    }
    
    //checkStack();
    return success;
}

bool loadGame(SaveData& outData) {
    Serial.println("=== LOAD GAME ===");
    //checkStack();
    
    if (!SD.exists(SAVE_FILE_PATH)) {
        Serial.println("❌ Save file doesn't exist");
        return false;
    }
    
    // Check file size first
    File sizeCheck = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!sizeCheck) {
        Serial.println("❌ Cannot open save file for reading");
        return false;
    }
    size_t fileSize = sizeCheck.size();
    sizeCheck.close();
    
    Serial.print("File size: ");
    Serial.print(fileSize);
    Serial.print("/");
    Serial.println(sizeof(SaveData));
    
    if (fileSize != sizeof(SaveData)) {
        Serial.println("❌ File size mismatch");
        return false;
    }
    
    // Read data
    File f = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!f) {
        Serial.println("❌ Cannot open save file for reading");
        return false;
    }
    
    size_t readBytes = f.read((uint8_t*)&outData, sizeof(SaveData));
    f.close();
    
    Serial.print("Read: ");
    Serial.print(readBytes);
    Serial.print("/");
    Serial.println(sizeof(SaveData));
    
    if (readBytes != sizeof(SaveData)) {
        Serial.println("❌ Read incomplete");
        return false;
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
    
    bool success = (savedChecksum == calculatedChecksum);
    if (success) {
        Serial.println("✅ Load successful");
    } else {
        Serial.println("❌ Checksum mismatch");
    }
    
    //checkStack();
    return success;
}