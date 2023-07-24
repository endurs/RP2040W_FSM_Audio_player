#include "finite_state_machine.h"

FSM::FSM() {
    // Constructor
    // Initialize variables here
    // Note: The actual initialization will be done in FSM::begin()
}

void FSM::begin() {
    // Initialize the FSM
    // This is where you'd set the current state to the initial state, etc.
    currentState = 0;
    lastStateChange = millis();
}

void FSM::loadConfiguration() {
    // Load the JSON configuration from the SD card
    // This function will read the JSON file from the SD card,
    // parse it into the states and transitions arrays,
    // and store the number of states.
    // TODO: Implement this
}

void FSM::checkSwitches(uint8_t sensorPins[8], bool sensorStates[8]) {
    // Check the state of the sensor switches
    // This function will read the state of each sensor switch
    // and store the state in the sensorStates array.
    for (uint8_t i = 0; i < 8; i++) {
        sensorStates[i] = digitalRead(sensorPins[i]);
    }
}

void FSM::checkResetSwitch(uint8_t resetPin) {
    // Check the state of the reset switch
    // If the switch has been held down for 3 seconds, reset the FSM
    if (digitalRead(resetPin) == LOW) {
        if (millis() - lastStateChange >= 3000) {
            currentState = 0;
            lastStateChange = millis();
        }
    } else {
        lastStateChange = millis();
    }
}

void FSM::changeState(bool sensorStates[8]) {
    // Handle state transitions based on the conditions in the current state's transitions
    // This function will check each condition of each transition of the current state,
    // and if all conditions of a transition are met, change to the target state.
    for (uint8_t i = 0; i < states[currentState].numTransitions; i++) { // For the number of transition conditions in the current state
        Transition& transition = states[currentState].transitions[i];   // grab a reference to a transition (all possible transitions will be looked at in order)
        bool transitionConditionsMet = true;                            // Assume the transition conditions are met

        for (uint8_t j = 0; j < transition.numConditions; j++) {        // For the number of conditions in the transition
            Condition& condition = transition.conditions[j];            // grab a reference to a condition (all conditions in a transition will be looked at in order)

            switch (condition.type) {
                case SENSOR:                                            //check relevant sensor pin for the desired state
                    if (sensorStates[condition.sensorPin] != condition.state) {
                        transitionConditionsMet = false;
                    }
                    break;
                case TIME_PASSED:                                       //check if the desired condition time has passed
                    if (millis() - lastStateChange < condition.duration) {
                        transitionConditionsMet = false;
                    }
                    break;
                case AUDIO_FINISHED:                                    //check if the audio has finished playing
                    // Here we would check if the audio has finished playing.
                    // As this requires interaction with the audio library, which is not
                    // part of this example, we'll just leave a TODO comment here.
                    // TODO: Check if audio has finished playing.
                    break;
            }

            if (!transitionConditionsMet) { // If any condition is not met "transition has failed", break out of the loop for efficiency
                break;
            }
        }

        if (transitionConditionsMet) {  // If all conditions are met (none of the condition tests were negative), change to the target state
            currentState = transition.targetState;
            lastStateChange = millis();
            break;
        }
    }
}


void FSM::playAudio(AudioPlayer& player) {
    // Play the audio for the current state
    // This function will load the audio file of the current state into the audio player
    // and start playing if not already playing.
    
    // get the current state
    State& state = states[currentState];
    
    // (check if audio is not playing) or (has finished) and (the state is set to repeat), then begin to play audio
    if (!player.isActive() || (player.isFinished() && state.repeat)) {
        // stop any previously playing audio
        player.stop();
        
        // set the audio file of the current state
        source.setFile(state.audioFile.c_str());
        
        // start playing the audio
        player.begin();
    }
}


void FSM::blinkLED(uint8_t ledPin) {
    // Calculate the length of one full cycle (blinking phase + waiting phase)
    // The blinking phase length is blinkCount * 2 * blinkDuration (on and off)
    // The waiting phase length is waitDuration

    uint8_t blinkCount = states[currentState].blinkCount;
    unsigned long blinkDuration = 200; // 200 ms on time (and )off time)
    unsigned long waitDuration = 1500; // 2 seconds of waiting after blinking
    unsigned long cycleLength = blinkCount *  (2 * blinkDuration) + waitDuration;

    // Calculate the time passed since the start of the current cycle
    unsigned long timeInCycle = (millis() - lastStateChange) % cycleLength;

    // Determine whether we are in the blinking phase or the waiting phase
    if (timeInCycle < blinkCount * 2 * blinkDuration) { // blinking phase
        
        unsigned long timeInBlink = timeInCycle % (2 * blinkDuration); //time since last blink started
        if (timeInBlink < blinkDuration) {
            digitalWrite(ledPin, HIGH);
        } else {
            digitalWrite(ledPin, LOW);
        }

    } else { // waiting phase
        digitalWrite(ledPin, LOW);
    }
}


void FSM::update(uint8_t sensorPins[8], bool sensorStates[8], uint8_t resetPin, AudioPlayer& player, uint8_t ledPin) {
    // Update the FSM
    // This function calls all the other functions in the appropriate order.
    checkSwitches(sensorPins, sensorStates);
    checkResetSwitch(resetPin);
    changeState(sensorStates);
    playAudio(player);
    blinkLED(ledPin);
}
