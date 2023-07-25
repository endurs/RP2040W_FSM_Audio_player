#include <Arduino.h>
#include "gpio_utils.h"

void flasher (unsigned int timer) {
  if ( timer % 300 < 125 && timer % 300 > 0){
    digitalWrite(led_status_pin, HIGH);
  } else {
    digitalWrite(led_status_pin, LOW);
  }
  if ( timer % 300 < 225 && timer % 300 > 100){
    digitalWrite(led_mode_pin, HIGH);
  } else {
    digitalWrite(led_mode_pin, LOW);
  }
  if ( (timer % 300)+25 < 325 && timer % 300 > 200 ){
    digitalWrite(led_state_pin, HIGH);
  } else {
    digitalWrite(led_state_pin, LOW);
  }
}