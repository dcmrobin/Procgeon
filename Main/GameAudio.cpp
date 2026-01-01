#include "GameAudio.h"
#include "Player.h"
#include <SD.h>

// Define the audio system objects
AudioPlaySdRaw      sfxPlayers[MAX_SIMULTANEOUS_SFX];
AudioMixer4         mixer1;  // For sfxPlayers 0-3
AudioMixer4         mixer2;  // For sfxPlayers 4-7
AudioMixer4         musicMixer; // New mixer for music
AudioOutputI2S      audioOutput;
AudioPlaySdWav      playWav1;
AudioPlaySdWav      playWav2;  // Jukebox music player
//AudioAmplifier      amp1;

// Create audio connections
// First mixer for sfxPlayers 0-3
AudioConnection     patchCord1(sfxPlayers[0], 0, mixer1, 0);
AudioConnection     patchCord2(sfxPlayers[1], 0, mixer1, 1);
AudioConnection     patchCord3(sfxPlayers[2], 0, mixer1, 2);
AudioConnection     patchCord4(sfxPlayers[3], 0, mixer1, 3);
// Second mixer for sfxPlayers 4-7
AudioConnection     patchCord5(sfxPlayers[4], 0, mixer2, 0);
AudioConnection     patchCord6(sfxPlayers[5], 0, mixer2, 1);
AudioConnection     patchCord7(sfxPlayers[6], 0, mixer2, 2);
AudioConnection     patchCord8(sfxPlayers[7], 0, mixer2, 3);
// Mix both SFX mixers and music into final mixer
AudioConnection     patchCord9(mixer1, 0, musicMixer, 0); // SFX mixer1 to musicMixer
AudioConnection     patchCord10(mixer2, 0, musicMixer, 1); // SFX mixer2 to musicMixer
AudioConnection     patchCord11(playWav1, 0, musicMixer, 2); // WAV music to musicMixer
AudioConnection     patchCord12(playWav2, 0, musicMixer, 3); // Jukebox music to musicMixer (moved to input 3)
AudioConnection     patchCord13(musicMixer, 0, audioOutput, 0);
AudioConnection     patchCord14(musicMixer, 0, audioOutput, 1);
AudioControlSGTL5000 sgtl5000_1;

int ambientNoiseLevel = 0;
int masterVolume = 10; // Default volume (1..10). sgtl5000_1.volume will be masterVolume/10.0
float jukeboxVolume = 0.0f;

const char* sfxFilenames[NUM_SFX] = {
  "player_hurt.raw",//        0
  "player_shoot.raw",//       1
  "player_use.raw",//         2
  "player_pickup.raw",//      3
  "player_footstep.raw",//    4 IMPLEMENT
  "player_eat.raw",//         5
  "player_drink.raw",//       6
  "menu_select.raw",//        7
  "menu_scroll.raw",//        8
  "menu_pause.raw",//         9
  "menu_gameOver.raw",//      10
  "level_end.raw",//          11
  "inventory_open.raw",//     12
  "inventory_close.raw",//    13
  "enemy_teleport.raw",//     14
  "damsel_putDown.raw",//     15
  "damsel_passive.raw",//     16
  "damsel_hurt.raw",//        17
  "damsel_good.raw",//        18
  "damsel_footstep.raw",//    19 IMPLEMENT
  "damsel_carry.raw",//       20
  "damsel_annoying.raw",//    21
  "bullet_impactWall.raw",//  22
  "bullet_impactEnemy.raw",// 23
  "succubus_hey.raw"//        24
};

// Array of currently playing sound effects
SFXPlayback activeSFX[MAX_SIMULTANEOUS_SFX];

// Initialize the audio system
void initAudio() {
    // Enable the audio shield
    // Increased AudioMemory to handle more simultaneous sounds
    AudioMemory(150);
    sgtl5000_1.enable();
    // Apply master volume (1..10 mapped to 0.0..1.0)
    float vol = constrain(masterVolume / 10.0f, 0.0f, 1.0f);
    sgtl5000_1.volume(vol);
    // Set mixer levels for each channel, scaled by master volume
    // Mixer1 (sfxPlayers 0-3)
    mixer1.gain(0, 0.0);
    mixer1.gain(1, 0.0);
    mixer1.gain(2, 0.0);
    mixer1.gain(3, 0.0);
    // Mixer2 (sfxPlayers 4-7)
    mixer2.gain(0, 0.0);
    mixer2.gain(1, 0.0);
    mixer2.gain(2, 0.0);
    mixer2.gain(3, 0.0);
    // Final musicMixer - combine both SFX mixers and music
    musicMixer.gain(0, vol); // SFX mixer1 scaled by master volume
    musicMixer.gain(1, vol); // SFX mixer2 scaled by master volume
    musicMixer.gain(2, 0.2 * vol); // WAV music (main) scaled by master volume
    musicMixer.gain(3, 0.0); // jukebox channel (starts at 0, controlled by setJukeboxVolume)
    Serial.println("Audio initialized");
}

