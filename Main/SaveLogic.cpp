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
    if (SD.exists(SAVE_FILE_PATH))
        return SD.remove(SAVE_FILE_PATH);
    return false;
}

bool saveGame(const SaveData& data) {
    // Remove this line that creates a stack copy:
    // SaveData copy = data;
    
    // Work directly with the data reference
    uint32_t originalChecksum = data.checksum;
    
    // Temporarily remove checksum for calculation
    SaveData* nonConstData = const_cast<SaveData*>(&data);
    nonConstData->checksum = 0;
    uint32_t newChecksum = calculateChecksum((uint8_t*)&data, sizeof(SaveData));
    nonConstData->checksum = originalChecksum;
    
    // Create a modified version without making a full stack copy
    uint8_t buffer[sizeof(SaveData)];
    memcpy(buffer, &data, sizeof(SaveData));
    
    // Overwrite checksum in buffer
    uint32_t* checksumInBuffer = (uint32_t*)(buffer + (sizeof(SaveData) - sizeof(uint32_t)));
    *checksumInBuffer = newChecksum;
    
    // Encrypt the buffer
    xorBuffer(buffer, sizeof(SaveData));
    
    // Write to SD
    File f = SD.open(SAVE_FILE_PATH, FILE_WRITE);
    if (!f) return false;
    
    size_t written = f.write(buffer, sizeof(SaveData));
    f.close();
    
    return (written == sizeof(SaveData));
}

bool loadGame(SaveData& outData) {
    if (!SD.exists(SAVE_FILE_PATH)) return false;

    File f = SD.open(SAVE_FILE_PATH, FILE_READ);
    if (!f) return false;

    // Read directly into the output struct
    size_t readBytes = f.read((uint8_t*)&outData, sizeof(SaveData));
    f.close();

    if (readBytes != sizeof(SaveData)) return false;

    // Decrypt
    xorBuffer((uint8_t*)&outData, sizeof(SaveData));

    // Verify checksum
    uint32_t savedChecksum = outData.checksum;
    outData.checksum = 0; // Clear for calculation
    uint32_t calculatedChecksum = calculateChecksum((uint8_t*)&outData, sizeof(SaveData));
    outData.checksum = savedChecksum; // Restore

    return (savedChecksum == calculatedChecksum);
}