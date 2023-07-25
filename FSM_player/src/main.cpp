#include "SdFat.h"
#include "FSM.h"
#include "gpio_utils.h"


uint8_t sensorPins[8] = {0, 1, 2, 3, 4, 5, 6, 7 }; 
bool sensorStates[8];

FSM fsm;

//------------------------------------------------------------------------------
void setup() {


  //initialize led pins
  pinMode(led_status_pin, OUTPUT);
  pinMode(led_mode_pin, OUTPUT);
  pinMode(led_state_pin, OUTPUT);


  //initialize sensor pins to input and pullup
  for (uint8_t i = 0; i < 8; i++) {
    pinMode(sensorPins[i], INPUT_PULLUP);
  }
  pinMode(resetPin, INPUT_PULLUP);

  Serial.begin(115200);

// wait for serial to be connected or for three seconds to pass. while waiting, flash all three leds in sequence
unsigned int startup_timer = millis();
while (!Serial && startup_timer+5000 > millis()) {
  flasher(millis()-startup_timer);
  delay(5);

}
  


  fsm.loadConfiguration();

  fsm.begin();


  Serial.println(F("\nSetup complete"));
  delay(500);
}

void loop() {

  fsm.update(sensorPins, sensorStates);

  delay(5);
}