void setJukeboxVolume(float v) {
    jukeboxVolume = constrain(v, 0.0f, 0.23f);
    // Scale jukebox volume by master volume so it respects the volume control
    float masterVol = masterVolume / 10.0f;
    musicMixer.gain(3, jukeboxVolume * masterVol); // Changed to input 3 (was 2)
}

bool playRawSFX(int sfxIndex) {
    ambientNoiseLevel++;
    if (sfxIndex < 0 || sfxIndex >= NUM_SFX) return false;
    
    // Find an available playback slot
    int slot = -1;
    for (int i = 0; i < MAX_SIMULTANEOUS_SFX; i++) {
        if (!sfxPlayers[i].isPlaying()) {
            slot = i;
            break;
        }
    }
    
    // If all slots are in use, return false or optionally replace the oldest sound
    if (slot == -1) {
        // Option 1: Fail to play the new sound
        return false;
        
        // Option 2: Replace the sound that's played the most
        /*
        int maxPlayedSamples = 0;
        for (int i = 0; i < MAX_SIMULTANEOUS_SFX; i++) {
            if (activeSFX[i].samplesPlayed > maxPlayedSamples) {
                maxPlayedSamples = activeSFX[i].samplesPlayed;
                slot = i;
            }
        }
        */
    }
    
    // Set volume
    activeSFX[slot].volume = 1.0f;
    float vol = masterVolume / 10.0f;
    float gain = activeSFX[slot].volume * 0.5f * vol;
    if (slot < 4) {
        mixer1.gain(slot, gain);
    } else {
        mixer2.gain(slot - 4, gain);
    }
    
    // Play the sound
    sfxPlayers[slot].play(sfxFilenames[sfxIndex]);
    
    return true;
}

// Play a sound effect with 3D positioning
bool playRawSFX3D(int sfxIndex, float soundX, float soundY) {
    // Calculate distance from player to sound source
    float dx = soundX - playerX;
    float dy = soundY - playerY;
    float distance = sqrt(dx * dx + dy * dy);
    
    // If sound is too far away, don't play it
    if (distance > MAX_AUDIO_DISTANCE) {
        return false;
    }
    
    // Calculate volume based on distance (linear falloff)
    float volume = 1.0f;
    if (distance > 0) {
        volume = 1.0f - (distance / MAX_AUDIO_DISTANCE);
        volume = constrain(volume, MIN_AUDIO_VOLUME, 1.0f);
    }
    
    ambientNoiseLevel++;
    if (sfxIndex < 0 || sfxIndex >= NUM_SFX) return false;
    
    // Find an available playback slot
    int slot = -1;
    for (int i = 0; i < MAX_SIMULTANEOUS_SFX; i++) {
        if (!sfxPlayers[i].isPlaying()) {
            slot = i;
            break;
        }
    }
    
    // If all slots are in use, return false
    if (slot == -1) {
        return false;
    }
    
    // Set volume with adjustment for specific SFX
    activeSFX[slot].volume = sfxIndex == 23 ? volume / 2.0f : volume;
    float vol = masterVolume / 10.0f;
    float gain = activeSFX[slot].volume * 0.5f * vol;
    if (slot < 4) {
        mixer1.gain(slot, gain);
    } else {
        mixer2.gain(slot - 4, gain);
    }
    
    // Play the sound
    sfxPlayers[slot].play(sfxFilenames[sfxIndex]);
    
    return true;
}

void serviceRawSFX() {
    // Process each active sound effect
    for (int i = 0; i < MAX_SIMULTANEOUS_SFX; i++) {
        if (!sfxPlayers[i].isPlaying()) {
            if (activeSFX[i].volume > 0.0f) {
                activeSFX[i].volume = 0.0f;
                if (i < 4) {
                    mixer1.gain(i, 0.0f);
                } else {
                    mixer2.gain(i - 4, 0.0f);
                }
            }
        }
    }
}

