#ifndef FINITE_STATE_MACHINE_H
#define FINITE_STATE_MACHINE_H

#include <SdFat.h>
#include <ArduinoJson.h>
#include "gpio_utils.h"

// Enumeration for condition types
enum ConditionType {
    SENSOR,
    TIME_PASSED,
    AUDIO_FINISHED,
    SKIP_FLAG,
    RESET_FLAG
};


// Structure to hold condition information
struct Condition {
    ConditionType type;   // Type of condition
    union {
        struct {                // Data for SENSOR type
            uint8_t sensorPin;  // Sensor pin related to this condition
            bool state;         // Desired state of the sensor
        };
        unsigned long duration; // Data for TIME_PASSED type
    };
};

// Structure to hold transition information
struct Transition {
    uint8_t targetState;        // Target state ID
    Condition* conditions;      // Array of conditions
    uint8_t numConditions;      // Number of conditions
};

// Structure to hold state information
struct State {
    uint8_t id;                 // Unique state ID
    String audioFile;           // Audio file to be played in this state
    bool repeat;                // Repeat behavior of the audio file
    uint8_t blinkCount;         // LED blink count
    Transition* transitions;    // Array of possible transitions
    uint8_t numTransitions;     // Number of transitions
};

class FSM {
    public:
        FSM(); //AudioSourceSDFAT& source_in);  // Constructor

        // Initializes the FSM
        void begin();

        // Loads the JSON configuration from the SD card
        void loadConfiguration();

        // Checks the state of the sensor switches
        void checkSwitches(uint8_t sensorPins[8], bool sensorStates[8]);

        // Checks the state of the reset switch
        void checkResetSwitch();

        // Handles state transitions
        void changeState(bool sensorStates[8]);

        // Plays the audio for the current state
        //void playAudio(AudioPlayer& player);

        // Blinks the LED for the current state
        void blinkLED();

        // Update the FSM
        void update(uint8_t sensorPins[8], bool sensorStates[8]); //AudioPlayer& player, 

    private:
        State* states;                  // Array of states
        uint8_t numStates;              // Number of states
        uint8_t currentState;           // Current state ID
        unsigned long lastStateChange;  // Timestamp of the last state change
        unsigned long buttonPressStart;  // Timestamp of the last button press
        bool buttonPressEvent;          // Flag for button press event
        String currentAudioFile;        // Current audio file

        bool skipFlag;                  // Flag to skip the current state
        bool resetFlag;                 // Flag to reset the FSM
        //AudioSourceSDFAT& source;       // Audio source
};

#endif  // FINITE_STATE_MACHINE_H
