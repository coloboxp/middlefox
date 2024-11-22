#pragma once

#include <Arduino.h>
#include <vector>

class BuzzerManager
{
private:
    static BuzzerManager *instance;
    int buzzerPin;
    bool isMuted;
    bool isInitialized;

    // Note definitions
    static const char NOTE_NAMES[];
    static const int NOTE_TONES[];
    static const int NOTE_COUNT = 16;
    static const int DEFAULT_TEMPO = 80;
    static const int SPEED_FACTOR = 2;

    // What the fox says song
    static const char FOX_SONG[];
    static const int FOX_BEATS[];
    static const int FOX_LENGTH;

    BuzzerManager() : buzzerPin(-1), isMuted(false), isInitialized(false) {}

    void playTone(int tone, int duration)
    {
        if (!isReady() || isMuted)
            return;
        for (long i = 0; i < duration * 1000L; i += tone * 2)
        {
            digitalWrite(buzzerPin, HIGH);
            delayMicroseconds(tone);
            digitalWrite(buzzerPin, LOW);
            delayMicroseconds(tone);
        }
    }

    void playNote(char note, int duration)
    {
        for (int i = 0; i < NOTE_COUNT; i++)
        {
            if (NOTE_NAMES[i] == note)
            {
                int newDuration = duration / SPEED_FACTOR;
                playTone(NOTE_TONES[i], newDuration);
                return;
            }
        }
    }

public:
    static BuzzerManager &getInstance()
    {
        if (instance == nullptr)
        {
            instance = new BuzzerManager();
        }
        return *instance;
    }

    bool begin(int pin)
    {
        if (pin < 0)
            return false;

        buzzerPin = pin;
        pinMode(buzzerPin, OUTPUT);
        digitalWrite(buzzerPin, LOW);
        isInitialized = true;
        return true;
    }

    bool isReady() const
    {
        return isInitialized && buzzerPin >= 0;
    }

    void setMute(bool mute)
    {
        isMuted = mute;
    }

    // Predefined tones
    void playBootTone()
    {
        if (!isReady() || isMuted)
            return;
        playNote('G', 100);
        delay(50);
        playNote('c', 200);
    }

    void playLowImportance()
    {
        if (!isReady() || isMuted)
            return;
        playNote('C', 100);
    }

    void playMediumImportance()
    {
        if (!isReady() || isMuted)
            return;
        playNote('G', 100);
        delay(50);
        playNote('G', 100);
    }

    void playHighImportance()
    {
        if (!isReady() || isMuted)
            return;
        for (int i = 0; i < 3; i++)
        {
            playNote('g', 100);
            delay(50);
        }
    }

    void playWhatTheFoxSaid()
    {
        if (!isReady() || isMuted)
            return;
        for (int i = 0; i < FOX_LENGTH; i++)
        {
            if (FOX_SONG[i] == ' ')
            {
                delay(FOX_BEATS[i] * DEFAULT_TEMPO);
            }
            else
            {
                playNote(FOX_SONG[i], FOX_BEATS[i] * DEFAULT_TEMPO);
            }
            delay(DEFAULT_TEMPO);
        }
    }

    // Custom tone sequence player
    struct ToneSequence
    {
        int frequency;
        int duration;
    };

    void playSequence(const std::vector<ToneSequence> &sequence)
    {
        if (!isReady() || isMuted)
            return;
        for (const auto &tone : sequence)
        {
            playTone(tone.frequency, tone.duration);
            delay(50); // Small gap between tones
        }
    }

    void playAlarmSiren() {
        if (!isReady() || isMuted) return;
        
        for (int i = 0; i < 3; i++) {  // 3 cycles
            // Up
            for (int freq = 800; freq < 2000; freq += 50) {
                playTone(freq, 20);
            }
            // Down
            for (int freq = 2000; freq > 800; freq -= 50) {
                playTone(freq, 20);
            }
        }
    }

    void playAmbulanceSiren() {
        if (!isReady() || isMuted) return;
        
        for (int i = 0; i < 3; i++) {  // 3 cycles
            // First tone
            playTone(1500, 300);
            delay(50);
            // Second tone
            playTone(1800, 300);
            delay(50);
        }
    }

    void playIncreasingPitch() {
        if (!isReady() || isMuted) return;
        
        for (int freq = 200; freq < 2000; freq += 50) {
            playTone(freq, 30);
            // No delay needed as playTone already takes time
        }
    }

    void playDecreasingPitch() {
        if (!isReady() || isMuted) return;
        
        for (int freq = 2000; freq > 200; freq -= 50) {
            playTone(freq, 30);
            // No delay needed as playTone already takes time
        }
    }

    // Prevent copying
    BuzzerManager(BuzzerManager const &) = delete;
    void operator=(BuzzerManager const &) = delete;
};

// Initialize static members
BuzzerManager *BuzzerManager::instance = nullptr;

const char BuzzerManager::NOTE_NAMES[] = {
    'C', 'D', 'E', 'F', 'G', 'A', 'B',
    'c', 'd', 'e', 'f', 'g', 'a', 'b',
    'x', 'y'};

const int BuzzerManager::NOTE_TONES[] = {
    1915, 1700, 1519, 1432, 1275, 1136, 1014,
    956, 834, 765, 593, 468, 346, 224,
    655, 715};

const char BuzzerManager::FOX_SONG[] = "A A A G A R E D E D E D E D E D E D E D";
const int BuzzerManager::FOX_BEATS[] = {
    // "What does the fox say?" (5 notes)
    2, 2, 2, 2, 4,
    // Rest
    2,
    // First "Ring-ding-ding-ding-dingeringeding!"
    1, 1, 1, 1, 1, 1, 1, 2,
    // Second repeat
    1, 1, 1, 1, 1, 1, 1, 2,
    // Third repeat
    1, 1, 1, 1, 1, 1, 1, 2
};
const int BuzzerManager::FOX_LENGTH = 29;
