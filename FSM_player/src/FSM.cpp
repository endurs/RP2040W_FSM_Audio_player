#include "FSM.h"

const char* conditionToStr(ConditionType conditionType) {
    switch (conditionType) {
        case SENSOR:         return "SENSOR";
        case TIME_PASSED:    return "TIME_PASSED";
        case AUDIO_FINISHED: return "AUDIO_FINISHED";
        case SKIP_FLAG:      return "SKIP_FLAG";
        case RESET_FLAG:     return "RESET_FLAG";
        default:             return "UNKNOWN";
    }
}

void print_transition(Transition& transition){
            Serial.print("\n\tTarget state: ");
            Serial.print(transition.targetState);
            Serial.print("\n\tNumber of conditions: ");
            Serial.print(transition.numConditions);

            //go through all conditions and print them
            for (uint8_t k = 0; k < transition.numConditions; k++) {
                Condition& condition = transition.conditions[k];            // grab a reference to a condition (all conditions in a transition will be looked at in order)

                if (transition.numConditions > 1) {
                    Serial.print("\n\t\tCondition #");
                    Serial.print(k);
                }

                Serial.print("\n\t\tType: ");
                Serial.print(conditionToStr(condition.type));

                switch (condition.type) {
                    case SENSOR:                                            //check relevant sensor pin for the desired state
                        Serial.print("\n\t\tSensor pin: ");
                        Serial.print(condition.sensorPin);
                        Serial.print("\n\t\tState: ");
                        Serial.print(condition.state);
                        break;
                    case TIME_PASSED:                                       //check if the desired condition time has passed
                        Serial.print("\n\t\tDuration: ");
                        Serial.print(condition.duration);
                        break;
                }
            }
}

FSM::FSM(){  
    //AudioSourceSDFAT& source_in) : source(source_in) {
    // Constructor
    // Initialize variables here
    // Note: The actual initialization will be done in FSM::begin()
}

void FSM::begin() {
    // Initialize the FSM
    // This is where you'd set the current state to the initial state, etc.
    currentState = 0;
    lastStateChange = millis();
    skipFlag = false;
    resetFlag = false;
}

void FSM::loadConfiguration() {
    // Load the JSON configuration from the SD card
    // Initialize SdFat or print a detailed error message and halt
    // Use SdFat sd;

    //light up mode led to indicate loading
    digitalWrite(led_mode_pin, HIGH);


    const uint8_t SD_CS_PIN = SS;
    #define SPI_CLOCK SD_SCK_MHZ(50)
    #define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)

    SdFat32 sd;
    File32 file;

    // Initialize the SD.
    if (!sd.begin(SD_CONFIG)) {
        sd.initErrorHalt(&Serial);
        return;
    }

    Serial.println("OK!");
    // Open the file
    File32 configFile = sd.open("/FSM_Config.json", O_READ);
    if (!configFile) {
        Serial.println("Failed to open config file");
        return;
    }

    // Parse the JSON document
    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
        Serial.println("Failed to parse config file");
        return;
    }

    // Close the file (we are done with it)
    configFile.close();

    // Extract states array
    JsonArray statesArray = doc["states"];
    numStates = statesArray.size();
    states = new State[numStates];

    // Loop through each state in the array
    for (uint8_t i = 0; i < numStates; i++) {
        JsonObject stateObject = statesArray[i];

        State &state = states[i];

        // Extract state properties
        state.id = stateObject["id"];
        state.audioFile = stateObject["audioFile"].as<String>();
        state.repeat = stateObject["repeat"];
        state.blinkCount = stateObject["blinkCount"];
        
        //print state info to serial
        Serial.print("\n\nState #");
        Serial.print(state.id);
        Serial.print("\nAudio file: ");
        Serial.print(state.audioFile);
        Serial.print("\nRepeat: ");
        Serial.print(state.repeat);
        Serial.print("\nBlink count: ");
        Serial.print(state.blinkCount);


        // Extract transitions array
        JsonArray transitionsArray = stateObject["transitions"];
        state.numTransitions = transitionsArray.size() +2;               // +2 for skip and reset transitions
        state.transitions = new Transition[states[i].numTransitions +2];

        //print number of transitions
        Serial.print("\nNumber of transitions: ");
        Serial.print(state.numTransitions);


        // Loop through each transition in the array // -2 for skip and reset transitions, they are added later
        for (uint8_t j = 0; j < state.numTransitions-2; j++) {
            JsonObject transitionObject = transitionsArray[j];
            Transition &transition = state.transitions[j];
            

            // Extract transition properties
            transition.targetState = transitionObject["targetState"];

            // Extract conditions array
            JsonArray conditionsArray = transitionObject["conditions"];
            transition.numConditions = conditionsArray.size();
            transition.conditions = new Condition[transition.numConditions];           

            // Loop through each condition in the array
            for (uint8_t k = 0; k < transition.numConditions; k++) {
                JsonObject conditionObject = conditionsArray[k];
                Condition &condition = transition.conditions[k];

                // Extract condition properties
                String type = conditionObject["type"].as<String>();
                if (type == "SENSOR") {
                    condition.type = SENSOR;
                    condition.sensorPin = conditionObject["data"]["sensorPin"];
                    condition.state = conditionObject["data"]["state"];


                } else if (type == "TIME_PASSED") {
                    condition.type = TIME_PASSED;
                    condition.duration = conditionObject["data"]["duration"];


                } else if (type == "AUDIO_FINISHED") {
                    condition.type = AUDIO_FINISHED;

                }
            }

            Serial.print("\n\tTransition #");
            Serial.print(j);
            print_transition(transition);

        }
    
        //add skip transition
        uint8_t skip_transition_index = state.numTransitions - 2;
        state.transitions[ skip_transition_index ].targetState = state.id + 1;
        state.transitions[ skip_transition_index ].numConditions = 1;
        state.transitions[ skip_transition_index ].conditions = new Condition[1];
        state.transitions[ skip_transition_index ].conditions[0].type = SKIP_FLAG;


        //add reset transition
        uint8_t reset_transition_index = state.numTransitions - 1;
        state.transitions[ reset_transition_index ].targetState = 0;
        state.transitions[ reset_transition_index ].numConditions = 1;
        state.transitions[ reset_transition_index ].conditions = new Condition[1];
        state.transitions[ reset_transition_index ].conditions[0].type = RESET_FLAG;
    
    }


    digitalWrite(led_mode_pin, LOW);
    //rapidly blink mode 10 times led to indicate done loading
    for (int i = 0; i < 4; i++) {
        digitalWrite(led_mode_pin, HIGH);
        delay(100);
        digitalWrite(led_mode_pin, LOW);
        delay(100);
    }
}


