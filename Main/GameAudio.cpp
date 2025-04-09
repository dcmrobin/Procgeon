#include "GameAudio.h"
#include <SD.h>

// Define the audio system objects
AudioPlaySdWav      playWav1;
AudioPlayQueue      queue1;
AudioMixer4         mixer1;
AudioOutputI2S      audioOutput;

AudioConnection     patchCord1(playWav1, 0, mixer1, 0);
AudioConnection     patchCord2(queue1, 0, mixer1, 1);
AudioConnection     patchCord3(mixer1, 0, audioOutput, 0);
AudioConnection     patchCord4(mixer1, 0, audioOutput, 1);
AudioControlSGTL5000 sgtl5000_1;

// RAM-loaded sound effect storage
uint8_t* sfxData[NUM_SFX] = { nullptr };
size_t sfxLength[NUM_SFX] = { 0 };

const char* sfxFilenames[NUM_SFX] = {
  "player_hurt.raw",
  "player_shoot.raw"
};

// Definitions for your functions
void playRawSFX(const uint8_t* data, uint32_t length) {
    int samplesRemaining = length / 2;

    while (samplesRemaining > 0) {
        int16_t* block = queue1.getBuffer();
        if (!block) break;

        int samplesToCopy = min(samplesRemaining, AUDIO_BLOCK_SAMPLES);
        memcpy(block, data + (length - samplesRemaining * 2), samplesToCopy * 2);

        if (samplesToCopy < AUDIO_BLOCK_SAMPLES) {
            memset(((uint8_t*)block) + samplesToCopy * 2, 0, (AUDIO_BLOCK_SAMPLES - samplesToCopy) * 2);
        }

        queue1.playBuffer();
        samplesRemaining -= samplesToCopy;
    }
}

void freeSFX() {
    for (int i = 0; i < NUM_SFX; i++) {
        if (sfxData[i]) {
            free(sfxData[i]);
            sfxData[i] = nullptr;
        }
    }
}

bool loadSFXtoRAM() {
    for (int i = 0; i < NUM_SFX; i++) {
        File f = SD.open(sfxFilenames[i]);
        if (!f) {
            Serial.printf("Failed to open %s\n", sfxFilenames[i]);
            return false;
        }

        size_t len = f.size();
        if (len > MAX_SFX_SIZE) len = MAX_SFX_SIZE;

        sfxData[i] = (uint8_t*)malloc(len);
        if (!sfxData[i]) {
            Serial.printf("Failed to allocate memory for %s\n", sfxFilenames[i]);
            f.close();
            return false;
        }

        f.read(sfxData[i], len);
        sfxLength[i] = len;
        f.close();
    }
    return true;
}