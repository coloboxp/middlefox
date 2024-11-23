#pragma once

#include <Arduino.h>
#include <vector>

class BuzzerManager
{
private:
    static BuzzerManager *instance;
    static const char *TAG;

    // Note definitions - declare first
    static const char NOTE_NAMES[];
    static const int NOTE_TONES[];
    static const int NOTE_COUNT = 16;

    // Song data - declare before use
    static const char FOX_SONG[];
    static const int FOX_BEATS[];
    static const int FOX_LENGTH;
    
    static const char CUCARACHA_SONG[];
    static const int CUCARACHA_BEATS[];
    static const int CUCARACHA_LENGTH;

    // Tempo and speed constants
    static const int DEFAULT_FOX_TEMPO = 40;
    static const int DEFAULT_CUCARACHA_TEMPO = 50;
    static const int DEFAULT_ALARM_SPEED = 20;
    static const int DEFAULT_AMBULANCE_SPEED = 300;
    static const int DEFAULT_PITCH_SPEED = 30;
    static const int DEFAULT_CRITICAL_ALARM_SPEED = 15;
    static const int CRITICAL_HIGH_FREQ = 2500;
    static const int CRITICAL_LOW_FREQ = 1800;

    // Instance variables
    int buzzerPin;
    bool isMuted;
    bool isInitialized;

    // Private constructor
    BuzzerManager() : buzzerPin(-1), isMuted(false), isInitialized(false) {}

    // Private helper methods
    void playTone(int tone, int duration);
    void playNote(char note, int duration);

public:
    // Keep all existing public methods unchanged
    static BuzzerManager& getInstance();
    bool begin(int pin);
    bool isReady() const;
    void setMute(bool mute);
    void playBootTone();
    void playLowImportance();
    void playMediumImportance();
    void playHighImportance();
    void playWhatTheFoxSaid(int tempo = DEFAULT_FOX_TEMPO);
    void playCucaracha(int tempo = DEFAULT_CUCARACHA_TEMPO);

    // Structs definitions
    struct ToneSequence {
        int frequency;
        int duration;
    };

    struct SongConfig {
        int tempo;
        bool repeat;
        int repeatCount;
        int delayBetweenNotes;
    };

    // Keep all existing methods
    static SongConfig getDefaultFoxConfig();
    static SongConfig getDefaultCucarachaConfig();
    void playWhatTheFoxSaid(const SongConfig& config);
    void playCucaracha(const SongConfig& config);
    void playSequence(const std::vector<ToneSequence>& sequence);
    void playAlarmSiren(int speed = DEFAULT_ALARM_SPEED);
    void playAmbulanceSiren(int speed = DEFAULT_AMBULANCE_SPEED);
    void playIncreasingPitch(int speed = DEFAULT_PITCH_SPEED);
    void playDecreasingPitch(int speed = DEFAULT_PITCH_SPEED);
    void playCriticalAlarm(int speed = DEFAULT_CRITICAL_ALARM_SPEED);

    // Prevent copying
    BuzzerManager(BuzzerManager const&) = delete;
    void operator=(BuzzerManager const&) = delete;
};

// Move all static member definitions to buzzer_manager.cpp

