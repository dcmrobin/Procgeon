/*
 * Teensy 4.1 SD Card Basic Test
 * Upload this alone to test if your SD card works
 */

#include <SD.h>

#define SDCARD_CS_PIN BUILTIN_SDCARD

void setup() {
  Serial.begin(9600);
  while (!Serial); // Wait for Serial monitor
  delay(1000);
  
  Serial.println();
  Serial.println("==================================");
  Serial.println("   TEENSY 4.1 SD CARD TEST");
  Serial.println("==================================");
  Serial.println();
  
  testSDCard();
}

void loop() {
  // Nothing here - run once
}

void testSDCard() {
  Serial.println("1. Initializing SD card...");
  
  if (!SD.begin(SDCARD_CS_PIN)) {
    Serial.println("❌ FAILED: SD.begin()");
    Serial.println();
    Serial.println("TROUBLESHOOTING:");
    Serial.println("- Is SD card inserted properly?");
    Serial.println("- Is SD card formatted as FAT32?");
    Serial.println("- Try a different SD card");
    Serial.println("- Check physical connection");
    return;
  }
  
  Serial.println("✅ SD.begin() successful");
  delay(100);

  Serial.println();
  Serial.println("2. Testing file creation...");
  
  File testFile = SD.open("/test.txt", FILE_WRITE);
  if (!testFile) {
    Serial.println("❌ FAILED: Cannot create file");
    return;
  }
  Serial.println("✅ File created successfully");
  
  Serial.println();
  Serial.println("3. Testing file write...");
  const char* testData = "Teensy SD Test - Hello World! 12345";
  size_t bytesWritten = testFile.write((const uint8_t*)testData, strlen(testData));
  testFile.close();
  
  Serial.print("   Written: ");
  Serial.print(bytesWritten);
  Serial.print("/");
  Serial.print(strlen(testData));
  Serial.println(" bytes");
  
  if (bytesWritten != strlen(testData)) {
    Serial.println("❌ FAILED: Write incomplete");
    return;
  }
  Serial.println("✅ File write successful");
  delay(100);

  Serial.println();
  Serial.println("4. Testing file read...");
  
  testFile = SD.open("/test.txt", FILE_READ);
  if (!testFile) {
    Serial.println("❌ FAILED: Cannot open file for reading");
    return;
  }
  
  char readBuffer[100];
  size_t bytesRead = testFile.read((uint8_t*)readBuffer, strlen(testData));
  testFile.close();
  
  Serial.print("   Read: ");
  Serial.print(bytesRead);
  Serial.print("/");
  Serial.print(strlen(testData));
  Serial.println(" bytes");
  
  if (bytesRead != strlen(testData)) {
    Serial.println("❌ FAILED: Read incomplete");
    return;
  }
  
  readBuffer[bytesRead] = '\0'; // Null terminate
  Serial.print("   Content: \"");
  Serial.print(readBuffer);
  Serial.println("\"");
  
  if (strcmp(readBuffer, testData) != 0) {
    Serial.println("❌ FAILED: Data corruption detected");
    Serial.print("   Expected: \"");
    Serial.print(testData);
    Serial.println("\"");
    return;
  }
  Serial.println("✅ File read successful - data matches");
  delay(100);

  Serial.println();
  Serial.println("5. Testing file size...");
  
  testFile = SD.open("/test.txt", FILE_READ);
  if (testFile) {
    size_t fileSize = testFile.size();
    testFile.close();
    Serial.print("   File size: ");
    Serial.print(fileSize);
    Serial.println(" bytes");
    
    if (fileSize != strlen(testData)) {
      Serial.println("❌ FAILED: File size incorrect");
      return;
    }
    Serial.println("✅ File size correct");
  }
  delay(100);

  Serial.println();
  Serial.println("6. Testing file deletion...");
  
  if (SD.remove("/test.txt")) {
    Serial.println("✅ File deleted successfully");
  } else {
    Serial.println("❌ FAILED: Cannot delete file");
    return;
  }

  Serial.println();
  Serial.println("7. Testing directory listing...");
  
  File root = SD.open("/");
  if (!root) {
    Serial.println("❌ FAILED: Cannot open root directory");
    return;
  }
  
  Serial.println("   Root directory contents:");
  int fileCount = 0;
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break;
    }
    Serial.print("   - ");
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println(" [DIR]");
    } else {
      Serial.print(" (");
      Serial.print(entry.size());
      Serial.println(" bytes)");
    }
    entry.close();
    fileCount++;
  }
  root.close();
  Serial.print("   Total files/folders: ");
  Serial.println(fileCount);

  Serial.println();
  Serial.println("==================================");
  Serial.println("✅ ALL SD CARD TESTS PASSED!");
  Serial.println("==================================");
  Serial.println();
  Serial.println("Your SD card is working properly.");
  Serial.println("The issue is in your game code, not the SD hardware.");
}