void FSM::checkSwitches(uint8_t sensorPins[8], bool sensorStates[8]) {
    // Check the state of the sensor switches
    // This function will read the state of each sensor switch
    // and store the state in the sensorStates array.
    for (uint8_t i = 0; i < 8; i++) {
        sensorStates[i] = digitalRead(sensorPins[i]);
    }
}

void FSM::checkResetSwitch() {
   
   //check if reset switch is pressed for less than 3 seconds, or held for 3 seconds

    if (buttonPressEvent){
        //check if held for more than 2 seconds
        if (millis() - buttonPressStart > 2000) {
            resetFlag = true;
            buttonPressEvent = false;
            
            long led_tracker = 0;
            while (digitalRead(resetPin) == LOW) { //wait until button is released
                delay(5);
                led_tracker += 1;

                if (led_tracker % 80 < 40) {
                    digitalWrite(led_mode_pin, HIGH);
                }
                else {
                    digitalWrite(led_mode_pin, LOW);
                }
            }
            digitalWrite(led_mode_pin, LOW);
            return;
        }
        
        else if (digitalRead(resetPin) == HIGH) { //check if released before 3 seconds
            skipFlag = true;
            buttonPressEvent = false;
            digitalWrite(led_mode_pin, LOW);
            return;
        }
    }

    //check if buttonpress has begun
   else if (digitalRead(resetPin) == LOW) {
        //debounce test
        bool debounce = false; 
        for (int i = 0; i < 10; i++) {
            delay(5);
            if (digitalRead(resetPin) == HIGH) {
                debounce = true;
                break;
            }
        }

        if (!debounce) {
            buttonPressStart = millis();
            buttonPressEvent = true;
            digitalWrite(led_mode_pin, HIGH);
        }
   
   }

}

