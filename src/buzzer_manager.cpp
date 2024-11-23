#include "buzzer_manager.h"

// Initialize static members
BuzzerManager* BuzzerManager::instance = nullptr;
const char* BuzzerManager::TAG = "BuzzerManager";

// Define song data
const char BuzzerManager::NOTE_NAMES[] = {
    'C', 'D', 'E', 'F', 'G', 'A', 'B',
    'c', 'd', 'e', 'f', 'g', 'a', 'b',
    'x', 'y'
};

const int BuzzerManager::NOTE_TONES[] = {
    1915, 1700, 1519, 1432, 1275, 1136, 1014,
    956, 834, 765, 593, 468, 346, 224,
    655, 715
};

const char BuzzerManager::FOX_SONG[] = "C C C B C   G E G G G G G G G G G G G G";
const int BuzzerManager::FOX_BEATS[] = {
    1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const int BuzzerManager::FOX_LENGTH = 28;

const char BuzzerManager::CUCARACHA_SONG[] = "C C C   F F   C C C   F F   C C C F F E D C   C   C C C F F E D C   C ";
const int BuzzerManager::CUCARACHA_BEATS[] = {
    1, 1, 2, 1, 2, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1
};
const int BuzzerManager::CUCARACHA_LENGTH = 27;

// Method implementations
BuzzerManager& BuzzerManager::getInstance() {
    if (instance == nullptr) {
        instance = new BuzzerManager();
    }
    return *instance;
}

bool BuzzerManager::begin(int pin) {
    if (pin < 0) return false;

    buzzerPin = pin;
    pinMode(buzzerPin, OUTPUT);
    digitalWrite(buzzerPin, LOW);
    isInitialized = true;
    return true;
}

void BuzzerManager::playTone(int tone, int duration) {
    if (!isReady() || isMuted) return;
    
    for (long i = 0; i < duration * 1000L; i += tone * 2) {
        digitalWrite(buzzerPin, HIGH);
        delayMicroseconds(tone);
        digitalWrite(buzzerPin, LOW);
        delayMicroseconds(tone);
    }
}

void BuzzerManager::playNote(char note, int duration) {
    for (int i = 0; i < NOTE_COUNT; i++) {
        if (NOTE_NAMES[i] == note) {
            playTone(NOTE_TONES[i], duration);
            return;
        }
    }
}

bool BuzzerManager::isReady() const {
    return isInitialized && buzzerPin >= 0;
}

void BuzzerManager::setMute(bool mute) {
    isMuted = mute;
}

void BuzzerManager::playBootTone() {
    if (!isReady() || isMuted) return;
    playNote('G', 100);
    delay(50);
    playNote('c', 200);
}

void BuzzerManager::playLowImportance() {
    if (!isReady() || isMuted) return;
    playNote('C', 100);
}

void BuzzerManager::playMediumImportance() {
    if (!isReady() || isMuted) return;
    playNote('G', 100);
    delay(50);
    playNote('G', 100);
}

void BuzzerManager::playHighImportance() {
    if (!isReady() || isMuted) return;
    for (int i = 0; i < 3; i++) {
        playNote('c', 100);
        delay(50);
    }
}

BuzzerManager::SongConfig BuzzerManager::getDefaultFoxConfig() {
    return {
        DEFAULT_FOX_TEMPO,
        false,
        1,
        50
    };
}

BuzzerManager::SongConfig BuzzerManager::getDefaultCucarachaConfig() {
    return {
        DEFAULT_CUCARACHA_TEMPO,
        false,
        1,
        50
    };
}

void BuzzerManager::playWhatTheFoxSaid(int tempo) {
    SongConfig config = getDefaultFoxConfig();
    config.tempo = tempo;
    playWhatTheFoxSaid(config);
}

void BuzzerManager::playCucaracha(int tempo) {
    SongConfig config = getDefaultCucarachaConfig();
    config.tempo = tempo;
    playCucaracha(config);
}

void BuzzerManager::playWhatTheFoxSaid(const SongConfig& config) {
    if (!isReady() || isMuted) return;
    
    for (int i = 0; i < FOX_LENGTH; i++) {
        if (FOX_SONG[i] != ' ') {
            playNote(FOX_SONG[i], FOX_BEATS[i] * config.tempo);
        }
        delay(config.delayBetweenNotes);
    }
}

void BuzzerManager::playCucaracha(const SongConfig& config) {
    if (!isReady() || isMuted) return;
    
    for (int i = 0; i < CUCARACHA_LENGTH; i++) {
        if (CUCARACHA_SONG[i] != ' ') {
            playNote(CUCARACHA_SONG[i], CUCARACHA_BEATS[i] * config.tempo);
        }
        delay(config.delayBetweenNotes);
    }
}

void BuzzerManager::playSequence(const std::vector<ToneSequence>& sequence) {
    if (!isReady() || isMuted) return;
    
    for (const auto& tone : sequence) {
        playTone(tone.frequency, tone.duration);
        delay(50);
    }
}

void BuzzerManager::playAlarmSiren(int speed) {
    if (!isReady() || isMuted) return;
    
    for (int i = 1000; i < 2000; i += 10) {
        playTone(i, speed);
    }
    for (int i = 2000; i > 1000; i -= 10) {
        playTone(i, speed);
    }
}

void BuzzerManager::playAmbulanceSiren(int speed) {
    if (!isReady() || isMuted) return;
    
    playTone(1500, speed);
    delay(50);
    playTone(2000, speed);
    delay(50);
}

void BuzzerManager::playIncreasingPitch(int speed) {
    if (!isReady() || isMuted) return;
    
    for (int i = 500; i < 2000; i += 10) {
        playTone(i, speed);
    }
}

void BuzzerManager::playDecreasingPitch(int speed) {
    if (!isReady() || isMuted) return;
    
    for (int i = 2000; i > 500; i -= 10) {
        playTone(i, speed);
    }
}

void BuzzerManager::playCriticalAlarm(int speed) {
    if (!isReady() || isMuted) return;
    
    for (int i = 0; i < 3; i++) {
        playTone(CRITICAL_HIGH_FREQ, speed);
        delay(speed);
        playTone(CRITICAL_LOW_FREQ, speed);
        delay(speed);
    }
}

// Implement all methods here... 