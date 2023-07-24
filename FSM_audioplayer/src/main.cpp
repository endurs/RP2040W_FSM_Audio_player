#include <SPI.h>
#include <SdFat.h>
#include "AudioTools.h"
#include "AudioLibs/AudioSourceSDFAT.h"
#include "AudioCodecs/CodecMP3Helix.h"
#include "finite_state_machine.h"

// Global Variables and Constants
uint8_t sensorPins[8] = {2, 3, 4, 5, 6, 7, 8, 9}; // Replace with actual pin numbers
const uint8_t resetPin = 10; // Replace with actual pin number
const uint8_t ledPin = 11; // Replace with actual pin number
FSM fsm;
bool sensorStates[8];
uint8_t currentState;
// Audio Tools objects
AudioSourceSDFAT source("/");
I2SStream i2s;
MP3DecoderHelix decoder;
AudioPlayer player(source, i2s, decoder);

void setup() {
    // Initialize sensor switch pins
    for (uint8_t i = 0; i < 8; i++) {
        pinMode(sensorPins[i], INPUT_PULLUP);
    }

    // Initialize reset switch pin
    pinMode(resetPin, INPUT_PULLUP);

    // Initialize LED pin
    pinMode(ledPin, OUTPUT);

    // Initialize I2S output device
    auto cfg = i2s.defaultConfig(TX_MODE);
    i2s.begin(cfg);

    // Initialize audio player
    player.begin();

    // Load and parse JSON configuration
    fsm.loadConfiguration();

    // Initialize finite state machine
    fsm.begin();
}

void loop() {
    // Update the FSM
    fsm.update(sensorPins, sensorStates, resetPin, player, ledPin);

    // Copy audio data to the output
    player.copy();
}