void FSM::changeState(bool sensorStates[8]) {
    // Handle state transitions based on the conditions in the current state's transitions
    // This function will check each condition of each transition of the current state,
    // and if all conditions of a transition are met, change to the target state.
    for (uint8_t i = 0; i < states[currentState].numTransitions; i++) { // For the number of transition conditions in the current state
        Transition& transition = states[currentState].transitions[i];   // grab a reference to a transition (all possible transitions will be looked at in order)
        uint8_t conditionsToMeet = transition.numConditions;                      // Assume the transition conditions are met
        
        for (uint8_t j = 0; j < transition.numConditions; j++) {        // For the number of conditions in the transition
            Condition& condition = transition.conditions[j];            // grab a reference to a condition (all conditions in a transition will be looked at in order)
            // Serial.println("Checking condition  type:");
            // Serial.println(condition.type);
            switch (condition.type) {
                case SENSOR:                                            //check relevant sensor pin for the desired state
                    if (sensorStates[condition.sensorPin] == condition.state) {
                        conditionsToMeet -= 1;
                    }
                    break;
                case TIME_PASSED:                                       //check if the desired condition time has passed
                    if (millis() - lastStateChange > condition.duration) {
                        conditionsToMeet -= 1;
                    }
                    break;

                case AUDIO_FINISHED:                                    //check if the audio has finished playing
                    // Here we would check if the audio has finished playing.
                    // As this requires interaction with the audio library, which is not
                    // part of this example, we'll just leave a TODO comment here.
                    // TODO: Check if audio has finished playing.
                    if (condition.type == AUDIO_FINISHED){
                        Serial.println("Audio finished transition triggered scuffed shit bruh");
                    }
                    break;
                case SKIP_FLAG:                                         //check if the skip flag is set
                    if (skipFlag) {
                        conditionsToMeet -= 1;
                        skipFlag = false;
                    }
                    break;
                case RESET_FLAG:                                        //check if the reset flag is set
                    if (resetFlag) {
                        conditionsToMeet -= 1;
                        resetFlag = false;
                    }
                    break;
            }
        }

        if (conditionsToMeet == 0) {  // If all conditions are met (none of the condition tests were negative), change to the target state
            currentState = transition.targetState;
            lastStateChange = millis();
            Serial.println("Transition successful: ");

            print_transition(transition);

            break;
        }
    }
}


// void FSM::playAudio(AudioPlayer& player) {
//     // Play the audio for the current state
//     // This function will load the audio file of the current state into the audio player
//     // and start playing if not already playing.

//     // get the current state
//     State& state = states[currentState];
    
//     // check if audio is not playing, or has finished and the state is set to repeat, or the current audio file does not match the state's audio file
    
//     bool playing_audio = !player.isActive();
//     //// bool EOF_and_repeat = (player && state.repeat); //didnt find easy implementation of checking if end of file 💀 can check when i get HW, also dont actually think thisi s needed
//     bool audio_file_mismatch = (state.audioFile != currentAudioFile);
    
//     if (playing_audio || audio_file_mismatch) {
//         // stop any previously playing audio
//         player.stop();
        
//         // set the audio file of the current state
//         //source.setPath(state.audioFile.c_str());

//         // start playing the audio
//         player.begin();
//         player.setAutoNext(state.repeat); ///!!!!!!!!!! if enabled can automatically move to next track(s)
//     }
// }



void FSM::blinkLED() {
    // Calculate the length of one full cycle (blinking phase + waiting phase)
    // The blinking phase length is blinkCount * 2 * blinkDuration (on and off)
    // The waiting phase length is waitDuration

    uint8_t blinkCount = states[currentState].blinkCount;
    unsigned long blinkDuration = 200; // 200 ms on time (and )off time)
    unsigned long waitDuration = 1500; // 2 seconds of waiting after blinking
    unsigned long cycleLength = blinkCount *  (2 * blinkDuration) + waitDuration;

    // Calculate the time passed since the start of the current cycle
    unsigned long timeInCycle = (millis() - lastStateChange) % cycleLength;

    // Serial.print("\nCycle time: ");
    // Serial.println(timeInCycle);
    // Serial.print("Cycle length: ");
    // Serial.println(cycleLength);

    // Determine whether we are in the blinking phase or the waiting phase
    if (timeInCycle < blinkCount * (2 * blinkDuration)) { // blinking phase
        unsigned long timeInBlink = timeInCycle % (2 * blinkDuration); //time since last blink started
        if (timeInBlink < blinkDuration) {
            digitalWrite(led_state_pin, HIGH);
        } else {
            digitalWrite(led_state_pin, LOW);
        }

    } else { // waiting phase
        digitalWrite(led_state_pin, LOW);
    }
}


void FSM::update(uint8_t sensorPins[8], bool sensorStates[8]) { //AudioPlayer& player
    // Update the FSM
    // This function calls all the other functions in the appropriate order.
    checkSwitches(sensorPins, sensorStates);
    checkResetSwitch();
    changeState(sensorStates);
    //playAudio(player);
    blinkLED();
}
